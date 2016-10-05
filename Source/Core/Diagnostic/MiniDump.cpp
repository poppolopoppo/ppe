#include "stdafx.h"

#include "MiniDump.h"

#include "Thread/AtomicSpinLock.h"

/*
// http://www.debuginfo.com/articles/effminidumps2.html
// http://blog.aaronballman.com/2011/05/generating-a-minidump/
*/

#ifndef _MSC_VER

namespace MiniDump
{
    EResult Write(const wchar_t *filename, void *exception_ptrs, ELevel level) {
        UNUSED(filename);
        UNUSED(exception_ptrs);
        UNUSED(level);
        return EResult::NotAvailable;
    }

    void FStartup() {}
    void Shutdown() {}

    const char *ResultMessage(EResult code) { return "MiniDump: unsupported platform"; }
}

#else

#include "DbghelpWrapper.h"

#include <Windows.h>
#include <winnt.h>
#include <time.h>
#include <TlHelp32.h>

#ifndef ARCH_X64
#   define HANDLE_VECTORED_EXCEPTION
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FAtomicSpinLock gMinidumpBarrier_;
//----------------------------------------------------------------------------
struct FMinidumpParams_ {
    const wchar_t* Filename;
    const void* ExceptionPtrs;
    MiniDump::InfoLevel Level;
};
//----------------------------------------------------------------------------
// We use this structure as a way to pass some information to our static
// callback.  It's only used for transport.
struct FMinidumpCallbackParam_ {
    DWORD ThreadId;
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
BOOL CALLBACK MinidumpFilter_(PVOID inParam, const PMINIDUMP_CALLBACK_INPUT inInput, PMINIDUMP_CALLBACK_OUTPUT outOutput )
{
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
    const MiniDump::InfoLevel infoLevel = p->Params->Level;

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
            if (inInput->IncludeThread.ThreadId == p->ThreadId)
                return FALSE;
            return TRUE;
        } break;

        case MemoryCallback: {
            // Small and medium sized dumps don't need full memory access
            if (MiniDump::InfoLevel::Small == infoLevel ||
                MiniDump::InfoLevel::Medium == infoLevel)
                return FALSE;
            return TRUE;
        } break;

        case ModuleCallback: {
            if (MiniDump::InfoLevel::Small == infoLevel) {
                // When creating a small dump file, we filter out any modules that
                // aren't being directly referenced.
                if (!(outOutput->ModuleWriteFlags & ModuleReferencedByMemory)) {
                    outOutput->ModuleWriteFlags &= ~ModuleWriteModule;
                }
            } else if (MiniDump::InfoLevel::Medium == infoLevel) {
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
    const FMinidumpParams_ *const p = (const FMinidumpParams_ *)inParam;

    // We need to keep track of the name of the process, sans extension, so that it can
    // be used if we're filtering module data segments.  So we calculate that up front.
    wchar_t processName[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(NULL, processName, MAX_PATH );
    ::_wsplitpath_s(processName, NULL, 0, NULL, 0, processName, MAX_PATH, NULL, 0 );

    // TFirst, attempt to create the file that the minidump will be written to
    HANDLE hFile = ::CreateFileW(p->Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if (INVALID_HANDLE_VALUE != hFile) {
        // Determine what information we want the minidumper to pass along for our
        // callback to filter out.  Generally speaking, the more things we include,
        // the larger our file will be.
        int type = MiniDumpNormal;
        switch (p->Level) {
            case MiniDump::InfoLevel::Small: {
                type |= MiniDumpWithIndirectlyReferencedMemory |
                        MiniDumpScanMemory;
            } break;
            case MiniDump::InfoLevel::Medium: {
                type |= MiniDumpWithDataSegs |
                        MiniDumpWithPrivateReadWriteMemory |
                        MiniDumpWithHandleData |
                        MiniDumpWithFullMemoryInfo |
                        MiniDumpWithThreadInfo |
                        MiniDumpWithUnloadedModules;
            } break;
            case MiniDump::InfoLevel::Large: {
                type |= MiniDumpWithDataSegs |
                        MiniDumpWithPrivateReadWriteMemory |
                        MiniDumpWithHandleData |
                        MiniDumpWithFullMemory |
                        MiniDumpWithFullMemoryInfo |
                        MiniDumpWithThreadInfo |
                        MiniDumpWithUnloadedModules |
                        MiniDumpWithProcessThreadData;
            } break;
        }

        // Set up the callback to be called by the minidump writer.  This allows us to
        // filter out information that we may not care about.
        ::MINIDUMP_CALLBACK_INFORMATION callback = { 0, nullptr };

        const FMinidumpCallbackParam_ info = { ::GetCurrentThreadId(), processName, p };
        callback.CallbackParam = (PVOID)&info;
        callback.CallbackRoutine = MinidumpFilter_;

        // TAfter all that, we can write out the minidump
        BOOL bRet = FDbghelpWrapper::Instance().Lock().MiniDumpWriteDump()(
            ::GetCurrentProcess(),
            ::GetCurrentProcessId(),
            hFile,
            (MINIDUMP_TYPE)type, NULL, NULL, &callback );

        if (FALSE == ::CloseHandle( hFile ))
            return DWORD(MiniDump::EResult::FailedToCloseHandle);

        return DWORD(bRet ? MiniDump::EResult::Success : MiniDump::EResult::DumpFailed);
    }

    return DWORD(MiniDump::EResult::CantCreateFile);
}
//----------------------------------------------------------------------------
static void EnumerateThreads_(DWORD (WINAPI *inCallback)( HANDLE ), DWORD inExceptThisOne)
{
    // Create a snapshot of all the threads in the process, and walk over
    // them, calling the callback function on each of them, except for
    // the thread identified by the inExceptThisOne parameter.
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0 );
    if (INVALID_HANDLE_VALUE != hSnapshot) {
        THREADENTRY32 thread;
        thread.dwSize = sizeof( thread );
        if (::Thread32First( hSnapshot, &thread )) {
            do {
                if (thread.th32OwnerProcessID == ::GetCurrentProcessId() &&
                    thread.th32ThreadID != inExceptThisOne &&
                    thread.th32ThreadID != ::GetCurrentThreadId()) {
                    // We're making a big assumption that this call will only
                    // be used to suspend or resume a thread, and so we know
                    // that we only require the THREAD_SUSPEND_RESUME access right.
                    HANDLE hThread = ::OpenThread( THREAD_SUSPEND_RESUME, FALSE, thread.th32ThreadID );
                    if (hThread) {
                        inCallback( hThread );
                        ::CloseHandle( hThread );
                    }
                }
            } while (::Thread32Next( hSnapshot, &thread ) );
        }

        ::CloseHandle( hSnapshot );
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core


namespace Core {
namespace MiniDump
{
    EResult Write(const wchar_t *filename, InfoLevel level,
        const void *exception_ptrs /* = nullptr */,
        bool inSuspendOtherThreads /* = false */)
    {
        if ( nullptr == filename )
            return EResult::InvalidFilename;

        if ( false == FDbghelpWrapper::HasInstance() )
            return EResult::NoDbgHelpDLL;

        const FMinidumpParams_ params = { filename, exception_ptrs, level };

        DWORD threadId = 0;
        HANDLE hThread = ::CreateThread(NULL, 0, MinidumpWriter_, (PVOID)&params, CREATE_SUSPENDED, &threadId );

        if (hThread) {
            // Having created the thread successfully, we need to put all of the other
            // threads in the process into a suspended state, making sure not to suspend
            // our newly-created thread.  We do this because we want this function to
            // behave as a snapshot, and that means other threads should not continue
            // to perform work while we're creating the minidump.
            if (inSuspendOtherThreads) {
                EnumerateThreads_(::SuspendThread, threadId );
            }

            // Now we can resume our worker thread
            ::ResumeThread(hThread );

            // Wait for the thread to finish working, without allowing the current
            // thread to continue working.  This ensures that the current thread won't
            // do anything interesting while we're writing the debug information out.
            // This also means that the minidump will show this as the current callstack.
            ::WaitForSingleObject(hThread, INFINITE );

            // The thread exit code tells us whether we were able to create the minidump
            DWORD code = 0;
            ::GetExitCodeThread(hThread, &code );
            ::CloseHandle(hThread );

            // If we suspended other threads, now is the time to wake them up
            if (inSuspendOtherThreads) {
                EnumerateThreads_(::ResumeThread, threadId );
            }

            return MiniDump::EResult(code);
        }

        return MiniDump::EResult::NotAvailable;
    }

    void Write(PEXCEPTION_POINTERS pExceptionInfo)
    {
        time_t tNow = ::time( NULL );
        struct tm pTm;
        localtime_s( &pTm, &tNow );

        wchar_t filename[MAX_PATH];
        {
            wchar_t modulePath[MAX_PATH];
            if (0 == ::GetModuleFileNameW(NULL, modulePath, MAX_PATH) )
                return;

            swprintf_s(filename, MAX_PATH,
                L"%s.%02d%02d%04d_%02d%02d%02d.dmp",
                modulePath,
                pTm.tm_year, pTm.tm_mon, pTm.tm_mday,
                pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
        }

        MiniDump::Write(filename, InfoLevel::Large, pExceptionInfo, true);
    }

    static volatile LPTOP_LEVEL_EXCEPTION_FILTER gPreviousUnhandledExceptionFilter = nullptr;
    LONG WINAPI OnUnhandledException_(PEXCEPTION_POINTERS pExceptionInfo)
    {
        // no reentrancy (needed by ::MiniDumpWriteDump !)
        const FAtomicSpinLock::FTryScope scopeLock(gMinidumpBarrier_);

        if (scopeLock.Locked())
            Write(pExceptionInfo);

        return EXCEPTION_EXECUTE_HANDLER;
    }

#ifdef HANDLE_VECTORED_EXCEPTION
    static volatile LPVOID gHandleVectoredExceptionHandler = nullptr;
    LONG WINAPI OnVectoredHandler_(PEXCEPTION_POINTERS pExceptionInfo)
    {
        // no reentrancy (needed by ::MiniDumpWriteDump !)
        const FAtomicSpinLock::FTryScope scopeLock(gMinidumpBarrier_);

        if (scopeLock.Locked())
            Write(pExceptionInfo);

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif

    void Start()
    {
        if (not FDbghelpWrapper::HasInstance())
            return;

#ifdef HANDLE_VECTORED_EXCEPTION
        gHandleVectoredExceptionHandler = ::AddVectoredExceptionHandler(0, &OnVectoredHandler_);
#endif

        gPreviousUnhandledExceptionFilter = ::SetUnhandledExceptionFilter(&OnUnhandledException_);
    }

    void Shutdown()
    {
        if (not FDbghelpWrapper::HasInstance())
            return;

        ::SetUnhandledExceptionFilter(gPreviousUnhandledExceptionFilter);

#ifdef HANDLE_VECTORED_EXCEPTION
        ::RemoveVectoredExceptionHandler(gHandleVectoredExceptionHandler);
#endif
    }
}//!namespace MiniDump
}//!namespace Core

#endif //!_MSC_VER

namespace Core {
namespace MiniDump
{
    const char *ResultMessage(EResult code)
    {
        switch (code)
        {
        case MiniDump::EResult::Success:
            return "minidump was successfully created";
        case MiniDump::EResult::NoDbgHelpDLL:
            return "minidump failed to retrieve dbghelp.dll";
        case MiniDump::EResult::InvalidFilename:
            return "minidump filename is invalid";
        case MiniDump::EResult::CantCreateFile:
            return "minidump file could not be created";
        case MiniDump::EResult::DumpFailed:
            return "failed to create a minidump";
        case MiniDump::EResult::FailedToCloseHandle:
            return "minidump file was not closed correctly";
        case MiniDump::EResult::NotAvailable:
            return "minidump is not available on this platform";
        default:
            return "invalid minidump result code";
        }
    }
} //!namespace MiniDump
} //!namespace Core
