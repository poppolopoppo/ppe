#pragma once

#include "HAL/TargetPlatform.h"
#include "HAL/Generic/GenericPlatformMemory.h"
#include "HAL/Generic/GenericPlatformThread.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EProcessPriority {
    Realtime = 0,
    High,
    AboveNormal,
    Normal,
    BelowNormal,
    Idle,
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformProcess {
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
    static FProcessHandle CurrentProcess() = delete;

    static void Daemonize() = delete; // make process run as system service
    static bool EnableDebugPrivileges() = delete; // ask for elevated access

    static bool IsForeground() = delete; // returns true if this app is visible and selected
    static bool IsFirstInstance() = delete; // returns false if the same process is already running

    static bool IsProcessAlive(FProcessHandle process) = delete;

    static FProcessHandle OpenProcess(FProcessId pid, bool fullAccess = true) = delete;

    static void WaitForProcess(FProcessHandle process) = delete; // stall until process is closed
    static bool WaitForProcess(FProcessHandle process, size_t timeoutMs) = delete;

    static void CloseProcess(FProcessHandle process) = delete;
    static void TerminateProcess(FProcessHandle process, bool killTree) = delete;

    static bool FindByName(FProcessId* pPid, const FWStringView& name) = delete;

    //------------------------------------------------------------------------
    // process infos

    using FAffinityMask = FGenericPlatformThread::FAffinityMask;
    using FMemoryStats = FGenericPlatformMemoryStats;

    STATIC_CONST_INTEGRAL(FAffinityMask, AllCoresAffinity, FGenericPlatformThread::AllCoresAffinity);

    static bool ExitCode(int* pExitCode, FProcessHandle process) = delete;

    static bool MemoryStats(FMemoryStats* pStats, FProcessHandle process) = delete;

    static bool Name(FString* name, FProcessHandle process) = delete;

    static bool Pid(FProcessId* pPid, FProcessHandle process) = delete;

    static bool Priority(EProcessPriority* pPriority, FProcessHandle process) = delete;
    static bool SetPriority(FProcessHandle process, EProcessPriority priority) = delete;

    static FAffinityMask AffinityMask(FProcessHandle process) = delete;
    static bool SetAffinityMask(FProcessHandle process, FAffinityMask mask) = delete;

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = void*;

    static bool IsPipeBlocked(FPipeHandle pipe) = delete;
    static bool CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite, bool shareRead) = delete;
    static size_t PeekPipe(FPipeHandle read) = delete;
    static size_t ReadPipe(FPipeHandle read, const FRawMemory& buffer) = delete;
    static size_t WritePipe(FPipeHandle write, const FRawMemoryConst& buffer) = delete;
    static void ClosePipe(FPipeHandle read, FPipeHandle write) = delete;

    //------------------------------------------------------------------------
    // spawn process

    static FProcessHandle CreateProcess(
        FProcessId* pPID,
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir,
        bool detached, bool hidden, bool inheritHandles, bool noWindow,
        EProcessPriority priority,
        FPipeHandle hStdin = nullptr,
        FPipeHandle hStderr = nullptr,
        FPipeHandle hStdout = nullptr ) = delete;

    static bool ExecDetachedProcess(
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir ) = delete;

    static bool ExecElevatedProcess(
        int* pReturnCode,
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir ) = delete;

    static bool OpenURL(const wchar_t* url) = delete; // launch external internet browser
    static bool OpenWithDefaultApp(const wchar_t* filename) = delete; // open with viewer associated to this file type
    static bool EditWithDefaultApp(const wchar_t* filename) = delete; // open with editor associated to this file type

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

    static FDynamicLibraryHandle AttachToDynamicLibrary(const wchar_t* name) = delete;
    static void DetachFromDynamicLibrary(FDynamicLibraryHandle lib) = delete;

    static FDynamicLibraryHandle OpenDynamicLibrary(const wchar_t* name) = delete;
    static void CloseDynamicLibrary(FDynamicLibraryHandle lib) = delete;

    static FWString DynamicLibraryFilename(FDynamicLibraryHandle lib) = delete;
    static void* DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
