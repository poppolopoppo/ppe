#pragma once

#include "Core.h"

#include "HAL/PlatformProcess.h"

#include <atomic>
#include <emmintrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// A lightweight spin-lock
// http://anki3d.org/spinlock/
// https://github.com/efficient/libcuckoo/blob/master/src/cuckoohash_map.hh
// Update:
// https://probablydance.com/2019/12/30/measuring-mutexes-spinlocks-and-how-bad-the-linux-scheduler-really-is/
//----------------------------------------------------------------------------
class FAtomicSpinLock : Meta::FNonCopyableNorMovable {
    std::atomic<bool> _locked{ false };

public:
#if USE_PPE_ASSERT
    FAtomicSpinLock() = default;
    ~FAtomicSpinLock() NOEXCEPT {
        Assert_Lightweight(!_locked);
    }
#endif

    void Lock() NOEXCEPT {
        for (size_t backoff = 0; !TryLock(); )
            FPlatformProcess::SleepForSpinning(backoff);
    }
    bool TryLock() NOEXCEPT {
        return (not _locked.load(std::memory_order_relaxed) and
                not _locked.exchange(true, std::memory_order_acquire) );
    }
    void Unlock() NOEXCEPT {
        _locked.store(false, std::memory_order_release);
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
        PPE_FAKEBOOL_OPERATOR_DECL() { return (Locked ? this : nullptr); }
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
// Less lightweight, but order preserving
// And more consistent :
// https://probablydance.com/2019/12/30/measuring-mutexes-spinlocks-and-how-bad-the-linux-scheduler-really-is/
//----------------------------------------------------------------------------
class FAtomicOrderedLock : Meta::FNonCopyableNorMovable {
    std::atomic<unsigned> In{ 0 };
    std::atomic<unsigned> Out{ 0 };

public:
    struct FScope : Meta::FNonCopyableNorMovable {
        FAtomicOrderedLock& Barrier;
#if USE_PPE_ASSERT
        const unsigned Ticket;
#endif

        FScope(FAtomicOrderedLock& barrier) NOEXCEPT
        :   Barrier(barrier)
#if USE_PPE_ASSERT
        ,   Ticket(barrier.In.fetch_add(1, std::memory_order_relaxed))
        {
#else
        {
            const unsigned Ticket = barrier.In.fetch_add(1, std::memory_order_relaxed);
#endif
            for (size_t backoff = 0; Barrier.Out.load(std::memory_order_acquire) != Ticket;)
                FPlatformProcess::SleepForSpinning(backoff);
        }

        ~FScope() NOEXCEPT {
            Assert_NoAssume(Barrier.Out == Ticket);
            // release the locks, the next waiting thread will stop spinning
            Barrier.Out.store(Barrier.Out.load(std::memory_order_relaxed) + 1,
                std::memory_order_release );
        }
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Read/Write lock with atomic spinning
// https://gist.github.com/yizhang82/500da684837161055978011c5850d296#file-rw_spin_lock-h
//----------------------------------------------------------------------------
class FAtomicReadWriteLock : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(unsigned, WRITER_LOCK, unsigned(-1));
    std::atomic<unsigned> Readers{ 0 };

#if USE_PPE_ASSERT
public:
    FAtomicReadWriteLock() = default;
    ~FAtomicReadWriteLock() NOEXCEPT {
        Assert_NoAssume(0 == Readers);
    }
#endif

public: // Reader
    struct FReaderScope {
        FAtomicReadWriteLock& Lock;
        FReaderScope(FAtomicReadWriteLock& lock)
        :   Lock(lock) {
            Lock.AcquireReader();
        }
        ~FReaderScope() {
            Lock.ReleaseReader();
        }
    };

    void AcquireReader() NOEXCEPT {
        for (size_t backoff = 0;;) {
            unsigned r = Readers;
            if (WRITER_LOCK != r) {
                if (Readers.compare_exchange_weak(r, r + 1))
                    return; // lock read
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    void ReleaseReader() NOEXCEPT {
        for (size_t backoff = 0;;) {
            unsigned r = Readers;
            Assert(r);
            if (WRITER_LOCK != r) {
                Assert_NoAssume(r > 0);
                if (Readers.compare_exchange_weak(r, r - 1))
                    return; // release read lock
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

public: // Writer
    struct FWriterScope {
        FAtomicReadWriteLock& Lock;
        FWriterScope(FAtomicReadWriteLock& lock)
        :   Lock(lock) {
            Lock.AcquireWriter();
        }
        ~FWriterScope() {
            Lock.ReleaseWriter();
        }
    };

    void AcquireWriter() NOEXCEPT {
        for (size_t backoff = 0;;) {
            unsigned r = Readers;
            if (0 == r) {
                if (Readers.compare_exchange_weak(r, WRITER_LOCK))
                    return; // lock write
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    void ReleaseWriter() NOEXCEPT {
        Assert_NoAssume(WRITER_LOCK == Readers);
        Readers = 0; // release write lock
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Atomic phase transition, hook on actual transition event
//----------------------------------------------------------------------------
class FAtomicPhaseLock : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(int, PhaseMask_, INT_MAX>>1);
    STATIC_CONST_INTEGRAL(int, TransitionBit_, INT_MAX & ~PhaseMask_);
    
    std::atomic<int> _phase{ 0 };
    
public:
    FAtomicPhaseLock() = default;
    
    explicit FAtomicPhaseLock(int phase) : _phase(phase) {}

#if USE_PPE_ASSERT
    ~FAtomicPhaseLock() {
        Assert_NoAssume(not (_phase & TransitionBit_));
    }
#endif
    
    template <typename _Functor>
    bool Transition(const int nextPhase, _Functor&& onTransition) {
        Assert_NoAssume(not (nextPhase & TransitionBit_));
        
        int expected = _phase.load(std::memory_order_relaxed);
        if (expected != nextPhase) {
            expected &= PhaseMask_; // clear potentially set transition bit
            
            if (Likely(_phase.compare_exchange_strong(expected, nextPhase | TransitionBit_))) {
                onTransition();
                _phase.store(nextPhase, std::memory_order_release);

                return true;
            }
            else for (size_t backoff = 0; expected != nextPhase;) {
                FPlatformProcess::SleepForSpinning(backoff);
                expected = _phase.load(std::memory_order_relaxed);
            }

            Assert_NoAssume(_phase == nextPhase);
        }

        return false;
    }

    static CONSTEXPR int Inc(int revision) {
       return (revision + 1) & PhaseMask_; 
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
