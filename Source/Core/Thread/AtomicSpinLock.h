#pragma once

#include "Core/Core.h"

#include <atomic>
#include <emmintrin.h>

// http://anki3d.org/spinlock/
// https://github.com/efficient/libcuckoo/blob/master/src/cuckoohash_map.hh

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// A fast, lightweight spinlock
//----------------------------------------------------------------------------
class FAtomicSpinLock {
    std::atomic_flag State = ATOMIC_FLAG_INIT;

public:
    void Unlock() { State.clear(std::memory_order_release); }
    bool TryLock() { return false == State.test_and_set(std::memory_order_acquire); }

    void Lock() {
        while(State.test_and_set(std::memory_order_acquire))
            _mm_pause();
    }

    struct FScope {
        FAtomicSpinLock& Barrier;
        FScope(FAtomicSpinLock& barrier)
            : Barrier(barrier) {
            Barrier.Lock();
        }
        ~FScope() {
            Barrier.Unlock();
        }
    };

    struct FTryScope {
        FAtomicSpinLock& Barrier;
        const bool Locked;
        FTryScope(FAtomicSpinLock& barrier)
            : Barrier(barrier)
            , Locked(barrier.TryLock()) {
        }
        ~FTryScope() {
            if (Locked)
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
    struct FScope {
        FAtomicOrderedLock& Barrier;
#ifdef WITH_CORE_ASSERT
        const size_t Order;
#endif

        FScope(FAtomicOrderedLock& barrier)
            : Barrier(barrier)
#ifdef WITH_CORE_ASSERT
            , Order(++Barrier.Revision)
        {
#else
        {
            const size_t Order = ++Barrier.Revision;
#endif
            for (;;) { // spin for lock
                size_t revision = Order;
                if (Barrier.Locked.compare_exchange_weak(revision, revision, std::memory_order_acquire))
                    return;

                _mm_pause();
            }
        }

        ~FScope() {
            Assert_NoAssume(Barrier.Locked == Order);

            ++Barrier.Locked; // release the locks, the next waiting thread will stop spinning
        }
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
