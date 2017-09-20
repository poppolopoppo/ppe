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
struct FAtomicSpinLock {
    std::atomic_flag State = ATOMIC_FLAG_INIT;

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
} //!namespace Core
