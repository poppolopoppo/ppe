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
struct FWindowsPlatformProcess : FGenericPlatformProcess {
public:
    STATIC_CONST_INTEGRAL(bool, HasSemaphore, true);
    STATIC_CONST_INTEGRAL(bool, HasSpawnProcess, true);

    //------------------------------------------------------------------------
    // platform hooks for process start/shutdown

    static PPE_CORE_API void OnProcessStart(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t* const* argv );
    static PPE_CORE_API void OnProcessShutdown();

    //------------------------------------------------------------------------
    // sleep

    static void Sleep(float seconds) NOEXCEPT {
        const ::DWORD milliseconds = (::DWORD)(seconds * 1000.0f);
        if (milliseconds == 0)
            ::SwitchToThread();
        else
            ::Sleep(milliseconds);
    }

    static void SleepForSpinning(i32& backoff) NOEXCEPT {
    #if 0
        // Hybrid spinning
        // http://www.1024cores.net/home/lock-free-algorithms/tricks/spinning
        if (backoff < 10)
            ::_mm_pause();
            // (1) improve performance (help to fight memory ordering issues inside of a processor),
            // (2) decrease power consumption,
            // (3) further improve performance in the context of HyperThreading/Simultaneous Multithreading/Chip-level Multithreading
        else if (backoff < 20)
            for (int i = 0; i != 50; i += 1) ::_mm_pause();
        else if (backoff < 22)
            ::SwitchToThread(); // limited to the current processor
        else if (backoff < 24)
            ::Sleep(0); //  limited to threads of no-less priority
        else if (backoff < 26)
            ::Sleep(1); // all threads
        else
            ::Sleep(10); // all threads

        ++backoff;
    #else
        // exponential backoff
        // https://github.com/marcgh/intel-tbb/blob/master/include/tbb/tbb_machine.h#L349
        STATIC_CONST_INTEGRAL(i32, LoopsBeforeExp, 4);
        STATIC_CONST_INTEGRAL(i32, LoopsBeforeYield, 50);
        STATIC_CONST_INTEGRAL(i32, LoopsBeforeSleep, 150);
        //STATIC_CONST_INTEGRAL(i32, LoopsBeforeStall, 2000);

        ++backoff;

        if (Likely(backoff <= LoopsBeforeExp)) {
            ::_mm_pause();
        }
        else if (backoff <= LoopsBeforeYield) {
            forrange(i, 0, backoff)
                ::_mm_pause();
            // Pause twice as long the next time.
            backoff *= 2;
        }
        else if (backoff < LoopsBeforeSleep) {
            // Pause is so long that we might as well yield CPU to scheduler.
            ::_mm_pause();
            ::SwitchToThread();
        }
        else { //if (backoff < LoopsBeforeStall) {
            // Limited to threads of no-less priority
            ::_mm_pause();
            ::Sleep(0);
        }/*
        else {
            // Don't yield but sleep to ensure that the thread is not
            // immediately run again in case scheduler's run queue is empty
            using namespace std::chrono;
            std::this_thread::sleep_for(500us);
        }*/
    #endif
    }

    //------------------------------------------------------------------------
    // system process

    using FProcessId = ::DWORD;
    using FProcessHandle = ::HANDLE;

    static FProcessId CurrentPID() { return ::GetCurrentProcessId(); }
    static FProcessHandle CurrentProcess() { return ::GetCurrentProcess(); }

    static PPE_CORE_API void Daemonize(); // make process run as system service
    static PPE_CORE_API bool EnableDebugPrivileges(); // ask for elevated access

    static PPE_CORE_API bool IsForeground(); // returns true if this app is visible and selected
    static PPE_CORE_API bool IsFirstInstance(); // returns false if the same process is already running

    static PPE_CORE_API bool IsProcessAlive(FProcessHandle process);

    static PPE_CORE_API FProcessHandle OpenProcess(FProcessId pid, bool fullAccess = true);

    static PPE_CORE_API void WaitForProcess(FProcessHandle process); // stall until process is closed
    static PPE_CORE_API bool WaitForProcess(FProcessHandle process, size_t timeoutMs);

    static PPE_CORE_API void CloseProcess(FProcessHandle process);
    static PPE_CORE_API void TerminateProcess(FProcessHandle process, bool killTree);

    static PPE_CORE_API bool FindByName(FProcessId* pPid, const FWStringView& name);

    //------------------------------------------------------------------------
    // process infos

    using FAffinityMask = FWindowsPlatformThread::FAffinityMask;
    using FMemoryStats = FWindowsPlatformMemory::FStats;

    static const FAffinityMask& AllCoresAffinity;

    static PPE_CORE_API bool ExitCode(int* pExitCode, FProcessHandle process);

    static PPE_CORE_API bool MemoryStats(FMemoryStats* pStats, FProcessHandle process);

    static PPE_CORE_API bool Name(FString* pName, FProcessHandle process);

    static PPE_CORE_API bool Pid(FProcessId* pPid, FProcessHandle process);

    static PPE_CORE_API bool Priority(EProcessPriority* pPriority, FProcessHandle process);
    static PPE_CORE_API bool SetPriority(FProcessHandle process, EProcessPriority priority);

    static PPE_CORE_API FAffinityMask AffinityMask(FProcessHandle process);
    static PPE_CORE_API bool SetAffinityMask(FProcessHandle process, FAffinityMask mask);

    //------------------------------------------------------------------------
    // pipe

    using FPipeHandle = ::HANDLE;

    static PPE_CORE_API bool IsPipeBlocked(FPipeHandle pipe);
    static PPE_CORE_API bool CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite, bool shareRead);
    static PPE_CORE_API size_t PeekPipe(FPipeHandle read);
    static PPE_CORE_API size_t ReadPipe(FPipeHandle read, const FRawMemory& buffer);
    static PPE_CORE_API size_t WritePipe(FPipeHandle write, const FRawMemoryConst& buffer);
    static PPE_CORE_API void ClosePipe(FPipeHandle read, FPipeHandle write);

    //------------------------------------------------------------------------
    // spawn process

    static PPE_CORE_API FProcessHandle CreateProcess(
        FProcessId* pPID,
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir,
        bool detached, bool hidden, bool inheritHandles, bool noWindow,
        EProcessPriority priority,
        FPipeHandle hStdin = nullptr,
        FPipeHandle hStderr = nullptr,
        FPipeHandle hStdout = nullptr );

    static PPE_CORE_API bool ExecDetachedProcess(
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir );

    static PPE_CORE_API bool ExecElevatedProcess(
        int* pReturnCode,
        const wchar_t* executable,
        const wchar_t* parameters,
        const wchar_t* workingDir );

    static PPE_CORE_API bool OpenURL(const wchar_t* url); // launch external internet browser
    static PPE_CORE_API bool OpenWithDefaultApp(const wchar_t* filename); // open with viewer associated to this file type
    static PPE_CORE_API bool EditWithDefaultApp(const wchar_t* filename); // open with editor associated to this file type

    //------------------------------------------------------------------------
    // semaphore

    using FSemaphore = ::HANDLE;

    static PPE_CORE_API FSemaphore CreateSemaphore(const char* name, bool create, size_t maxLocks);
    static PPE_CORE_API void LockSemaphore(FSemaphore semaphore);
    static PPE_CORE_API bool TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds);
    static PPE_CORE_API void UnlockSemaphore(FSemaphore semaphore);
    static PPE_CORE_API bool DestroySemaphore(FSemaphore semaphore);

    //------------------------------------------------------------------------
    // dynamic library helpers (DLL)

    using FDynamicLibraryHandle = ::HMODULE;

    static PPE_CORE_API FDynamicLibraryHandle AttachToDynamicLibrary(const wchar_t* name);
    static PPE_CORE_API void DetachFromDynamicLibrary(FDynamicLibraryHandle lib);

    static PPE_CORE_API FDynamicLibraryHandle OpenDynamicLibrary(const wchar_t* name);
    static PPE_CORE_API void CloseDynamicLibrary(FDynamicLibraryHandle lib);

    static PPE_CORE_API FWString DynamicLibraryFilename(FDynamicLibraryHandle lib);
    static PPE_CORE_API void* DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
