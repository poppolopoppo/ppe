#pragma once

#include "HAL/Generic/GenericPlatformThread.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FWindowsPlatformThread : FGenericPlatformThread {
public:
    STATIC_CONST_INTEGRAL(bool, HasReadWriteLock, true);

    static PPE_CORE_API void OnThreadStart();
    static PPE_CORE_API void OnThreadShutdown();

    //------------------------------------------------------------------------
    // thread properties

    using FGenericPlatformThread::FAffinityMask;

    static const FAffinityMask AllCoresAffinity;

    static PPE_CORE_API FAffinityMask MainThreadAffinity();
    static PPE_CORE_API FAffinityMask SecondaryThreadAffinity();

    static PPE_CORE_API FAffinityMask AffinityMask();
    static PPE_CORE_API void SetAffinityMask(FAffinityMask mask);

    static PPE_CORE_API EThreadPriority Priority();
    static PPE_CORE_API void SetPriority(EThreadPriority priority);

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
        Assert_NoAssume(not ::IsThreadAFiber());

        const ::HANDLE fiber = ::ConvertThreadToFiberEx(nullptr, FiberFlags);

        Assert(fiber && fiber != INVALID_HANDLE_VALUE);
        return fiber;
    }

    static void RevertCurrentFiberToThread(FFiber threadFiber) {
        Assert(threadFiber && threadFiber != INVALID_HANDLE_VALUE);
        Assert_NoAssume(::GetCurrentFiber() == threadFiber);
        Unused(threadFiber);

        ::ConvertFiberToThread();
    }

    static FFiber CurrentFiber() { return ::GetCurrentFiber(); }
    static void* FiberData() { return ::GetFiberData(); }

    static FFiber CreateFiber(size_t stackSize, FEntryPoint entryPoint, void* fiberData) {
        Assert(stackSize >= 1024);
        Assert(entryPoint);

        return ::CreateFiberEx(stackSize, stackSize, FiberFlags, entryPoint, fiberData);
    }

    static void SwitchToFiber(FFiber fiber) {
        Assert(fiber);

        ::SwitchToFiber(fiber);
    }

    static void DestroyFiber(FFiber fiber) {
        Assert(fiber);

        ::DeleteFiber(fiber);
    }

    static void FiberStackRegion(FFiber fiber, const void** pStackBottom, size_t* pStackSize) NOEXCEPT;

    //------------------------------------------------------------------------
    // critical section

    using FCriticalSection = ::CRITICAL_SECTION;

    static FORCE_INLINE void CreateCriticalSection(FCriticalSection* pcs) {
        u32 flags = RTL_CRITICAL_SECTION_FLAG_DYNAMIC_SPIN;
#if !USE_PPE_DEBUG
        flags |= RTL_CRITICAL_SECTION_FLAG_NO_DEBUG_INFO;
#endif
        ::InitializeCriticalSectionEx(pcs, 4000/* MS undocumented default */, flags);
    }

    static FORCE_INLINE void Lock(FCriticalSection& cs) {
        ::EnterCriticalSection(&cs);
    }

    static FORCE_INLINE bool TryLock(FCriticalSection& cs) {
        return ::TryEnterCriticalSection(&cs);
    }

    static FORCE_INLINE void Unlock(FCriticalSection& cs) {
        ::LeaveCriticalSection(&cs);
    }

    static FORCE_INLINE void DestroyCriticalSection(FCriticalSection* pcs) {
        Assert(pcs);
    ::  DeleteCriticalSection(pcs);
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

    //------------------------------------------------------------------------
    // synchronization barrier

    using FSynchronizationBarrier = ::SYNCHRONIZATION_BARRIER;

    static FORCE_INLINE void CreateSynchronizationBarrier(FSynchronizationBarrier* pbarrier, size_t numThreads) {
        Assert(pbarrier);
        ::InitializeSynchronizationBarrier(pbarrier, static_cast<int>(numThreads), -1);
    }

    static FORCE_INLINE bool EnterSynchronizationBarrier(FSynchronizationBarrier& barrier) {
        // return true for only *ONE* thread
        // https://docs.microsoft.com/fr-fr/windows/win32/sync/synchronization-barriers
        return ::EnterSynchronizationBarrier(&barrier, SYNCHRONIZATION_BARRIER_FLAGS_NO_DELETE);
    }

    static FORCE_INLINE void DestroySynchronizationBarrier(FSynchronizationBarrier* pbarrier) NOEXCEPT {
        Assert(pbarrier);
        Verify(::DeleteSynchronizationBarrier(pbarrier));
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
