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
#pragma warning( push )
#pragma warning( disable : 4324 ) // la structure a été remplie en raison du spécificateur d'alignement.
CACHELINE_ALIGNED class FAtomicSpinLock {
public:
    FAtomicSpinLock() { _state.clear(); }

    FAtomicSpinLock(const FAtomicSpinLock& ) = delete;
    FAtomicSpinLock& operator =(const FAtomicSpinLock& ) = delete;

    void Unlock() { _state.clear(std::memory_order_release); }
    bool TryLock() { return false == _state.test_and_set(std::memory_order_acquire); }

    void Lock() {
        while(_state.test_and_set(std::memory_order_acquire))
            _mm_pause();
    }

    struct FScope {
        FScope(FAtomicSpinLock& lock) : _lock(lock) { _lock.Lock(); }
        ~FScope() { _lock.Unlock(); }

        FScope(const FScope& ) = delete;
        FScope& operator =(const FScope& ) = delete;

    private:
        FAtomicSpinLock& _lock;
    };

    struct FTryScope {
        FTryScope(FAtomicSpinLock& lock) : _lock(lock), _locked(lock.TryLock()) {}
        ~FTryScope() { if (_locked) _lock.Unlock(); }

        FTryScope(const FTryScope& ) = delete;
        FTryScope& operator =(const FTryScope& ) = delete;

        bool Locked() const { return _locked; }

    private:
        FAtomicSpinLock& _lock;
        bool _locked;
    };

private:
    std::atomic_flag _state;
};
#pragma warning( pop )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
