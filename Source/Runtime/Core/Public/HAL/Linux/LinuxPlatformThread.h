#pragma once

#include "HAL/Generic/GenericPlatformThread.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformIncludes.h"

#if PPE_HAS_CXX14
#   include <shared_mutex>
#else
#   error "need std::shared_mutex to replace SRWLOCK"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLinuxFiber;
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformThread : FGenericPlatformThread {
public:
    STATIC_CONST_INTEGRAL(bool, HasReadWriteLock, true);

    static void OnThreadStart();
    static void OnThreadShutdown();

    //------------------------------------------------------------------------
    // thread properties

    using FGenericPlatformThread::FAffinityMask;

    static const FAffinityMask AllCoresAffinity;

    static FAffinityMask MainThreadAffinity();
    static FAffinityMask SecondaryThreadAffinity();

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

    using FFiber = FLinuxFiber*;
    typedef void (*FEntryPoint)(void *arg);

    // no difference between thread and fiber on linux
    static bool IsInFiber() { return true; }

    static FFiber ConvertCurrentThreadToFiber();
    static void RevertCurrentFiberToThread(FFiber threadFiber);

    static FFiber CurrentFiber();
    static void* FiberData();

    static FFiber CreateFiber(
        size_t stackCommitSize,
        size_t stackReservedSize,
        FEntryPoint entryPoint,
        void* fiberData );

    static void SwitchToFiber(FFiber fiber);
    static void DestroyFiber(FFiber fiber);

    //------------------------------------------------------------------------
    // read-write lock

    using FReadWriteLock = std::shared_timed_mutex;

    static FORCE_INLINE void CreateReadWriteLock(FReadWriteLock* ) {
        // std::shared_timed_mutex doesn't require construction
    }

    static FORCE_INLINE void LockRead(FReadWriteLock& rwlock) {
        rwlock.lock();
    }

    static FORCE_INLINE bool TryLockRead(FReadWriteLock& rwlock) {
        return rwlock.try_lock_shared();
    }

    static FORCE_INLINE void UnlockRead(FReadWriteLock& rwlock) {
        rwlock.unlock_shared();
    }

    static FORCE_INLINE void LockWrite(FReadWriteLock& rwlock) {
        rwlock.lock();
    }

    static FORCE_INLINE bool TryLockWrite(FReadWriteLock& rwlock) {
        return rwlock.try_lock();
    }

    static FORCE_INLINE void UnlockWrite(FReadWriteLock& rwlock) {
        rwlock.unlock();
    }

    static FORCE_INLINE void DestroyReadWriteLock(FReadWriteLock* ) NOEXCEPT {
        // std::shared_timed_mutex doesn't require destrucion
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
