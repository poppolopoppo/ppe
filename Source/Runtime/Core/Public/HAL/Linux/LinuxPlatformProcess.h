#pragma once

#include "HAL/Generic/GenericPlatformProcess.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformIncludes.h"
#include "HAL/Linux/LinuxPlatformMemory.h"
#include "HAL/Linux/LinuxPlatformThread.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLinuxProcessState : Meta::FNonCopyable {
public:
    enum state_t : u32 {
        kDefault    = 0,
        kRunning    = 1<<0,
        kWaitFor    = 1<<1,
        kFireForget = 1<<2,
    };

    FLinuxProcessState() NOEXCEPT : _pid(0), _state(kDefault), _exitCode(-1) {}
    ~FLinuxProcessState();

    explicit FLinuxProcessState(::pid_t pid, bool fireAndForget) NOEXCEPT;

    ::pid_t Pid() const { return _pid; }

    bool IsRunning();
    bool ExitCode(i32* pExitCode);
    void Wait();
    bool WaitFor(int timeoutMS);

private:
    friend struct FLinuxPlatformProcess;

    ::pid_t _pid;

    u32 _state;
    i32 _exitCode;
};
//----------------------------------------------------------------------------
struct FLinuxProcessHandle {
    FLinuxProcessState* ProcInfo;
    ::pid_t OpenedPid;

    FLinuxProcessHandle() NOEXCEPT : ProcInfo(nullptr), OpenedPid(-1) {}
    ~FLinuxProcessHandle() { Assert(nullptr == ProcInfo); }

    explicit FLinuxProcessHandle(FLinuxProcessState* procInfo) NOEXCEPT : ProcInfo(procInfo), OpenedPid(-1) {}
    explicit FLinuxProcessHandle(::pid_t pid) : ProcInfo(nullptr), OpenedPid(pid) {}

    bool Valid() const { return (nullptr != ProcInfo || -1 != OpenedPid); }
    ::pid_t Pid() const { return (ProcInfo ? ProcInfo->Pid() : OpenedPid); }

    void Reset() NOEXCEPT {
        Assert(ProcInfo);
        ProcInfo = nullptr;
        OpenedPid = -1;
    }
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformProcess : FGenericPlatformProcess {
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

    using FProcessId = ::pid_t;
    using FProcessHandle = FLinuxProcessHandle*;

    static FProcessId CurrentPID();
    static FProcessHandle CurrentProcess();

    static void Daemonize(); // make process run as system service
    static bool EnableDebugPrivileges(); // ask for elevated access

    static bool IsForeground(); // returns true if this app is visible and selected
    static bool IsFirstInstance(); // returns false if the same process is already running

    static bool IsProcessAlive(FProcessHandle& process);

    static FProcessHandle OpenProcess(FProcessId pid, bool fullAccess = true);

    static void WaitForProcess(FProcessHandle& process); // stall until process is closed
    static bool WaitForProcess(FProcessHandle& process, size_t timeoutMs);

    static void CloseProcess(FProcessHandle& process);
    static void TerminateProcess(FProcessHandle& process, bool killTree);

    static bool FindByName(FProcessId* pPid, const FWStringView& name);

    //------------------------------------------------------------------------
    // process infos

    using FAffinityMask = FLinuxPlatformThread::FAffinityMask;
    using FMemoryStats = FLinuxPlatformMemory::FStats;

    static const FAffinityMask& AllCoresAffinity;

    static bool ExitCode(int* pExitCode, FProcessHandle& process);

    static bool MemoryStats(FMemoryStats* pStats, FProcessHandle& process);

    static bool Name(FString* pName, FProcessHandle& process);

    static bool Pid(FProcessId* pPid, FProcessHandle& process);

    static bool Priority(EProcessPriority* pPriority, FProcessHandle& process);
    static bool SetPriority(FProcessHandle& process, EProcessPriority priority);

    static FAffinityMask AffinityMask(FProcessHandle& process);
    static bool SetAffinityMask(FProcessHandle& process, FAffinityMask mask);

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = void*;

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

    using FSemaphore = void*;

    static FSemaphore CreateSemaphore(const char* name, bool create, size_t maxLocks);
    static void LockSemaphore(FSemaphore semaphore);
    static bool TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds);
    static void UnlockSemaphore(FSemaphore semaphore);
    static bool DestroySemaphore(FSemaphore semaphore);

    //------------------------------------------------------------------------
    // dynamic library helpers (DLL)

    using FDynamicLibraryHandle = void*;

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

#endif //!PLATFORM_LINUX
