#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EThreadPriority {
    Realtime        =  3,
    Highest         =  2,
    AboveNormal     =  1,
    Normal          =  0,
    BelowNormal     = -1,
    Lowest          = -2,
    Idle            = -3,
};
//----------------------------------------------------------------------------
struct FGenericPlatformThread {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasReadWriteLock, false);

    static void OnThreadStart() = delete;
    static void OnThreadShutdown() = delete;

    //------------------------------------------------------------------------
    // thread properties

    using FAffinityMask = u64;
    STATIC_ASSERT(PPE_MAX_NUMCPUCORE <= sizeof(FAffinityMask)<<3); // should change FAffinityMask type otherwise

    STATIC_CONST_INTEGRAL(FAffinityMask, AllCoresAffinity, FAffinityMask(-1));

    static FAffinityMask MainThreadAffinity() = delete;
    static FAffinityMask SecondaryThreadAffinity() = delete;

    static FAffinityMask AffinityMask() = delete; // get core affinities for current thread
    static void SetAffinityMask(FAffinityMask mask) = delete; // set core affinities for current thread

    static EThreadPriority Priority() = delete; // get current thread priority
    static void SetPriority(EThreadPriority priority) = delete; // set current thread priority

    //------------------------------------------------------------------------
    // Worker threads are locked to cores
    // * Avoid context switches and unwanted core switching
    // * Kernel threads can otherwise cause ripple effects across the cores
    // http://www.benicourt.com/blender/wp-content/uploads/2015/03/parallelizing_the_naughty_dog_engine_using_fibers.pdf

    struct FThreadGroupInfo {
        size_t NumWorkers = INDEX_NONE;
        EThreadPriority Priority = EThreadPriority::Normal;
        FAffinityMask Affinities[PPE_MAX_NUMCPUCORE] = { AllCoresAffinity };
    };

    static FThreadGroupInfo BackgroundThreadsInfo() = delete;
    static FThreadGroupInfo GlobalThreadsInfo() = delete;
    static FThreadGroupInfo HighPriorityThreadsInfo() = delete;
    static FThreadGroupInfo IOThreadsInfo() = delete;

    //------------------------------------------------------------------------
    // fibers

    using FFiber = void*;
    typedef void(*FEntryPoint)(void *arg);

    static bool IsInFiber() = delete;

    static FFiber ConvertCurrentThreadToFiber() = delete;
    static void RevertCurrentFiberToThread(FFiber threadFiber) = delete;

    static FFiber CreateFiber(
        size_t stackSize,
        FEntryPoint entryPoint,
        void* fiberData ) = delete;

    static FFiber CurrentFiber() = delete;
    static void* FiberData() = delete;

    static void SwitchToFiber(FFiber fiber) = delete;
    static void DestroyFiber(FFiber fiber) = delete;

    static void FiberStackRegion(FFiber fiber, const void** pStackBottom, size_t* pStackSize) = delete;

    //------------------------------------------------------------------------
    // critical section

    using FCriticalSection = void*;

    static void CreateCriticalSection(FCriticalSection* pcs) = delete;
    static void Lock(FCriticalSection& cs) = delete;
    static bool TryLock(FCriticalSection& cs) = delete;
    static void Unlock(FCriticalSection& cs) = delete;
    static void DestroyCriticalSection(FCriticalSection* pcs) = delete;

    //------------------------------------------------------------------------
    // read-write lock

    using FReadWriteLock = void*;

    static void CreateReadWriteLock(FReadWriteLock* prwlock) = delete;
    static void LockRead(FReadWriteLock& rwlock) = delete;
    static bool TryLockRead(FReadWriteLock& rwlock) = delete;
    static void UnlockRead(FReadWriteLock& rwlock) = delete;
    static void LockWrite(FReadWriteLock& rwlock) = delete;
    static bool TryLockWrite(FReadWriteLock& rwlock) = delete;
    static void UnlockWrite(FReadWriteLock& rwlock) = delete;
    static void DestroyReadWriteLock(FReadWriteLock* prwlock) = delete;

    //------------------------------------------------------------------------
    // synchronization barrier

    using FSynchronizationBarrier = void*;

    static void CreateSynchronizationBarrier(FSynchronizationBarrier* pbarrier, size_t numThreads) = delete;
    static bool EnterSynchronizationBarrier(FSynchronizationBarrier& barrier) = delete;
    static void DestroySynchronizationBarrier(FSynchronizationBarrier* pbarrier) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
