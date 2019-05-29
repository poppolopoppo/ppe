#pragma once

#include "Core.h"

#include "HAL/PlatformProcess.h"

#include <atomic>
#include <emmintrin.h>

// http://anki3d.org/spinlock/
// https://github.com/efficient/libcuckoo/blob/master/src/cuckoohash_map.hh

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// A fast, lightweight spinlock
//----------------------------------------------------------------------------
class FAtomicSpinLock {
    std::atomic_flag State = ATOMIC_FLAG_INIT;

public:
    void Unlock() NOEXCEPT { State.clear(std::memory_order_release); }
    bool TryLock() NOEXCEPT { return (not State.test_and_set(std::memory_order_acquire)); }

    void Lock() NOEXCEPT {
        size_t backoff = 0;
        while (State.test_and_set(std::memory_order_acquire))
            FPlatformProcess::SleepForSpinning(backoff);
    }

    struct FScope : Meta::FNonCopyableNorMovable {
        FAtomicSpinLock& Barrier;
        FScope(FAtomicSpinLock& barrier) NOEXCEPT
        :   Barrier(barrier) {
            Barrier.Lock();
        }
        ~FScope() NOEXCEPT {
            Barrier.Unlock();
        }
    };

    struct FTryScope : Meta::FNonCopyableNorMovable {
        FAtomicSpinLock& Barrier;
        bool Locked;
        FTryScope(FAtomicSpinLock& barrier) NOEXCEPT
        :   Barrier(barrier)
        ,   Locked(barrier.TryLock()) {
        }
        void Unlock() {
            Assert_NoAssume(Locked);
            Barrier.Unlock();
            Locked = false;
        }
        ~FTryScope() NOEXCEPT {
            if (Locked)
                Barrier.Unlock();
        }
    };

    struct FUniqueLock : Meta::FNonCopyableNorMovable {
        FAtomicSpinLock& Barrier;
        bool NeedUnlock;
        FUniqueLock(FAtomicSpinLock& barrier) NOEXCEPT
        :   Barrier(barrier)
        ,   NeedUnlock(true) {
            Barrier.Lock();
        }
        ~FUniqueLock() NOEXCEPT {
            if (NeedUnlock)
                Barrier.Unlock();
        }
        void Lock() NOEXCEPT {
            Barrier.Lock();
            NeedUnlock = true;
        }
        void Unlock() NOEXCEPT {
            NeedUnlock = false;
            Barrier.Unlock();
        }
    };

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Less fast, but order preserving
//----------------------------------------------------------------------------
class FAtomicOrderedLock {
    std::atomic<size_t> Locked = 0;
    std::atomic<size_t> Revision = size_t(-1); // <-- must be == to (Locked - 1)

public:
    struct FScope : Meta::FNonCopyableNorMovable {
        FAtomicOrderedLock& Barrier;
#if USE_PPE_ASSERT
        const size_t Order;
#endif

        FScope(FAtomicOrderedLock& barrier) NOEXCEPT
        :   Barrier(barrier)
#if USE_PPE_ASSERT
        ,   Order(++Barrier.Revision)
        {
#else
        {
            const size_t Order = ++Barrier.Revision;
#endif
            //size_t backoff = 0;
            for (;;) { // spin for lock
                size_t revision = Order;
                if (Barrier.Locked.compare_exchange_weak(revision, revision, std::memory_order_acquire))
                    return;

                ::_mm_pause();//FPlatformProcess::SleepForSpinning(backoff); <- performs very poorly in this lock :/
            }
        }

        ~FScope() NOEXCEPT {
            Assert_NoAssume(Barrier.Locked == Order);

            ++Barrier.Locked; // release the locks, the next waiting thread will stop spinning
        }
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
