#pragma once

#include "HAL/Generic/GenericPlatformThread.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformThread : FGenericPlatformThread {
public:
    STATIC_CONST_INTEGRAL(bool, HasReadWriteLock, true);

    static void OnThreadStart();
    static void OnThreadShutdown();

    //------------------------------------------------------------------------
    // thread properties

    using FGenericPlatformThread::FAffinityMask;

    STATIC_CONST_INTEGRAL(FAffinityMask, ValidAffinityMask, 0xFF);

    using FGenericPlatformThread::AllThreadsAffinity;
    using FGenericPlatformThread::MainThreadAffinity;
    using FGenericPlatformThread::SecondaryThreadAffinity;

    static FAffinityMask AffinityMask();
    static void SetAffinityMask(FAffinityMask mask);

    static EThreadPriority Priority();
    static void SetPriority(EThreadPriority priority);

    //------------------------------------------------------------------------
    // task group

    using FGenericPlatformThread::FThreadGroupInfo;

    static FThreadGroupInfo BackgroundThreadsInfo();
    static FThreadGroupInfo GlobalThreadsInfo();
    static FThreadGroupInfo HighPriorityThreadsInfo();
    static FThreadGroupInfo IOThreadsInfo();

    //------------------------------------------------------------------------
    // fibers

    using FFiber = ::HANDLE;
    typedef void (STDCALL *FEntryPoint)(void *arg);
    STATIC_CONST_INTEGRAL(::DWORD, FiberFlags, FIBER_FLAG_FLOAT_SWITCH);

    static bool IsInFiber() { return ::IsThreadAFiber(); }

    static FFiber ConvertCurrentThreadToFiber() {
        Assert(not ::IsThreadAFiber());

        const ::HANDLE fiber = ::ConvertThreadToFiberEx(nullptr, FiberFlags);

        Assert(fiber && fiber != INVALID_HANDLE_VALUE);
        return fiber;
    }

    static void RevertCurrentFiberToThread(FFiber threadFiber) {
        Assert(threadFiber && threadFiber != INVALID_HANDLE_VALUE);
        Assert(::GetCurrentFiber() == threadFiber);
        UNUSED(threadFiber);

        ::ConvertFiberToThread();
    }

    static FFiber CurrentFiber() { return ::GetCurrentFiber(); }
    static void* FiberData() { return ::GetFiberData(); }

    static FFiber CreateFiber(
        size_t stackCommitSize,
        size_t stackReservedSize,
        FEntryPoint entryPoint,
        void* fiberData) {
        Assert(stackReservedSize >= 1024);
        Assert(entryPoint);

        return ::CreateFiberEx(stackCommitSize, stackReservedSize, FiberFlags, entryPoint, fiberData);
    }

    static void SwitchToFiber(FFiber fiber) {
        Assert(fiber);
        ::SwitchToFiber(fiber);
    }

    static void DestroyFiber(FFiber fiber) {
        Assert(fiber);
        ::DeleteFiber(fiber);
    }

    //------------------------------------------------------------------------
    // read-write lock

    using FReadWriteLock = ::SRWLOCK;

    static FORCE_INLINE void CreateReadWriteLock(FReadWriteLock* prwlock) {
        Assert(prwlock);
        ::InitializeSRWLock(prwlock);
    }

    static FORCE_INLINE void LockRead(FReadWriteLock& rwlock) {
        ::AcquireSRWLockShared(&rwlock);
    }

    static FORCE_INLINE bool TryLockRead(FReadWriteLock& rwlock) {
        return ::TryAcquireSRWLockShared(&rwlock);
    }

    static FORCE_INLINE void UnlockRead(FReadWriteLock& rwlock) {
        ::ReleaseSRWLockShared(&rwlock);
    }

    static FORCE_INLINE void LockWrite(FReadWriteLock& rwlock) {
        ::AcquireSRWLockExclusive(&rwlock);
    }

    static FORCE_INLINE bool TryLockWrite(FReadWriteLock& rwlock) {
        return ::TryAcquireSRWLockExclusive(&rwlock);
    }

    static FORCE_INLINE void UnlockWrite(FReadWriteLock& rwlock) {
        ::ReleaseSRWLockExclusive(&rwlock);
    }

    static FORCE_INLINE void DestroyReadWriteLock(FReadWriteLock* ) NOEXCEPT {
        //SRW locks do not need to be explicitly destroyed.
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS