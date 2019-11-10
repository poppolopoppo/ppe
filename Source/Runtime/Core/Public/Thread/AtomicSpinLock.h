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
//----------------------------------------------------------------------------
class FAtomicSpinLock : Meta::FNonCopyableNorMovable {
    std::atomic_flag State = ATOMIC_FLAG_INIT;

#if USE_PPE_ASSERT
public:
    FAtomicSpinLock() = default;
    ~FAtomicSpinLock() NOEXCEPT {
        Assert_NoAssume(TryLock());
    }
#endif

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
//----------------------------------------------------------------------------
class FAtomicOrderedLock : Meta::FNonCopyableNorMovable {
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
// Read/Write lock with atomic spinning
// https://gist.github.com/yizhang82/500da684837161055978011c5850d296#file-rw_spin_lock-h
//----------------------------------------------------------------------------
class FAtomicReadWriteLock : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, WRITER_LOCK, INDEX_NONE);
    std::atomic<size_t> Readers{ 0 };

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
            size_t r = Readers;
            if (WRITER_LOCK != r) {
                if (Readers.compare_exchange_weak(r, r + 1))
                    return; // lock read
            }

            FPlatformProcess::SleepForSpinning(backoff);
        }
    }

    void ReleaseReader() NOEXCEPT {
        for (size_t backoff = 0;;) {
            size_t r = Readers;
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
            size_t r = Readers;
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
} //!namespace PPE
