#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformThread.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformDebug.h"
#include "HAL/Linux/LinuxPlatformMisc.h"

#include "Allocator/TrackingMalloc.h"

#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ucontext.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FLinuxPlatformThread::FAffinityMask LogicalAffinityMask_(size_t coreMask) {
    const size_t numCores = FLinuxPlatformMisc::NumCores();
    const size_t numThreads = FLinuxPlatformMisc::NumCoresWithSMT();
    Assert_NoAssume(numCores <= numThreads);

    // handles hyper threading (HT) :
    //
    //  CORE#0 | CORE#1 | CORE#2 | CORE#3 | PHYSICAL CORES
    //  ------------------------------------------------------
    //   PU#0  |  PU#1  |  PU#2  |  PU#3  | LOGICAL THREADS
    //   PU#4  |  PU#5  |  PU#6  |  PU#7  |

    FLinuxPlatformThread::FAffinityMask m = coreMask & (
        (FLinuxPlatformThread::FAffinityMask(1) << numCores) - 1);
    return ((numCores < numThreads)
        ? m | (m << (numThreads - numCores))
        : m );
}
//----------------------------------------------------------------------------
static FLinuxPlatformThread::FAffinityMask FetchAllCoresAffinityMask_() NOEXCEPT {
    using affinity_t = FLinuxPlatformThread::FAffinityMask;

    const affinity_t numCores = checked_cast<affinity_t>(FLinuxPlatformMisc::NumCoresWithSMT());
    Assert(numCores <= PPE_MAX_NUMCPUCORE);

    return (numCores == PPE_MAX_NUMCPUCORE
        ? affinity_t(-1)
        : (affinity_t(1) << numCores) - affinity_t(1));
}
//----------------------------------------------------------------------------
static EThreadPriority TranslateThreadPriority_(i32 process, i32 thread) {
    // In general, the range is -20 to 19 (negative is highest, positive is lowest)
    const i32 off = (thread - process);
    if (off <= -20) return EThreadPriority::Realtime;
    if (off <= -15) return EThreadPriority::Highest;
    if (off <= -10) return EThreadPriority::AboveNormal;
    if (off <= 0) return EThreadPriority::Normal;
    if (off <= 5) return EThreadPriority::BelowNormal;
    if (off <= 10) return EThreadPriority::Lowest;

    return EThreadPriority::Idle;
}
//----------------------------------------------------------------------------
static i32 TranslateThreadPriority_(EThreadPriority priority) {
    // In general, the range is -20 to 19 (negative is highest, positive is lowest)
    i32 niceLevel = 0;
    switch (priority) {
    case EThreadPriority::Realtime: niceLevel = -20; break;
    case EThreadPriority::Highest: niceLevel = -15; break;
    case EThreadPriority::AboveNormal: niceLevel = -10; break;
    case EThreadPriority::Normal: niceLevel = 0; break;
    case EThreadPriority::BelowNormal: niceLevel = 5; break;
    case EThreadPriority::Lowest: niceLevel = 10; break;
    case EThreadPriority::Idle: niceLevel = 15; break; // total starvation !

    default:
        AssertNotImplemented();
    }

    // note: a non-privileged process can only go as low as RLIMIT_NICE
    return niceLevel;
}
//----------------------------------------------------------------------------
static i32 ThreadPriorityBaseLine_() {
    static bool GHasNiceBaseLine = false;

    if (not GHasNiceBaseLine) {
        // checking errno is necessary since -1 is a valid priority to return from getpriority()
        errno = 0;
        i32 currentPriority = ::getpriority(PRIO_PROCESS, ::getpid());

        // if getting priority wasn't successful, don't change the baseline value (will be 0 - i.e. normal - by default)
        if (currentPriority != -1 || errno == 0) {
            GHasNiceBaseLine = true;
            return currentPriority;
        }
    }

    return 0;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FLinuxPlatformThread::FAffinityMask FLinuxPlatformThread::AllCoresAffinity{
     FetchAllCoresAffinityMask_() };
//----------------------------------------------------------------------------
void FLinuxPlatformThread::OnThreadStart() {
#if USE_PPE_PLATFORM_DEBUG
    FLinuxPlatformDebug::GuaranteeStackSizeForStackOverflowRecovery();
#endif
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::OnThreadShutdown() {
    NOOP(); // #TODO ?
}
//----------------------------------------------------------------------------
FLinuxPlatformThread::FAffinityMask FLinuxPlatformThread::MainThreadAffinity() {
    return LogicalAffinityMask_(1ul); // only the first core, with HT
}
//----------------------------------------------------------------------------
FLinuxPlatformThread::FAffinityMask FLinuxPlatformThread::SecondaryThreadAffinity() {
    return LogicalAffinityMask_(~1ul); // all but first core, with HT
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::AffinityMask() -> FAffinityMask {
    ::cpu_set_t cpu_set;
    Verify(0 == ::pthread_getaffinity_np(::pthread_self(), sizeof(cpu_set), &cpu_set));

    FAffinityMask affinity{ 0 };
    forrange(i, 0, PPE_MAX_NUMCPUCORE)
        if (CPU_ISSET(i, &cpu_set))
            affinity |= FAffinityMask(1 << i);

    return affinity;
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::SetAffinityMask(FAffinityMask mask) {
    Assert_NoAssume((mask & ~AllCoresAffinity) == 0);

    ::cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    forrange(i, 0, PPE_MAX_NUMCPUCORE)
        if (mask & (1 << i))
            CPU_SET(i, &cpu_set);

    Verify(0 == ::pthread_setaffinity_np(::pthread_self(), sizeof(cpu_set), &cpu_set));
}
//----------------------------------------------------------------------------
EThreadPriority FLinuxPlatformThread::Priority() {
    int policy;
    struct ::sched_param sched_param;
    Verify(0 == ::pthread_getschedparam(::pthread_self(), &policy, &sched_param));

    return TranslateThreadPriority_(ThreadPriorityBaseLine_(), sched_param.sched_priority);
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::SetPriority(EThreadPriority priority) {
    const i32 niceLevel = TranslateThreadPriority_(priority);

    struct ::sched_param sched_param;
    sched_param.sched_priority = ThreadPriorityBaseLine_() + niceLevel;

    Verify(0 == ::pthread_setschedparam(::pthread_self(), SCHED_OTHER, &sched_param));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Fibers emulation
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FLinuxFiber {
    ucontext_t Context;
    void* FiberData;

    static THREAD_LOCAL FLinuxFiber Main;
    static THREAD_LOCAL FLinuxFiber* Running;
};
STATIC_ASSERT(std::is_pod_v<FLinuxFiber>);
THREAD_LOCAL FLinuxFiber FLinuxFiber::Main;
THREAD_LOCAL FLinuxFiber* FLinuxFiber::Running{ nullptr };
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::CurrentFiber() -> FFiber {
    return FLinuxFiber::Running;
}
//----------------------------------------------------------------------------
void* FLinuxPlatformThread::FiberData() {
    return FLinuxFiber::Running->FiberData;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::ConvertCurrentThreadToFiber() -> FFiber {
    Assert(nullptr == FLinuxFiber::Running);

    FFiber main = &FLinuxFiber::Main;
    main->FiberData = nullptr;
    Verify(0 == ::getcontext(&main->Context));

    FLinuxFiber::Running = main;

    return main;
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::RevertCurrentFiberToThread(FFiber threadFiber) {
    Assert(threadFiber);
    Assert(threadFiber == &FLinuxFiber::Main);
    Assert(threadFiber == FLinuxFiber::Running);

    FLinuxFiber::Running = nullptr;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::CreateFiber(
    size_t stackCommitSize,
    size_t stackReservedSize,
    FEntryPoint entryPoint,
    void* fiberData ) -> FFiber {
    Assert(entryPoint);
    Assert(stackReservedSize > stackCommitSize);
    UNUSED(stackCommitSize);

    FFiber fiber = (FFiber)TRACKING_MALLOC(Task, sizeof(FLinuxFiber) + stackReservedSize);

    Verify(0 == ::getcontext(&fiber->Context));
    fiber->FiberData = fiberData;
    fiber->Context.uc_link = nullptr;
    fiber->Context.uc_stack.ss_sp = (fiber + 1); // stack is after the context
    fiber->Context.uc_stack.ss_size = stackReservedSize;

    /* Some notes:
    - If a CPU needs stronger alignment for the stack than malloc()
    guarantees (like i.e. IA64) then makecontext() is supposed to
    add that alignment internally.
    - According to POSIX the arguments to function() are of type int
    and there are in fact 64-bit implementations which support only
    32 bits per argument meaning that a pointer argument has to be
    splitted into two arguments.
    - Most implementations interpret context.uc_stack.ss_sp on entry
    as the lowest stack address even if the CPU stack actually grows
    downwards. Although this means that ss_sp does NOT represent the
    CPU stack pointer this behaviour makes perfectly sense as it is
    the only way to stay independent from the CPU architecture. But
    Solaris prior to release 10 interprets ss_sp as highest stack
    address thus requiring special handling. */
    ::makecontext(&fiber->Context, (void(*)())entryPoint, 1, fiberData);

    return fiber;
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::SwitchToFiber(FFiber fiber) {
    FFiber yielding = FLinuxFiber::Running;
    FLinuxFiber::Running = fiber;
    Verify(0 == ::swapcontext(&yielding->Context, &fiber->Context));
}
//----------------------------------------------------------------------------
void FLinuxPlatformThread::DestroyFiber(FFiber fiber) {
    Assert(fiber);
    Assert(FLinuxFiber::Running != fiber);

    TRACKING_FREE(Task, fiber);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Task groups
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::BackgroundThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::Lowest;
    info.NumWorkers = Min(2ull, FLinuxPlatformMisc::NumCores() / 2ull);
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = SecondaryThreadAffinity();
    return info;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::GlobalThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::Normal;
    info.NumWorkers = Max(FLinuxPlatformMisc::NumCores() - 1ul, 1ul);
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = LogicalAffinityMask_(1ull << (i + 1ul));
    return info;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::HighPriorityThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.Priority = EThreadPriority::AboveNormal;
    info.NumWorkers = Min(FLinuxPlatformMisc::NumCoresWithSMT(), size_t(PPE_MAX_NUMCPUCORE));
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = FAffinityMask(1ull << i);
    return info;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformThread::IOThreadsInfo() -> FThreadGroupInfo {
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

#endif //!PLATFORM_LINUX
