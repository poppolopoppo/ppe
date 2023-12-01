#pragma once

#include "HAL/Generic/GenericPlatformThread.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformIncludes.h"

#include <bits/pthreadtypes.h>
#include <mutex>

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
    static bool IsInFiber() NOEXCEPT;

    static FFiber ConvertCurrentThreadToFiber();
    static void RevertCurrentFiberToThread(FFiber threadFiber);

    static FFiber CurrentFiber();
    static void* FiberData();

    static FFiber CreateFiber(
        size_t stackSize,
        FEntryPoint entryPoint,
        void* fiberData );

    static void SwitchToFiber(FFiber fiber);
    static void DestroyFiber(FFiber fiber);

    static void FiberStackRegion(FFiber fiber, const void** pStackBottom, size_t* pStackSize) NOEXCEPT;

    //------------------------------------------------------------------------
    // critical section

    using FCriticalSection = std::mutex;

    static void CreateCriticalSection(FCriticalSection* ) {
        // std::mutex doesn't require construction
    }

    static void Lock(FCriticalSection& cs) {
        cs.lock();
    }

    static bool TryLock(FCriticalSection& cs) {
        return cs.try_lock();
    }

    static void Unlock(FCriticalSection& cs) {
        cs.unlock();
    }

    static void DestroyCriticalSection(FCriticalSection* ) {
        // std::mutex doesn't require destrucion
    }

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

    //------------------------------------------------------------------------
    // synchronization barrier

    using FSynchronizationBarrier = ::pthread_barrier_t;

    static void CreateSynchronizationBarrier(FSynchronizationBarrier* pbarrier, size_t numThreads);
    static bool EnterSynchronizationBarrier(FSynchronizationBarrier& barrier);
    static void DestroySynchronizationBarrier(FSynchronizationBarrier* pbarrier);


};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
