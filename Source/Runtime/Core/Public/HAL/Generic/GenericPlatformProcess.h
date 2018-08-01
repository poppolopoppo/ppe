#pragma once

#include "HAL/TargetPlatform.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_API FGenericPlatformProcess {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSemaphore, false);
    STATIC_CONST_INTEGRAL(bool, HasSpawnProcess, false);

    //------------------------------------------------------------------------
    // platform hooks for process start/shutdown

    static void OnProcessStart() = delete;
    static void OnProcessShutdown() = delete;

    //------------------------------------------------------------------------
    // sleep

    static void Sleep(float seconds) = delete;
    static void SleepInfinite() = delete;
    static void SleepForSpinning(size_t& backoff) = delete;

    //------------------------------------------------------------------------
    // system process

    using FProcessId = size_t;
    using FProcessHandle = void*;

    static FProcessId CurrentPID() = delete;
    static void Daemonize() = delete; // make process run as system service

    static bool IsForeground() = delete; // returns true if this app is visible and selected
    static bool IsFirstInstance() = delete; // returns false if the same process is already running

    static bool IsProcessAlive(FProcessId pid) = delete;
    static bool IsProcessAlive(FProcessHandle process) = delete;

    static FProcessHandle OpenProcess(FProcessId pid) = delete;
    static void WaitForProcess(FProcessHandle process) = delete; // stall until process is closed
    static void CloseProcess(FProcessHandle process) = delete;
    static void TerminateProcess(FProcessHandle process, bool killTree) = delete;

    static FString ProcessName(FProcessId pid) = delete;
    static u64 ProcessMemoryUsage(FProcessId pid) = delete;

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = void*;

    static bool CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite) = delete;
    static bool ReadPipe(FPipeHandle read, FStringBuilder& buffer) = delete;
    static bool WritePipe(FPipeHandle write, const FStringView& buffer) = delete;
    static void ClosePipe(FPipeHandle read, FPipeHandle write) = delete;

    //------------------------------------------------------------------------
    // spawn process

    static FProcessHandle CreateProcess(
        FProcessId* pPID,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        bool detached, bool hidden, bool noWindow, int priority,
        const FWStringView& optionalWorkingDir,
        FPipeHandle readPipe = nullptr,
        FPipeHandle writePipe = nullptr ) = delete;

    static bool ExecProcess(
        int* pReturnCode,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        FStringBuilder* pStdout = nullptr,
        FStringBuilder* pStderr = nullptr ) = delete;

    static bool ExecElevatedProcess(
        int* pReturnCode,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args ) = delete;

    static bool ExecDetachedProcess(const FWStringView& commandLine) = delete;

    static bool OpenURL(const FWStringView& url) = delete; // launch external internet browser
    static bool OpenWithDefaultApp(const FWStringView& filename) = delete; // open with viewer associated to this file type
    static bool EditWithDefaultApp(const FWStringView& filename) = delete; // open with editor associated to this file type

    //------------------------------------------------------------------------
    // semaphore

    using FSemaphore = void*;

    static FSemaphore CreateSemaphore(const char* name, bool create, size_t maxLocks) = delete;
    static void LockSemaphore(FSemaphore semaphore) = delete;
    static bool TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds) = delete;
    static void UnlockSemaphore(FSemaphore semaphore) = delete;
    static bool DestroySemaphore(FSemaphore semaphore) = delete;

    //------------------------------------------------------------------------
    // dynamic library helpers (DLL)

    using FDynamicLibraryHandle = void*;

    static FDynamicLibraryHandle AttachToDynamicLibrary(const FWStringView& name) = delete;
    static void DetachFromDynamicLibrary(FDynamicLibraryHandle lib) = delete;

    static FDynamicLibraryHandle OpenDynamicLibrary(const FWStringView& name) = delete;
    static void CloseDynamicLibrary(FDynamicLibraryHandle lib) = delete;

    static FWString DynamicLibraryFilename(FDynamicLibraryHandle lib) = delete;
    static void* DynamicLibraryFunction(FDynamicLibraryHandle lib, const FStringView& name) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
