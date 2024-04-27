// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformThread.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformDebug.h"
#include "HAL/Windows/WindowsPlatformMisc.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// #TODO: replace affinity mask by cpu sets, affinity mask tamper with work scheduling on Windows!
// see p.58 https://gpuopen.com/gdc-presentations/2024/GDC2024_AMD_Ryzen_Processor_Software_Optimization.pdf
//----------------------------------------------------------------------------
// Windows uses a special API for CPUs with more than 64 cores
// https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-setthreadaffinitymask
STATIC_ASSERT(PPE_MAX_NUMCPUCORE <= 64);
//----------------------------------------------------------------------------
static FWindowsPlatformThread::FAffinityMask LogicalAffinityMask_(size_t coreMask) {
    const size_t numCores = FWindowsPlatformMisc::NumCores();
    const size_t numThreads = FWindowsPlatformMisc::NumCoresWithSMT();
    Assert_NoAssume(numCores <= numThreads);

    // handles hyper threading (HT) :
    //
    //  CORE#0 | CORE#1 | CORE#2 | CORE#3 | PHYSICAL CORES
    //  ------------------------------------------------------
    //   PU#0  |  PU#1  |  PU#2  |  PU#3  | LOGICAL THREADS
    //   PU#4  |  PU#5  |  PU#6  |  PU#7  |

    FWindowsPlatformThread::FAffinityMask m = coreMask & (
        (FWindowsPlatformThread::FAffinityMask(1) << numCores) - 1);
    return ((numCores < numThreads)
        ? m | (m << (numThreads - numCores))
        : m );
}
//----------------------------------------------------------------------------
static FWindowsPlatformThread::FAffinityMask FetchAllCoresAffinityMask_() NOEXCEPT {
    using affinity_t = FWindowsPlatformThread::FAffinityMask;

    const affinity_t numCores = checked_cast<affinity_t>(FWindowsPlatformMisc::NumCoresWithSMT());
    Assert(numCores <= PPE_MAX_NUMCPUCORE);

    return (numCores == PPE_MAX_NUMCPUCORE
        ? affinity_t(-1)
        : (affinity_t(1) << numCores) - affinity_t(1));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FWindowsPlatformThread::FAffinityMask FWindowsPlatformThread::AllCoresAffinity{
     FetchAllCoresAffinityMask_() };
//----------------------------------------------------------------------------
void FWindowsPlatformThread::OnThreadStart() {
#if USE_PPE_PLATFORM_DEBUG
    FWindowsPlatformDebug::GuaranteeStackSizeForStackOverflowRecovery();
#endif
}
//----------------------------------------------------------------------------
void FWindowsPlatformThread::OnThreadShutdown() {
    NOOP(); // #TODO ?
}
//----------------------------------------------------------------------------
FWindowsPlatformThread::FAffinityMask FWindowsPlatformThread::MainThreadAffinity() {
    return LogicalAffinityMask_(static_cast<FAffinityMask>(1)); // only the first core, with HT
}
//----------------------------------------------------------------------------
FWindowsPlatformThread::FAffinityMask FWindowsPlatformThread::SecondaryThreadAffinity() {
    return LogicalAffinityMask_(~static_cast<FAffinityMask>(1)); // all but first core, with HT
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::AffinityMask() -> FAffinityMask {
    const ::HANDLE hThread = ::GetCurrentThread();

    ::DWORD_PTR affinityMask;

    affinityMask = ::SetThreadAffinityMask(hThread, checked_cast<::DWORD_PTR>(AllCoresAffinity));
    PPE_CLOG(0 == affinityMask, HAL, Fatal, "SetThreadAffinityMask({0:#16x}) failed with = {1}", AllCoresAffinity, FLastError());

    const FAffinityMask result = checked_cast<FAffinityMask>(affinityMask);

    affinityMask = ::SetThreadAffinityMask(hThread, affinityMask);
    PPE_CLOG(0 == affinityMask, HAL, Fatal, "SetThreadAffinityMask({0:#16x}) failed with = {1}", affinityMask, FLastError());

    return result;
}
//----------------------------------------------------------------------------
void FWindowsPlatformThread::SetAffinityMask(FAffinityMask mask) {
    Assert_NoAssume((mask & ~AllCoresAffinity) == 0);

    const ::HANDLE hThread = ::GetCurrentThread();

    const ::DWORD_PTR affinityMask = ::SetThreadAffinityMask(hThread, (::DWORD_PTR)mask);
    PPE_CLOG(0 == affinityMask, HAL, Fatal, "SetThreadAffinityMask({0:#16x}) failed with = {1}", affinityMask, FLastError());

#if USE_PPE_ASSERT
    const FAffinityMask actualMask = FWindowsPlatformThread::AffinityMask();
    Assert((mask & AllCoresAffinity) == actualMask);
#endif
    Unused(affinityMask);
}
//----------------------------------------------------------------------------
EThreadPriority FWindowsPlatformThread::Priority() {
    const HANDLE hThread = ::GetCurrentThread();

    switch (::GetThreadPriority(hThread)) {
    case THREAD_PRIORITY_TIME_CRITICAL:
        return EThreadPriority::Realtime;
    case THREAD_PRIORITY_HIGHEST:
        return EThreadPriority::Highest;
    case THREAD_PRIORITY_ABOVE_NORMAL:
        return EThreadPriority::AboveNormal;
    case THREAD_PRIORITY_NORMAL:
        return EThreadPriority::Normal;
    case THREAD_PRIORITY_BELOW_NORMAL:
        return EThreadPriority::BelowNormal;
    case THREAD_PRIORITY_LOWEST:
        return EThreadPriority::Lowest;
    case THREAD_PRIORITY_IDLE:
        return EThreadPriority::Idle;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformThread::SetPriority(EThreadPriority priority) {
    const ::HANDLE hThread = ::GetCurrentThread();

    int priorityWin32 = 0;
    switch (priority) {
    case PPE::EThreadPriority::Realtime:
        priorityWin32 = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    case PPE::EThreadPriority::Highest:
        priorityWin32 = THREAD_PRIORITY_HIGHEST;
        break;
    case PPE::EThreadPriority::AboveNormal:
        priorityWin32 = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case PPE::EThreadPriority::Normal:
        priorityWin32 = THREAD_PRIORITY_NORMAL;
        break;
    case PPE::EThreadPriority::BelowNormal:
        priorityWin32 = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case PPE::EThreadPriority::Lowest:
        priorityWin32 = THREAD_PRIORITY_LOWEST;
        break;
    case PPE::EThreadPriority::Idle:
        priorityWin32 = THREAD_PRIORITY_IDLE;
        break;
    }

    Verify(::SetThreadPriority(hThread, priorityWin32));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Fibers hack
//----------------------------------------------------------------------------
void FWindowsPlatformThread::FiberStackRegion(FFiber fiber, const void** pStackBottom, size_t* pStackSize) NOEXCEPT {
    // reference: https://github.com/edubart/minicoro/commit/e2fd1dd219f725b2da58e9533fa4e1b59038a3ab
    /* Reverse engineered Fiber struct, used to get stack base. */
    typedef struct _win32_fiber_t {
        LPVOID param;                /* fiber param */
        void* except;                /* saved exception handlers list */
        void* stack_top;             /* top of fiber stack */
        void* stack_limit;           /* fiber stack low-water mark */
        void* stack_allocation;      /* base of the fiber stack allocation */
        CONTEXT context;             /* fiber context */
        DWORD flags;                 /* fiber flags */
        LPFIBER_START_ROUTINE start; /* start routine */
        void** fls_slots;            /* fiber storage slots */
    }   _win32_fiber_t;

    _win32_fiber_t* const pWin32Fiber = static_cast<_win32_fiber_t*>(fiber);

    *pStackBottom = pWin32Fiber->stack_limit;
    *pStackSize = checked_cast<size_t>(uintptr_t(pWin32Fiber->stack_top) - uintptr_t(pWin32Fiber->stack_limit));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Task groups
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::BackgroundThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::Lowest;
    info.NumWorkers = Clamp(1_size_t, 4_size_t, FWindowsPlatformMisc::NumCores() / 2);
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = SecondaryThreadAffinity();
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::GlobalThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::Normal;
    info.NumWorkers = Max(FWindowsPlatformMisc::NumCores() - 1_size_t, 1_size_t);
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = LogicalAffinityMask_(1ull << (i + 1ul));
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::HighPriorityThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::AboveNormal;
    info.NumWorkers = Min(FWindowsPlatformMisc::NumCoresWithSMT(), size_t(PPE_MAX_NUMCPUCORE));
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = FAffinityMask(1ull << i);
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::IOThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::Highest; // highest priority to be resumed asap
    info.NumWorkers = 2; // IO should be operated in 2 threads max to prevent slow seeks
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = FAffinityMask(~1ul) & AllCoresAffinity; // all but first *thread*, let IO overlap on main thread with HT
    return info;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
