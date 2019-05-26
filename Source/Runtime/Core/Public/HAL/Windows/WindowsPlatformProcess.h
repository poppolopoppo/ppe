#pragma once

#include "HAL/Generic/GenericPlatformProcess.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformProcess : FGenericPlatformProcess {
public:
    STATIC_CONST_INTEGRAL(bool, HasSemaphore, true);
    STATIC_CONST_INTEGRAL(bool, HasSpawnProcess, true);

    //------------------------------------------------------------------------
    // platform hooks for process start/shutdown

    static void OnProcessStart();
    static void OnProcessShutdown();

    //------------------------------------------------------------------------
    // sleep

    static void Sleep(float seconds);
    static void SleepInfinite();
    static void SleepForSpinning(size_t& backoff);

    //------------------------------------------------------------------------
    // system process

    using FProcessId = ::DWORD;
    using FProcessHandle = ::HANDLE;

    static FProcessId CurrentPID();
    static void Daemonize(); // make process run as system service

    static EProcessPriority Priority();
    static void SetPriority(EProcessPriority priority);

    static bool IsFirstInstance(); // returns false if the same process is already running

    static bool IsProcessAlive(FProcessId pid);
    static bool IsProcessAlive(FProcessHandle process);

    static FProcessHandle OpenProcess(FProcessId pid);
    static void WaitForProcess(FProcessHandle process); // stall until process is closed
    static void CloseProcess(FProcessHandle process);
    static void TerminateProcess(FProcessHandle process, bool killTree);

    static FString ProcessName(FProcessId pid);
    static u64 ProcessMemoryUsage(FProcessId pid);

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = ::HANDLE;

    static bool CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite);
    static bool ReadPipe(FPipeHandle read, FStringBuilder& buffer);
    static bool WritePipe(FPipeHandle write, const FStringView& buffer);
    static void ClosePipe(FPipeHandle read, FPipeHandle write);

    //------------------------------------------------------------------------
    // spawn process

    static FProcessHandle CreateProcess(
        FProcessId* pPID,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        bool detached, bool hidden, bool noWindow, int priority,
        const FWStringView& optionalWorkingDir,
        FPipeHandle readPipe = nullptr,
        FPipeHandle writePipe = nullptr );

    static bool ExecProcess(
        int* pReturnCode,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        FStringBuilder* pStdout = nullptr,
        FStringBuilder* pStderr = nullptr );

    static bool ExecElevatedProcess(
        int* pReturnCode,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args );

    static bool ExecDetachedProcess(const FWStringView& commandLine);

    static bool OpenURL(const FWStringView& url); // launch external internet browser
    static bool OpenWithDefaultApp(const FWStringView& filename); // open with viewer associated to this file type
    static bool EditWithDefaultApp(const FWStringView& filename); // open with editor associated to this file type

    //------------------------------------------------------------------------
    // semaphore

    using FSemaphore = ::HANDLE;

    static FSemaphore CreateSemaphore(const char* name, bool create, size_t maxLocks);
    static void LockSemaphore(FSemaphore semaphore);
    static bool TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds);
    static void UnlockSemaphore(FSemaphore semaphore);
    static bool DestroySemaphore(FSemaphore semaphore);

    //------------------------------------------------------------------------
    // dynamic library helpers (DLL)

    using FDynamicLibraryHandle = ::HMODULE;

    static FDynamicLibraryHandle AttachToDynamicLibrary(const wchar_t* name);
    static void DetachFromDynamicLibrary(FDynamicLibraryHandle lib);

    static FDynamicLibraryHandle OpenDynamicLibrary(const wchar_t* name);
    static void CloseDynamicLibrary(FDynamicLibraryHandle lib);

    static FWString DynamicLibraryFilename(FDynamicLibraryHandle lib);
    static void* DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
