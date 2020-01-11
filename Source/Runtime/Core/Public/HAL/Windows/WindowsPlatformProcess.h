#pragma once

#include "HAL/Generic/GenericPlatformProcess.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"
#include "HAL/Windows/WindowsPlatformMemory.h"
#include "HAL/Windows/WindowsPlatformThread.h"

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

    static void OnProcessStart(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t* const* argv );
    static void OnProcessShutdown();

    //------------------------------------------------------------------------
    // sleep

    static void Sleep(float seconds);
    static void SleepForSpinning(size_t& backoff);

    //------------------------------------------------------------------------
    // system process

    using FProcessId = ::DWORD;
    using FProcessHandle = ::HANDLE;

    static FProcessId CurrentPID();
    static FProcessHandle CurrentProcess();

    static void Daemonize(); // make process run as system service
    static bool EnableDebugPrivileges(); // ask for elevated access

    static bool IsForeground(); // returns true if this app is visible and selected
    static bool IsFirstInstance(); // returns false if the same process is already running

    static bool IsProcessAlive(FProcessHandle process);

    static FProcessHandle OpenProcess(FProcessId pid, bool fullAccess = true);

    static void WaitForProcess(FProcessHandle process); // stall until process is closed
    static bool WaitForProcess(FProcessHandle process, size_t timeoutMs);

    static void CloseProcess(FProcessHandle process);
    static void TerminateProcess(FProcessHandle process, bool killTree);

    static bool FindByName(FProcessId* pPid, const FWStringView& name);

    //------------------------------------------------------------------------
    // process infos

    using FAffinityMask = FWindowsPlatformThread::FAffinityMask;
    using FMemoryStats = FWindowsPlatformMemory::FStats;

    static const FAffinityMask& AllCoresAffinity;

    static bool ExitCode(int* pExitCode, FProcessHandle process);

    static bool MemoryStats(FMemoryStats* pStats, FProcessHandle process);

    static bool Name(FString* pName, FProcessHandle process);

    static bool Pid(FProcessId* pPid, FProcessHandle process);

    static bool Priority(EProcessPriority* pPriority, FProcessHandle process);
    static bool SetPriority(FProcessHandle process, EProcessPriority priority);

    static FAffinityMask AffinityMask(FProcessHandle process);
    static bool SetAffinityMask(FProcessHandle process, FAffinityMask mask);

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = ::HANDLE;

    static bool IsPipeBlocked(FPipeHandle pipe);
    static bool CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite, bool shareRead);
    static size_t PeekPipe(FPipeHandle read);
    static size_t ReadPipe(FPipeHandle read, const FRawMemory& buffer);
    static size_t WritePipe(FPipeHandle write, const FRawMemoryConst& buffer);
    static void ClosePipe(FPipeHandle read, FPipeHandle write);

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
        FPipeHandle hStdout = nullptr );

    static bool ExecDetachedProcess(
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir );

    static bool ExecElevatedProcess(
        int* pReturnCode,
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir );

    static bool OpenURL(const wchar_t* url); // launch external internet browser
    static bool OpenWithDefaultApp(const wchar_t* filename); // open with viewer associated to this file type
    static bool EditWithDefaultApp(const wchar_t* filename); // open with editor associated to this file type

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
