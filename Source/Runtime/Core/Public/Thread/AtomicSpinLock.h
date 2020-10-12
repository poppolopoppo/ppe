#pragma once

#include "Core.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformProcess.h"

#include <atomic>
#include <emmintrin.h>

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

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
        for (i32 backoff = 0; !TryLock(); )
            FPlatformProcess::SleepForSpinning(backoff);
    }
    bool TryLock() NOEXCEPT {
        return (not _locked.load(std::memory_order_relaxed) &&
                not _locked.exchange(true, std::memory_order_acquire) );
    }
    void Unlock() NOEXCEPT {
        _locked.store(false, std::memory_order_release);
    }

    struct FScope : Meta::FNonCopyableNorMovable {
        FAtomicSpinLock& Barrier;
        explicit FScope(FAtomicSpinLock& barrier) NOEXCEPT
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
        explicit FTryScope(FAtomicSpinLock& barrier) NOEXCEPT
        :   Barrier(barrier)
        ,   Locked(barrier.TryLock()) {
        }
        PPE_FAKEBOOL_OPERATOR_DECL() { return (Locked ? this : nullptr); }
        void Unlock() NOEXCEPT {
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
        explicit FUniqueLock(FAtomicSpinLock& barrier) NOEXCEPT
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
class CACHELINE_ALIGNED FAtomicOrderedLock : Meta::FNonCopyableNorMovable {
    std::atomic<unsigned> In{ 0 };
    std::atomic<unsigned> Out{ 0 };

public:
    struct FScope : Meta::FNonCopyableNorMovable {
        FAtomicOrderedLock& Barrier;
#if USE_PPE_ASSERT
        const unsigned Ticket;
#endif

        explicit FScope(FAtomicOrderedLock& barrier) NOEXCEPT
        :   Barrier(barrier)
#if USE_PPE_ASSERT
        ,   Ticket(barrier.In.fetch_add(1, std::memory_order_relaxed))
        {
#else
        {
            const unsigned Ticket = barrier.In.fetch_add(1, std::memory_order_relaxed);
#endif
            for (i32 backoff = 0; Barrier.Out.load(std::memory_order_acquire) != Ticket;)
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
    struct FReaderScope : Meta::FNonCopyableNorMovable {
        FAtomicReadWriteLock& Lock;
        explicit FReaderScope(FAtomicReadWriteLock& lock) NOEXCEPT
        :   Lock(lock) {
            Lock.AcquireReader();
        }
        ~FReaderScope() NOEXCEPT {
            Lock.ReleaseReader();
        }
    };

    void AcquireReader() NOEXCEPT {
        for (i32 backoff = 0;;) {
            unsigned r = Readers.load(std::memory_order_relaxed);
            if (WRITER_LOCK != r) {
                if (Readers.compare_exchange_weak(r, r + 1))
                    return; // lock read
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    void ReleaseReader() NOEXCEPT {
        for (i32 backoff = 0;;) {
            unsigned r = Readers.load(std::memory_order_relaxed);
            Assert(r > 0); // or we couldn't have locked reader
            if (Readers.compare_exchange_weak(r, r - 1,
                std::memory_order_release, std::memory_order_relaxed))
                return; // release read lock

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

public: // Writer
    struct FWriterScope : Meta::FNonCopyableNorMovable {
        FAtomicReadWriteLock& Lock;
        explicit FWriterScope(FAtomicReadWriteLock& lock) NOEXCEPT
        :   Lock(lock) {
            Lock.AcquireWriter();
        }
        ~FWriterScope() NOEXCEPT {
            Lock.ReleaseWriter();
        }
    };

    void AcquireWriter() NOEXCEPT {
        for (i32 backoff = 0;;) {
            unsigned r = Readers.load(std::memory_order_relaxed);
            if (0 == r) {
                if (Readers.compare_exchange_weak(r, WRITER_LOCK,
                    std::memory_order_release, std::memory_order_relaxed))
                    return; // lock write
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    bool TryAcquireWriter() NOEXCEPT {
        unsigned r = Readers.load(std::memory_order_relaxed);
        return ((0 == r) && Readers.compare_exchange_weak(r, WRITER_LOCK,
            std::memory_order_release, std::memory_order_relaxed));
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
public:
    using size_type = size_t;

    FAtomicPhaseLock() = default;

    explicit FAtomicPhaseLock(size_type phase) NOEXCEPT : _phase(phase) {}

#if USE_PPE_ASSERT
    ~FAtomicPhaseLock() NOEXCEPT {
        Assert_NoAssume(not (_phase & TransitionBit_));
    }
#endif

    template <typename _Functor>
    bool Transition(const size_type nextPhase, _Functor&& onTransition) NOEXCEPT {
        Assert_NoAssume(not (nextPhase & TransitionBit_));

        size_type expected = _phase.load(std::memory_order_relaxed);
        if (expected != nextPhase) {
            expected &= PhaseMask_; // clear potentially set transition bit

            if (Likely(_phase.compare_exchange_strong(expected, nextPhase | TransitionBit_))) {
                onTransition();
                _phase.store(nextPhase, std::memory_order_release);

                return true;
            }
            else for (i32 backoff = 0; expected != nextPhase;) {
                FPlatformProcess::SleepForSpinning(backoff);
                expected = _phase.load(std::memory_order_relaxed);
            }

            Assert_NoAssume(_phase == nextPhase);
        }

        return false;
    }

    static CONSTEXPR size_type Inc(size_type revision) NOEXCEPT {
       return ((revision + 1) & PhaseMask_);
    }

private:
    STATIC_CONST_INTEGRAL(size_type, PhaseMask_, SIZE_MAX >> 1);
    STATIC_CONST_INTEGRAL(size_type, TransitionBit_, SIZE_MAX & ~PhaseMask_);

    std::atomic<size_type> _phase{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Can lock a subset instead of all resource
//----------------------------------------------------------------------------
class CACHELINE_ALIGNED FAtomicMaskLock : Meta::FNonCopyableNorMovable {
public:
    using size_type = size_t;
    STATIC_CONST_INTEGRAL(size_type, AllMask, size_type(-1));
    STATIC_CONST_INTEGRAL(size_type, NumBuckets, sizeof(size_type) << 3);

    struct FScopeLock : Meta::FNonCopyableNorMovable {
        FAtomicMaskLock& Owner;
        const size_type Subset;
        FScopeLock(FAtomicMaskLock& owner, size_type subset) NOEXCEPT
        :   Owner(owner), Subset(subset) {
            Owner.Lock(Subset);
        }
        ~FScopeLock() NOEXCEPT {
            Owner.Unlock(Subset);
        }
    };

    void Lock(size_type subset) NOEXCEPT {
        Assert(subset);
        for (i32 backoff = 0;;) {
            size_type msk = _mask.load(std::memory_order_relaxed);
            if ((msk & subset) == 0 && _mask.compare_exchange_weak(msk, msk | subset,
                    std::memory_order_release, std::memory_order_relaxed) ) {
                return;
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    bool TryLock(size_type subset) NOEXCEPT {
        Assert(subset);
        size_type msk = _mask.load(std::memory_order_relaxed);
        if ((msk & subset) == 0 && _mask.compare_exchange_weak(msk, msk | subset,
            std::memory_order_release, std::memory_order_relaxed)) {
            return true;
        }
        return false;
    }

    void Unlock(size_type subset) NOEXCEPT {
        Assert(subset);
        size_type msk = _mask.load(std::memory_order_relaxed);
        Assert((msk & subset) == subset);
        _mask = (msk & ~subset);
    }

private:
    std::atomic<size_type> _mask{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Order preserving + read/write
// "Scalable Reader-Writer Synchronization for Shared-Memory Multiprocessors"
// https://www.cs.utexas.edu/~pingali/CS378/2015sp/lectures/Spinlocks%20and%20Read-Write%20Locks.htm
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FAtomicTicketRWLock : Meta::FNonCopyableNorMovable {
    union UQueue {
        u32 u;
        u16 us;
        struct
        {
            u8 Write;
            u8 Read;
            u8 Users;
        } s;
    };

    volatile UQueue Queue{ 0 };

public: // Read
    struct FReaderScope : Meta::FNonCopyableNorMovable {
        FAtomicTicketRWLock& RWLock;
        explicit FReaderScope(FAtomicTicketRWLock& rwlock) NOEXCEPT
        :   RWLock(rwlock) {
            RWLock.LockRead();
        }
        ~FReaderScope() NOEXCEPT {
            RWLock.UnlockRead();
        }
    };

    void LockRead() NOEXCEPT {
        const u32 me = FPlatformAtomics::Add(reinterpret_cast<volatile i32*>(&Queue.u), (1<<16));
        const u8 val = static_cast<u8>(me >> 16);

        for (i32 backoff = 0; Queue.s.Read != val;)
            FPlatformProcess::SleepForSpinning(backoff);

        Queue.s.Read++;
    }

    void UnlockRead() NOEXCEPT {
        FPlatformAtomics::Increment(reinterpret_cast<volatile i8*>(&Queue.s.Write));
    }

    bool TryLockRead() NOEXCEPT {
        const u32 me = Queue.s.Users;
        const u8 menew = static_cast<u8>(me + 1);
        const u32 write = Queue.s.Write;
        const u32 cmp =  (me << 16) + (me << 8) + write;
        const u32 cmpnew = (u32(menew) << 16) + (menew << 8) + write;

        return (FPlatformAtomics::CompareExchange(
            reinterpret_cast<volatile i32*>(&Queue.u),
            static_cast<i32>(cmp),
            static_cast<i32>(cmpnew)) == static_cast<i32>(cmp));
    }

public: // Write
    struct FWriterScope : Meta::FNonCopyableNorMovable {
        FAtomicTicketRWLock& RWLock;
        explicit FWriterScope(FAtomicTicketRWLock& rwlock) NOEXCEPT
        :   RWLock(rwlock) {
            RWLock.LockWrite();
        }
        ~FWriterScope() NOEXCEPT {
            RWLock.UnlockWrite();
        }
    };

    void LockWrite() NOEXCEPT {
        const u32 me = FPlatformAtomics::Add(reinterpret_cast<volatile i32*>(&Queue.u), (1<<16));
        const u8 val = static_cast<u8>(me >> 16);

        for (i32 backoff = 0; Queue.s.Write != val;)
            FPlatformProcess::SleepForSpinning(backoff);
    }

    void UnlockWrite() NOEXCEPT {
        UQueue t{ Queue.u };
        FPlatformAtomics::MemoryBarrier();

        t.s.Read++;
        t.s.Write++;

        *reinterpret_cast<volatile u16*>(&Queue) = t.us;
    }

    bool TryLockWrite() NOEXCEPT {
        const u32 me = Queue.s.Users;
        const u8 menew = static_cast<u8>(me + 1);

        const u32 read = Queue.s.Read << 8;
        const u32 cmp =  (me << 16) + read + me;
        const u32 cmpnew = (u32(menew) << 16) + read + me;

        return (FPlatformAtomics::CompareExchange(
            reinterpret_cast<volatile i32*>(&Queue.u),
            static_cast<i32>(cmp),
            static_cast<i32>(cmpnew)) == static_cast<i32>(cmp));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
