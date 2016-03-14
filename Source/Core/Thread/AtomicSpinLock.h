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
CACHELINE_ALIGNED class AtomicSpinLock {
public:
    AtomicSpinLock() { _state.clear(); }

    AtomicSpinLock(const AtomicSpinLock& ) = delete;
    AtomicSpinLock& operator =(const AtomicSpinLock& ) = delete;

    void Unlock() { _state.clear(std::memory_order_release); }
    bool TryLock() { return false == _state.test_and_set(std::memory_order_acquire); }

    void Lock() {
        while(_state.test_and_set(std::memory_order_acquire))
            _mm_pause();
    }

    struct Scope {
        Scope(AtomicSpinLock& lock) : _lock(lock) { _lock.Lock(); }
        ~Scope() { _lock.Unlock(); }

        Scope(const Scope& ) = delete;
        Scope& operator =(const Scope& ) = delete;

    private:
        AtomicSpinLock& _lock;
    };

    struct TryScope {
        TryScope(AtomicSpinLock& lock) : _lock(lock), _locked(lock.TryLock()) {}
        ~TryScope() { if (_locked) _lock.Unlock(); }

        TryScope(const TryScope& ) = delete;
        TryScope& operator =(const TryScope& ) = delete;

        bool Locked() const { return _locked; }

    private:
        AtomicSpinLock& _lock;
        bool _locked;
    };

private:
    std::atomic_flag _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
