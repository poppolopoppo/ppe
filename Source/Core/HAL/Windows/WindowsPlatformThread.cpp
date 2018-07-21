#include "stdafx.h"

#include "WindowsPlatformThread.h"

#ifdef PLATFORM_WINDOWS

#include "LastError.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformThread::OnThreadStart() {
    NOOP(); // #TODO ?
}
//----------------------------------------------------------------------------
void FWindowsPlatformThread::OnThreadShutdown() {
    NOOP(); // #TODO ?
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::AffinityMask() -> FAffinityMask {
    const ::HANDLE hThread = ::GetCurrentThread();

    ::DWORD_PTR affinityMask;

    affinityMask = ::SetThreadAffinityMask(hThread, 0xFFul);
    CLOG(0 == affinityMask, HAL, Fatal, L"SetThreadAffinityMask({0:#16x}) failed with = {1}", 0xFF, FLastError());

    const FAffinityMask result = checked_cast<FAffinityMask>(affinityMask);

    affinityMask = ::SetThreadAffinityMask(hThread, affinityMask);
    CLOG(0 == affinityMask, HAL, Fatal, L"SetThreadAffinityMask({0:#16x}) failed with = {1}", affinityMask, FLastError());

    return result;
}
//----------------------------------------------------------------------------
void FWindowsPlatformThread::SetAffinityMask(FAffinityMask mask) {
    const ::HANDLE hThread = ::GetCurrentThread();

    const ::DWORD_PTR affinityMask = ::SetThreadAffinityMask(hThread, (::DWORD_PTR)mask);
    CLOG(0 == affinityMask, HAL, Fatal, L"SetThreadAffinityMask({0:#16x}) failed with = {1}", affinityMask, FLastError());

#ifdef WITH_CORE_ASSERT
    const FAffinityMask actualMask = FWindowsPlatformThread::AffinityMask();
    Assert((mask & ValidAffinityMask) == actualMask);
#endif
    UNUSED(affinityMask);
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
    case Core::EThreadPriority::Realtime:
        priorityWin32 = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    case Core::EThreadPriority::Highest:
        priorityWin32 = THREAD_PRIORITY_HIGHEST;
        break;
    case Core::EThreadPriority::AboveNormal:
        priorityWin32 = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case Core::EThreadPriority::Normal:
        priorityWin32 = THREAD_PRIORITY_NORMAL;
        break;
    case Core::EThreadPriority::BelowNormal:
        priorityWin32 = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case Core::EThreadPriority::Lowest:
        priorityWin32 = THREAD_PRIORITY_LOWEST;
        break;
    case Core::EThreadPriority::Idle:
        priorityWin32 = THREAD_PRIORITY_IDLE;
        break;
    }

    Verify(::SetThreadPriority(hThread, priorityWin32));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Task groups
//----------------------------------------------------------------------------
constexpr FWindowsPlatformThread::FAffinityMask AllButTwoFirsts =
    (FWindowsPlatformThread::AllThreadsAffinity & (~FWindowsPlatformThread::FAffinityMask(3)));
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::BackgroundThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.NumWorkers = 2;
    info.Affinities[0] = AllButTwoFirsts;
    info.Affinities[1] = AllButTwoFirsts;
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::GlobalThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.NumWorkers = Max(std::thread::hardware_concurrency() - 2ul, 1ul);
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = FAffinityMask(1) << (2ul + i);
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::HighPriorityThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.NumWorkers = Min(std::thread::hardware_concurrency(), size_t(CORE_MAX_NUMCORES));
    forrange(i, 0, FAffinityMask(info.NumWorkers))
        info.Affinities[i] = FAffinityMask(1) << i;
    return info;
}
//----------------------------------------------------------------------------
auto FWindowsPlatformThread::IOThreadsInfo() -> FThreadGroupInfo {
    FThreadGroupInfo info;
    info.NumWorkers = 2;
    info.Affinities[0] = AllButTwoFirsts;
    info.Affinities[1] = AllButTwoFirsts;
    return info;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
