﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformCrash.h"

#ifdef PLATFORM_WINDOWS

/*
// http://www.debuginfo.com/articles/effminidumps2.html
// http://blog.aaronballman.com/2011/05/generating-a-minidump/
*/

#include "Memory/MemoryDomain.h"
#include "Thread/AtomicSpinLock.h"

#ifndef ARCH_X64
#   define USE_VECTORED_EXCEPTION_HANDLER 0
#else
#   define USE_VECTORED_EXCEPTION_HANDLER 0 //1 // bad idea : vectored exception are used internally by windows
#endif

#if USE_PPE_MEMORYDOMAINS
#   define USE_PPE_MINIDUMP_EMBED_MEMORYTRACKING 1
#else
#   define USE_PPE_MINIDUMP_EMBED_MEMORYTRACKING 0
#endif

#if USE_PPE_MINIDUMP_EMBED_MEMORYTRACKING
#   include "Diagnostic/CurrentProcess.h"
#   include "IO/TextWriter.h"
#   include "Memory/MemoryTracking.h"
#endif

#include "HAL/Windows/DbgHelpWrapper.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"
#include "HAL/Windows/WindowsPlatformTime.h"

#include <chrono>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FAtomicSpinLock GMinidumpBarrier_;
//----------------------------------------------------------------------------
struct FMinidumpParams_ {
    const ::DWORD ExceptionThreadId;
    const wchar_t* Filename;
    const void* ExceptionPtrs;
    FWindowsPlatformCrash::EInfoLevel Level;
};
//----------------------------------------------------------------------------
// We use this structure as a way to pass some information to our static
// callback.  It's only used for transport.
struct FMinidumpCallbackParam_ {
    const ::DWORD MinidumpThreadId;
    const wchar_t* ProcessName;
    const FMinidumpParams_* Params;
};
//----------------------------------------------------------------------------
static bool IsDataSegmentNeeded_(const wchar_t* processName, const wchar_t *inPath) {
    if (!inPath)
        return false;

    // Get the name of the module we're trying to query.  We
    // only want to allow the data segments for the bare minimum
    // number of modules.  That includes the kernel, and the process
    // itself.  However, we may want to extend this in the future
    // to allow the application to register modules it cares about too.
    wchar_t fileName[MAX_PATH] = { 0 };
    ::_wsplitpath_s(inPath, NULL, 0, NULL, 0, fileName, MAX_PATH, NULL, 0 );

    if (::_wcsicmp(fileName, processName) == 0 ||
        ::_wcsicmp(fileName, L"kernel32" ) == 0 ||
        ::_wcsicmp(fileName, L"ntdll" ) == 0) {
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
BOOL CALLBACK MinidumpFilter_(PVOID inParam, const PMINIDUMP_CALLBACK_INPUT inInput, PMINIDUMP_CALLBACK_OUTPUT outOutput ) {
    using EInfoLevel = FWindowsPlatformCrash::EInfoLevel;

    if (!inInput || !outOutput || !inParam)
        return FALSE;

    // Most of the time, we're going to let the minidump writer do whatever
    // it needs to do.  However, when trying to limit information for things
    // like smaller dumps, we want to filter out various things.  So this callback
    // is used as a way to filter out information that the user has said they
    // don't want.
    //
    // If we were so inclined, we could use a delegate class to allow the user
    // to customize the dump files even further.  But for right now, this is
    // close enough.
    const FMinidumpCallbackParam_ *const p = (FMinidumpCallbackParam_ *)inParam;
    const EInfoLevel infoLevel = p->Params->Level;

    switch (inInput->CallbackType) {
        case IncludeModuleCallback:
        case ThreadCallback:
        case ThreadExCallback:
            return TRUE;
        case CancelCallback:
            return FALSE;

        case IncludeThreadCallback: {
            // We don't want to include information about the minidump writing
            // thread, as that's not of interest to the caller
            if (inInput->IncludeThread.ThreadId == p->MinidumpThreadId)
                return FALSE;
            return TRUE;
        } break;

        case MemoryCallback: {
            // Small and medium sized dumps don't need full memory access
            if (EInfoLevel::Small == infoLevel ||
                EInfoLevel::Medium == infoLevel)
                return FALSE;
            return TRUE;
        } break;

        case ModuleCallback: {
            if (EInfoLevel::Small == infoLevel) {
                // When creating a small dump file, we filter out any modules that
                // aren't being directly referenced.
                if (!(outOutput->ModuleWriteFlags & ModuleReferencedByMemory)) {
                    outOutput->ModuleWriteFlags &= ~ModuleWriteModule;
                }
            } else if (EInfoLevel::Medium == infoLevel) {
                // When creating a medium-sized dump file, we filter out any module
                // data segments if they're not part of our core module list.  This
                // helps reduce the size of the dump file by quite a bit.
                if (outOutput->ModuleWriteFlags & ModuleWriteDataSeg) {
                    if (!IsDataSegmentNeeded_(p->ProcessName, inInput->Module.FullPath)) {
                        outOutput->ModuleWriteFlags &= ~ModuleWriteDataSeg;
                    }
                }
            }
            return TRUE;
        } break;
    }

    return FALSE;
}
//----------------------------------------------------------------------------
DWORD CALLBACK MinidumpWriter_(LPVOID inParam) {
    using EInfoLevel = FWindowsPlatformCrash::EInfoLevel;
    using EResult = FWindowsPlatformCrash::EResult;

    const FMinidumpParams_ *const p = (const FMinidumpParams_ *)inParam;

    // We need to keep track of the name of the process, without extension, so that it can
    // be used if we're filtering module data segments.  So we calculate that up front.
    wchar_t processName[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(NULL, processName, MAX_PATH );
    ::_wsplitpath_s(processName, NULL, 0, NULL, 0, processName, MAX_PATH, NULL, 0 );

    // First, attempt to create the file that the minidump will be written to
    HANDLE hFile = ::CreateFileW(p->Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if (INVALID_HANDLE_VALUE != hFile) {
        // Determine what information we want the minidumper to pass along for our
        // callback to filter out.  Generally speaking, the more things we include,
        // the larger our file will be.
        int type = MiniDumpNormal;
        switch (p->Level) {
            case EInfoLevel::Small: {
                type |= MiniDumpWithIndirectlyReferencedMemory |
                        MiniDumpScanMemory;
            } break;
            case EInfoLevel::Medium: {
                type |= MiniDumpWithDataSegs |
                        MiniDumpWithPrivateReadWriteMemory |
                        MiniDumpWithHandleData |
                        MiniDumpWithFullMemoryInfo |
                        MiniDumpWithThreadInfo |
                        MiniDumpWithUnloadedModules;
            } break;
            case EInfoLevel::Large: {
                type |= MiniDumpWithDataSegs |
                        MiniDumpWithPrivateReadWriteMemory |
                        MiniDumpWithHandleData |
                        MiniDumpWithFullMemory |
                        MiniDumpWithFullMemoryInfo |
                        MiniDumpWithThreadInfo |
                        MiniDumpWithUnloadedModules |
                        MiniDumpWithProcessThreadData;
            } break;
            default: {
                AssertNotImplemented();
            } break;
        }

        // Set up exception informations
        ::MINIDUMP_EXCEPTION_INFORMATION exception;
        exception.ThreadId = p->ExceptionThreadId;
        exception.ExceptionPointers = (::PEXCEPTION_POINTERS)p->ExceptionPtrs;
        exception.ClientPointers = (p->ExceptionPtrs ? TRUE : FALSE);

        // Set up user streams (can optional put memory tracking data)
        ::MINIDUMP_USER_STREAM_INFORMATION userstreams;
#if USE_PPE_MINIDUMP_EMBED_MEMORYTRACKING
        char memoryTracking[PPE_SYSALLOCA_SIZELIMIT / sizeof(char)];
        FFixedSizeTextWriter oss(memoryTracking);
        oss << Eol;
        FCurrentProcess::Get().DumpCrashInfos(oss);
        oss << Eos;

        ::MINIDUMP_USER_STREAM memorystream;
        memorystream.Buffer = memoryTracking;
        memorystream.BufferSize = checked_cast<::ULONG>(oss.size() - 1/* null char */);
        memorystream.Type = CommentStreamA;

        userstreams.UserStreamArray = &memorystream;
        userstreams.UserStreamCount = 1;
#else
        userstreams.UserStreamArray = NULL;
        userstreams.UserStreamCount = 0;
#endif

        // Set up the callback to be called by the minidump writer.  This allows us to
        // filter out information that we may not care about.
        ::MINIDUMP_CALLBACK_INFORMATION callback = { 0, nullptr };

        const FMinidumpCallbackParam_ info = { ::GetCurrentThreadId(), processName, p };
        callback.CallbackParam = (PVOID)&info;
        callback.CallbackRoutine = MinidumpFilter_;

        // After all that, we can write out the minidump
        ::BOOL bRet;
        {
            const FDbghelpWrapper::FLocked threadSafe(FDbghelpWrapper::Get());
            bRet = threadSafe->MiniDumpWriteDump(
                ::GetCurrentProcess(),
                ::GetCurrentProcessId(),
                hFile,
                ::MINIDUMP_TYPE(type),
                &exception,
                &userstreams,
                &callback );
        }

        if (FALSE == ::CloseHandle( hFile ))
            return DWORD(EResult::FailedToCloseHandle);

        return DWORD(bRet ? EResult::Success : EResult::DumpFailed);
    }

    return DWORD(EResult::CantCreateFile);
}
//----------------------------------------------------------------------------
// Define the NtGetNextThread function
typedef LONG (NTAPI *pfnNtGetNextThread) (
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Flags,
    OUT PHANDLE NewThreadHandle
);
static pfnNtGetNextThread LoadNtGetNextThread_() {
    // Load ntdll.dll
    HMODULE hNtDll = ::LoadLibrary(TEXT("ntdll.dll"));
    if (hNtDll == NULL) {
        // Error loading module
        return NULL;
    }

    // Get the NtGetNextThread function
    pfnNtGetNextThread NtGetNextThread = (pfnNtGetNextThread)::GetProcAddress(hNtDll, "NtGetNextThread");
    if (NtGetNextThread == NULL) {
        // Error getting function
        ::FreeLibrary(hNtDll);
        return NULL;
    }

    return NtGetNextThread;
}
//----------------------------------------------------------------------------
static FWindowsPlatformCrash::EResult EnumerateThreads_(DWORD (WINAPI *inCallback)( HANDLE ), DWORD inExceptThisOne, DWORD inCurrentThread) {
    const HANDLE hProcess = ::GetCurrentProcess();
    if (hProcess == NULL)
        return FWindowsPlatformCrash::NoDbgHelpDLL;

    static const pfnNtGetNextThread NtGetNextThread = LoadNtGetNextThread_();
    if (NtGetNextThread == NULL)
        return FWindowsPlatformCrash::NoDbgHelpDLL;

    // Enumerate the threads
    HANDLE hThread = NULL;
    while (NtGetNextThread(hProcess, hThread, THREAD_ALL_ACCESS, 0, 0, &hThread) == 0) {
        // Do something with the thread handle, e.g., print the thread ID
        const DWORD dwThreadId = ::GetThreadId(hThread);
        if (dwThreadId != inExceptThisOne &&
            dwThreadId != inCurrentThread) {
            inCallback(hThread);
        }

        // Close the thread handle
        ::CloseHandle(hThread);
    }

    return FWindowsPlatformCrash::Success;
}
//----------------------------------------------------------------------------
static FWindowsPlatformCrash::EResult DumpException_(::PEXCEPTION_POINTERS pExceptionInfo) {
    // Get current time
    const i64 now = FWindowsPlatformTime::Timestamp();
    u32 year, month, dayOfWeek, dayOfYear, dayOfMon, hour, min, sec;
    FWindowsPlatformTime::LocalTime(now, year, month, dayOfWeek, dayOfYear, dayOfMon, hour, min, sec);

    wchar_t filename[MAX_PATH];
    {
        wchar_t modulePath[MAX_PATH];
        if (0 == ::GetModuleFileNameW(NULL, modulePath, MAX_PATH))
            return FWindowsPlatformCrash::EResult::InvalidFilename;

        if (0 == ::swprintf_s(filename, MAX_PATH,
            L"%s.%02d%02d%04d_%02d%02d%02d.dmp",
            modulePath,
            year, month, dayOfMon,
            hour, min, sec))
            return FWindowsPlatformCrash::EResult::InvalidFilename;
    }

    return FWindowsPlatformCrash::WriteMiniDump(
        MakeCStringView(filename),
        FWindowsPlatformCrash::EInfoLevel::Large,
        pExceptionInfo, true );
}
//----------------------------------------------------------------------------
static void AbortProgramAndDumpException_(::PEXCEPTION_POINTERS pExceptionInfo) {
    DumpException_(pExceptionInfo);
}
//----------------------------------------------------------------------------
::LONG WINAPI OnUnhandledException_(::PEXCEPTION_POINTERS pExceptionInfo) {
    AbortProgramAndDumpException_(pExceptionInfo);
    return EXCEPTION_CONTINUE_SEARCH;
}
//----------------------------------------------------------------------------
#if USE_VECTORED_EXCEPTION_HANDLER
static volatile LPVOID GHandleVectoredExceptionHandler = nullptr;
::LONG WINAPI OnVectoredHandler_(PEXCEPTION_POINTERS pExceptionInfo) {
    AbortProgramAndDumpException_(pExceptionInfo);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif //!USE_VECTORED_EXCEPTION_HANDLER
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto FWindowsPlatformCrash::WriteMiniDump() -> EResult {
    return DumpException_(NULL);
}
//----------------------------------------------------------------------------
auto FWindowsPlatformCrash::WriteMiniDump(
    const FWStringView& filename,
    EInfoLevel level/* = Medium */,
    const void* exception_ptrs/* = nullptr */,
    bool suspendThreads/* = true */) -> EResult {

    // no reentrancy (needed by ::MiniDumpWriteDump !)
    const FAtomicSpinLock::FTryScope scopeLock(GMinidumpBarrier_);
#if USE_PPE_ASSERT
    if (not FDbghelpWrapper::HasInstance())
        return EResult::NoDbgHelpDLL;
#endif
    if (not FDbghelpWrapper::Get().Available())
        return EResult::NoDbgHelpDLL;
    if (not scopeLock.Locked)
        return EResult::Reentrancy;
    if (filename.empty())
        return EResult::InvalidFilename;

    const FMinidumpParams_ params = {
        ::GetCurrentThreadId(),
        filename.data(),
        exception_ptrs,
        level
    };

    ::DWORD threadId = 0;
    const ::HANDLE hThread = ::CreateThread(NULL, 0, MinidumpWriter_, (PVOID)&params, CREATE_SUSPENDED, &threadId);

    if (hThread) {
        // Having created the thread successfully, we need to put all of the other
        // threads in the process into a suspended state, making sure not to suspend
        // our newly-created thread.  We do this because we want this function to
        // behave as a snapshot, and that means other threads should not continue
        // to perform work while we're creating the minidump.
        if (suspendThreads) {
            const EResult result = EnumerateThreads_(::SuspendThread, threadId, params.ExceptionThreadId);
            if (result != EResult::Success)
                return result;
        }

        // Now we can resume our worker thread
        ::ResumeThread(hThread);

        // Wait for the thread to finish working, without allowing the current
        // thread to continue working.  This ensures that the current thread won't
        // do anything interesting while we're writing the debug information out.
        // This also means that the minidump will show this as the current callstack.
        ::WaitForSingleObject(hThread, INFINITE);

        // The thread exit code tells us whether we were able to create the minidump
        ::DWORD code = 0;
        ::GetExitCodeThread(hThread, &code);
        ::CloseHandle(hThread);

        // If we suspended other threads, now is the time to wake them up
        if (suspendThreads) {
            const EResult result = EnumerateThreads_(::ResumeThread, threadId, params.ExceptionThreadId);
            if (result != EResult::Success)
                return result;
        }

        return EResult(code);
    }

    return EResult::NotAvailable;
}
//----------------------------------------------------------------------------
void FWindowsPlatformCrash::SetExceptionHandlers() {
#if USE_VECTORED_EXCEPTION_HANDLER
    ::AddVectoredExceptionHandler(0, &OnVectoredHandler_);
#endif
    ::SetUnhandledExceptionFilter(&OnUnhandledException_);
}
//----------------------------------------------------------------------------
void FWindowsPlatformCrash::AbortProgramWithDump() {
    throw std::runtime_error("abort program with dump");
}
//----------------------------------------------------------------------------
void FWindowsPlatformCrash::SubmitErrorReport(const FStringView& context, EReportMode mode/* = Unattended */) {
    // #TODO pain in the ass atm
    Unused(context);
    Unused(mode);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
