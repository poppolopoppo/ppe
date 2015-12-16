#include "stdafx.h"

#include "MiniDump.h"

#include "Thread/AtomicSpinLock.h"

/*
// EFFECTIVE MINIDUMPS
// http://www.debuginfo.com/articles/effminidumps2.html
*/

#ifndef _MSC_VER

namespace MiniDump
{
    Result Write(const char *, void *exception_ptrs, const UserData *, size_t ) { return Result::NotAvailable; }

    void Startup() {}
    void Shutdown() {}

    const char *ResultMessage(Result code) { return "MiniDump: unsupported platform"; }
}

#else

#include <windows.h>
#include <winnt.h>
#include <DbgHelp.h>
#include <time.h>

#ifndef ARCH_X64
#   define HANDLE_VECTORED_EXCEPTION
#endif

namespace Core {
namespace MiniDump { namespace
{
    typedef BOOL (WINAPI *MiniDumpWriteDump_t)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
        PMINIDUMP_CALLBACK_INFORMATION CallbackParam
        );

    static AtomicSpinLock gMiniDumpBarrier;
    static AtomicSpinLock gMiniDumpWritten;
    static volatile MiniDumpWriteDump_t gMiniDumpWriteDump = nullptr;

    bool RetrieveDbgHelp_()
    {
        if (nullptr != gMiniDumpWriteDump)
            return true;

        HMODULE hModule = ::LoadLibraryA("DbgHelp.dll");
        if ( NULL == hModule )
            return false;

        gMiniDumpWriteDump = ( MiniDumpWriteDump_t )GetProcAddress(hModule, "MiniDumpWriteDump");
        if ( NULL == gMiniDumpWriteDump )
            return false;

        return true;
    }

    // this allocator is not thread safe but exclusive access is guaranteed through the critical section
    static size_t   gFailsafeUsedSize = 0;
    static uint8_t  gFailsafePage[16*1024]; // 16 Kbytes reserved to dodge dynamic and stack allocations

    template <typename T>
    struct FailsafeAlloca_
    {
        size_t  size;
        T       *ptr;

        operator T * () const { return ptr; }

        explicit FailsafeAlloca_(size_t size) : size(size), ptr(nullptr)
        {
            const size_t sizeInBytes = size*sizeof(T);
            if ( gFailsafeUsedSize + sizeInBytes <= ARRAYSIZE(gFailsafePage) )
            {
                ptr = reinterpret_cast<T *>(&gFailsafePage[gFailsafeUsedSize]);
                gFailsafeUsedSize += sizeInBytes;
            }
        }

        ~FailsafeAlloca_()
        {
            const size_t sizeInBytes = size*sizeof(T);
            if ( nullptr != ptr && reinterpret_cast<uint8_t *>(ptr) == &gFailsafePage[gFailsafeUsedSize - sizeInBytes] )
            {
                gFailsafeUsedSize -= sizeInBytes;
            }
        }
    };

    void OutputDebugFormatA_(const char *fmt, ...)
    {
        const FailsafeAlloca_<char> buffer(2048);
        va_list args;
        va_start(args, fmt);
        if ( 0 < sprintf_s(buffer.ptr, buffer.size, fmt, args) )
            ::OutputDebugStringA(buffer.ptr);
        va_end(args);

    }

    void OutputDebugFormatW_(const wchar_t *fmt, ...)
    {
        const FailsafeAlloca_<wchar_t> buffer(2048);
        va_list args;
        va_start(args, fmt);
        if ( 0 < vswprintf(buffer.ptr, buffer.size, fmt, args) )
            ::OutputDebugStringW(buffer.ptr);
        va_end(args);
    }

    static const char *gIgnoredModules[] =
    {
        "ADVAPI32.dll",
        "bcryptPrimitives.dll",
        "CRYPTBASE.dll",
        "dbgcore.DLL",
        "DbgHelp.dll",
        "RPCRT4.dll",
        "sechost.dll",
        "SspiCli.dll",
    };

    bool IsModuleIncluded_(HMODULE module)
    {
        // Test is the module belongs to current process
        const HMODULE currentModule = ::GetModuleHandle( NULL );
        if ( module == currentModule )
            return true;

        // Retrieve module basename
        FailsafeAlloca_<char> filename(2048);
        if ( 0 == ::GetModuleFileNameA(module, filename.ptr, checked_cast<DWORD>(filename.size) ) )
            return false;

        const char *basename = strrchr(filename, '\\');
        basename = ( nullptr == basename ) ? filename : basename+1;

        // Check if the module has been whitelisted
        for(size_t i = 0; i < ARRAYSIZE(gIgnoredModules); ++i)
            if ( 0 == _stricmp(gIgnoredModules[i], basename) )
            {
                OutputDebugFormatA_("%s: ignore module for minidump\n", filename.ptr);
                return false;
            }

        return true;
    }

    struct FilterCallbackContext_
    {
        const MemoryLocation    *MemoryBegin;
        const MemoryLocation    *MemoryEnd;
        bool                    MemoryCallbackCalled;
    };

    BOOL CALLBACK FilterCallback_(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
    {
        BOOL bResult = FALSE;

        FilterCallbackContext_& context = *reinterpret_cast<FilterCallbackContext_ *>(pParam);

        switch ( pInput->CallbackType )
        {
        case IncludeModuleCallback:
            // enable/disable entirely a module
            bResult = IsModuleIncluded_((HMODULE)pInput->IncludeModule.BaseOfImage ) ? TRUE : FALSE;
            break;

        case IncludeThreadCallback:
            // enable/disable entirely a thread
            bResult = TRUE;
            break;

        case ModuleCallback:
            // enable/disable specific data on enabled modules
            if ( ::GetModuleHandle( NULL ) == (HMODULE)pInput->Module.BaseOfImage || // current process ?
                 nullptr != ::wcsstr(pInput->Module.FullPath, L"ntdll.dll")          // ntdll ?
                )
            {
                OutputDebugFormatW_(L"%s: full module informations\n", pInput->Module.FullPath);
                pOutput->ModuleWriteFlags = ModuleWriteModule
                                          | ModuleWriteMiscRecord
                                          | ModuleWriteCvRecord
                                          | ModuleWriteDataSeg
                                          | ModuleWriteTlsData
                                          | ModuleReferencedByMemory;
            }
            else // or included module
            {
                OutputDebugFormatW_(L"%s: partial module informations\n", pInput->Module.FullPath);
                pOutput->ModuleWriteFlags = ModuleWriteModule
                                          | ModuleWriteMiscRecord
                                          | ModuleWriteCvRecord
                                          | ModuleReferencedByMemory;
            }
            bResult = TRUE;
            break;

        case ThreadCallback:
            // Include all available information
            bResult = TRUE;
            break;

        case ThreadExCallback:
            // Include all available information
            bResult = TRUE;
            break;

        case MemoryCallback:
            // Include user defined memory locations
            context.MemoryCallbackCalled = true;
            if ( context.MemoryBegin != context.MemoryEnd )
            {
                pOutput->MemoryBase = checked_cast<ULONG64>(uintptr_t(context.MemoryBegin->MemoryBase));
                pOutput->MemorySize = checked_cast<ULONG>(context.MemoryBegin->MemorySize);
                context.MemoryBegin++;
                return TRUE;
            }
            else
            {
                bResult = FALSE; // None added
            }
            break;

        case CancelCallback:
            if( !context.MemoryCallbackCalled )
            {
                // Continue receiving CancelCallback callbacks
                pOutput->Cancel       = FALSE;
                pOutput->CheckCancel  = TRUE;
            }
            else
            {
                // No cancel callbacks anymore
                pOutput->Cancel       = FALSE;
                pOutput->CheckCancel  = FALSE;
            }
            bResult = TRUE;
            break;

        default:
            // unhandled
            bResult = FALSE;
            break;
        }

        return bResult;
    }

} //!namespace
} //!namespace MiniDump
} //!namespace Core


namespace Core {
namespace MiniDump
{
    Result Write(   const char *filename, void *exception_ptrs,
                    const UserData *pdata, size_t dataCount,
                    const MemoryLocation *pmemory, size_t memoryCount )
    {
        if ( nullptr == filename )
            return Result::InvalidFilename;

        const AtomicSpinLock::Scope scopeLock(gMiniDumpBarrier);

        if ( false == RetrieveDbgHelp_() )
            return Result::NoDbgHelpDLL;

        // Try to open the file
        HANDLE hFile = ::CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if( ( hFile == NULL ) || ( hFile == INVALID_HANDLE_VALUE ) )
            return Result::CantCreateFile;

        // Minidump type
        MINIDUMP_TYPE miniDumpType = (MINIDUMP_TYPE)(
                MiniDumpNormal // default datas (always included)
            |   MiniDumpWithFullMemoryInfo // virtual memory layout of the process (!vadump, !vprot)
            |   MiniDumpWithThreadInfo // user/kernel time for each thread (.ttime)
            |   MiniDumpWithIndirectlyReferencedMemory // 1024 bytes around each referenced pointer (256 bytes before and 768 bytes after)
            |   MiniDumpWithHandleData // all handles in the process handle table at the moment of failure
            |   MiniDumpWithDataSegs // infos about globals vars (filtered)
            |   MiniDumpWithPrivateReadWriteMemory // stack, heap, TLS, PEB & TEBs (filtered)
            );

        // Exception information
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        memset( &mdei, 0, sizeof(mdei) );

        CONTEXT ctx; // used when exception_ptrs is null,
        EXCEPTION_POINTERS ep; // will contain information about the current thread

        if (nullptr == exception_ptrs)
        {
            DWORD dwThreadId = ::GetCurrentThreadId();

            memset( &ctx, 0, sizeof(ctx) );
            ctx.ContextFlags = CONTEXT_FULL;

            /*
            HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, dwThreadId );
            UNUSED(hThread);
            */

            memset( &ep, 0, sizeof(ep) );
            ep.ContextRecord = &ctx;
            ep.ExceptionRecord = NULL; // no exception

            mdei.ThreadId           = dwThreadId;
            mdei.ExceptionPointers  = &ep;
            mdei.ClientPointers     = FALSE;
        }
        else
        {
            mdei.ThreadId           = ::GetCurrentThreadId();
            mdei.ExceptionPointers  = (PEXCEPTION_POINTERS)exception_ptrs;
            mdei.ClientPointers     = FALSE;
        }

        // User datas (only first 8)
        MINIDUMP_USER_STREAM userStreamArray[8];
        memset( &userStreamArray, 0, sizeof(userStreamArray) );

        ULONG userStreamCount = 0;
        if (nullptr != pdata)
        {
            while ( userStreamCount < ARRAYSIZE(userStreamArray) && userStreamCount < dataCount )
            {
                const UserData& src = pdata[userStreamCount];
                ::MINIDUMP_USER_STREAM& dst = userStreamArray[userStreamCount];

                dst.Type        = checked_cast<ULONG32>(src.Type);
                dst.BufferSize  = checked_cast<ULONG>(src.BufferSize);
                dst.Buffer      = src.Buffer;

                userStreamCount++;
            }
        }

        MINIDUMP_USER_STREAM_INFORMATION musi;
        memset( &musi, 0, sizeof(musi) );
        musi.UserStreamArray    = userStreamArray;
        musi.UserStreamCount    = userStreamCount;

        // Minidump filter callback
        FilterCallbackContext_ context;
        context.MemoryBegin     = pmemory;
        context.MemoryEnd       = pmemory + memoryCount;
        context.MemoryCallbackCalled = false;

        MINIDUMP_CALLBACK_INFORMATION mci;
        memset( &mci, 0, sizeof(mci) );
        mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)&FilterCallback_;
        mci.CallbackParam       = &context;

        // Create the minidump
        HANDLE currentProcess = ::GetCurrentProcess();
        DWORD currentProcessId = ::GetCurrentProcessId();
        BOOL rv = gMiniDumpWriteDump(currentProcess, currentProcessId,
                                     hFile, miniDumpType, &mdei, &musi, &mci );

        Result res = ( TRUE == rv ) ? Result::Success : Result::DumpFailed;

        // Try to close the file
        if ( FALSE == ::CloseHandle(hFile) )
            res = ( Result::Success == res ) ? Result::FailedToCloseHandle : res;

        return res;
    }

    void Write(PEXCEPTION_POINTERS pExceptionInfo)
    {
        time_t tNow = ::time( NULL );
        struct tm pTm;
        localtime_s( &pTm, &tNow );

        FailsafeAlloca_<char> filename(2048);
        {
            FailsafeAlloca_<char> modulePath(2048);
            if (0 == ::GetModuleFileNameA(NULL, modulePath.ptr, checked_cast<DWORD>(modulePath.size)) )
                return;

            sprintf_s(filename.ptr, filename.size,
                "%s.%02d%02d%04d_%02d%02d%02d.dmp",
                modulePath.ptr,
                pTm.tm_year, pTm.tm_mon, pTm.tm_mday,
                pTm.tm_hour, pTm.tm_min, pTm.tm_sec);
        }

        MiniDump::Result result = MiniDump::Write(filename, pExceptionInfo, nullptr, 0, nullptr, 0);
    }

    struct ThreadToFiber_
    {
        LPVOID pCallee;
        PEXCEPTION_POINTERS pExceptionInfo;
    };

    VOID WINAPI OnUnhandledExceptionFiber_(LPVOID lpFiberParameter)
    {
        const ThreadToFiber_ *pThreadToFiber = (const ThreadToFiber_ *)lpFiberParameter;
        if (nullptr == pThreadToFiber)
            exit(42); // should never ever be called, but handled

        Write(pThreadToFiber->pExceptionInfo);

        ::SwitchToFiber(pThreadToFiber->pCallee);
    }

    static volatile LPTOP_LEVEL_EXCEPTION_FILTER gPreviousUnhandledExceptionFilter = nullptr;
    LONG WINAPI OnUnhandledException_(PEXCEPTION_POINTERS pExceptionInfo)
    {
        // no reentrancy (needed by ::MiniDumpWriteDump !)
        if (false == gMiniDumpWritten.TryLock())
            return EXCEPTION_EXECUTE_HANDLER;

        ThreadToFiber_ threadToFiber;
        threadToFiber.pExceptionInfo = pExceptionInfo;
        threadToFiber.pCallee = ::ConvertThreadToFiber( NULL );
        if (nullptr == threadToFiber.pCallee)
            return EXCEPTION_CONTINUE_SEARCH;

        // we'll use a fiber to write the minidump, since the current stack might be overrun (lighter than thread)
        LPVOID fiber = ::CreateFiber(512, &OnUnhandledExceptionFiber_, &threadToFiber);
        if (nullptr == fiber)
            return EXCEPTION_CONTINUE_SEARCH;

        ::SwitchToFiber(fiber);
        // here jump to forth and back in OnUnhandledExceptionFiber_
        ::ConvertFiberToThread();

        return EXCEPTION_EXECUTE_HANDLER;
    }

#ifdef HANDLE_VECTORED_EXCEPTION
    static volatile LPVOID gHandleVectoredExceptionHandler = nullptr;
    LONG WINAPI OnVectoredHandler_(PEXCEPTION_POINTERS pExceptionInfo)
    {
        // no reentrancy (needed by ::MiniDumpWriteDump !)
        if (false == gMiniDumpWritten.TryLock())
            return EXCEPTION_EXECUTE_HANDLER;

        Write(pExceptionInfo);
        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif

    void Start()
    {
        if (!RetrieveDbgHelp_())
            return;

#ifdef HANDLE_VECTORED_EXCEPTION
        gHandleVectoredExceptionHandler = ::AddVectoredExceptionHandler(0, &OnVectoredHandler_);
#endif

        gPreviousUnhandledExceptionFilter = ::SetUnhandledExceptionFilter(&OnUnhandledException_);
    }

    void Shutdown()
    {
        if (nullptr != gPreviousUnhandledExceptionFilter)
            ::SetUnhandledExceptionFilter(gPreviousUnhandledExceptionFilter);

#ifdef HANDLE_VECTORED_EXCEPTION
        if (nullptr != gHandleVectoredExceptionHandler)
            ::RemoveVectoredExceptionHandler(gHandleVectoredExceptionHandler);
#endif
    }
}//!namespace MiniDump
}//!namespace Core

#endif //!_MSC_VER

namespace Core {
namespace MiniDump
{
    const char *ResultMessage(Result code)
    {
        switch (code)
        {
        case MiniDump::Result::Success:
            return "minidump was successfully created";
        case MiniDump::Result::NoDbgHelpDLL:
            return "minidump failed to retrieve dbghelp.dll";
        case MiniDump::Result::InvalidFilename:
            return "minidump filename is invalid";
        case MiniDump::Result::CantCreateFile:
            return "minidump file could not be created";
        case MiniDump::Result::DumpFailed:
            return "failed to create a minidump";
        case MiniDump::Result::FailedToCloseHandle:
            return "minidump file was not closed correctly";
        case MiniDump::Result::NotAvailable:
            return "minidump is not available on this platform";
        default:
            return "invalid minidump result code";
        }
    }
} //!namespace MiniDump
} //!namespace Core
