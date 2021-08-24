#pragma once

#include "Core_fwd.h"

#define USE_PPE_DATARACE_CHECKS (!USE_PPE_FINAL_RELEASE)

#if USE_PPE_DATARACE_CHECKS

// Those helpers are for debugging purposes, don't use them as a regular lock!

#include <atomic>
#include <mutex>
#include <thread>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Reentrant/recursive lock detecting unsafe concurrent accesses
//----------------------------------------------------------------------------
class FDataRaceCheck : Meta::FNonCopyableNorMovable {
public:
    FDataRaceCheck() = default;

    static size_t Token() NOEXCEPT {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    NODISCARD bool Lock() const {
        const size_t id = Token();

        size_t owner = _state.load(std::memory_order_acquire);
        if (id == owner)
            return true; // recursive-lock

        owner = 0;
        const bool locked = _state.compare_exchange_strong(owner, id, std::memory_order_relaxed);
        AssertReleaseMessage_NoAssume(L"Race condition detected!", 0 == owner); // locked by another thread

        return locked;
    }

    void Unlock() const {
        Assert_NoAssume(Token() == _state);
        _state.store(0, std::memory_order_relaxed);
    }

    struct FScopeLock : Meta::FNonCopyableNorMovable {
        const FDataRaceCheck& Resource;
        const bool WasLocked;
        explicit FScopeLock(const FDataRaceCheck& resource) NOEXCEPT
        :   Resource(resource)
        ,   WasLocked(Resource.Lock())
        {}
        ~FScopeLock() NOEXCEPT {
            if (not WasLocked)
                Resource.Unlock();
        }
    };

private:
    mutable std::atomic<size_t> _state{0};
};
//----------------------------------------------------------------------------
// Read/Write lock detecting unsafe concurrent accesses
//----------------------------------------------------------------------------
class FRWDataRaceCheck : Meta::FNonCopyableNorMovable {
public:
    FRWDataRaceCheck() = default;

    NODISCARD bool LockExclusive() {
        const bool locked = _lockWrite.try_lock();
        AssertReleaseMessage(L"Locked by another thread!", locked); // race condition detected

        int expected = _readCounter.load(std::memory_order_acquire);
        AssertReleaseMessage(L"Has read lock(s)!", expected <= 0); // race condition detected

        Verify(_readCounter.compare_exchange_strong(expected, expected - 1, std::memory_order_relaxed));
        AssertReleaseMessage(L"Has read lock(s)!", expected <= 0); // race condition detected

        return true;
    }

    void UnlockExclusive() {
        _readCounter.fetch_add(1, std::memory_order_relaxed);
        _lockWrite.unlock();
    }

    NODISCARD bool LockShared() const {
        bool locked;
        int expected = 0;
        do {
            locked = _readCounter.compare_exchange_weak(expected, expected + 1, std::memory_order_relaxed);

            // if has exclusive lock in current thread
            if (expected < 0 && _lockWrite.try_lock()) {
                _lockWrite.unlock();
                return false; // don't call UnlockShared()
            }

            AssertReleaseMessage(L"Has write lock(s)", expected >= 0);

        } while (not locked);

        return true;
    }

    void UnlockShared() const {
        // if has exclusive lock in current thread
        if (_readCounter.load(std::memory_order_acquire) < 0 && _lockWrite.try_lock()) {
            _lockWrite.unlock();
            return;
        }

        _readCounter.fetch_sub(1, std::memory_order_relaxed);
    }

    struct FScopeLockExclusive : Meta::FNonCopyableNorMovable {
        FRWDataRaceCheck& Resource;
        const bool WasLocked;
        explicit FScopeLockExclusive(FRWDataRaceCheck& resource) NOEXCEPT
        :   Resource(resource)
        ,   WasLocked(Resource.LockExclusive())
        {}
        ~FScopeLockExclusive() NOEXCEPT {
            if (not WasLocked)
                Resource.UnlockExclusive();
        }
    };

    struct FScopeLockShared : Meta::FNonCopyableNorMovable {
        const FRWDataRaceCheck& Resource;
        const bool WasLocked;
        explicit FScopeLockShared(const FRWDataRaceCheck& resource) NOEXCEPT
        :   Resource(resource)
        ,   WasLocked(Resource.LockShared())
        {}
        ~FScopeLockShared() NOEXCEPT {
            if (not WasLocked)
                Resource.UnlockShared();
        }
    };

private:
    mutable std::recursive_mutex _lockWrite;
    mutable std::atomic<int> _readCounter{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_DATARACE_CHECKS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDataRaceCheckResource {
public:
#if USE_PPE_DATARACE_CHECKS
    struct FScopeLock : FDataRaceCheck::FScopeLock {
        FScopeLock(const FDataRaceCheckResource& resource)
        :   FDataRaceCheck::FScopeLock(resource._dataRaceCheck)
        {}
    };
private:
    FDataRaceCheck _dataRaceCheck;
#endif
};
//----------------------------------------------------------------------------
class PPE_CORE_API FRWDataRaceCheckResource {
public:
#if USE_PPE_DATARACE_CHECKS
    struct FScopeLockExclusive : FRWDataRaceCheck::FScopeLockExclusive {
        FScopeLockExclusive(FRWDataRaceCheckResource& resource)
        :   FRWDataRaceCheck::FScopeLockExclusive(resource._rwDataRaceCheck)
        {}
    };
    struct FScopeLockShared : FRWDataRaceCheck::FScopeLockShared {
        FScopeLockShared(const FRWDataRaceCheckResource& resource)
        :   FRWDataRaceCheck::FScopeLockShared(resource._rwDataRaceCheck)
        {}
    };
private:
    FRWDataRaceCheck _rwDataRaceCheck;
#endif
};
//----------------------------------------------------------------------------
#if USE_PPE_DATARACE_CHECKS
#   define PPE_DATARACE_CHECK_SCOPE(_pResource) PPE::FDataRaceCheckResource::FScopeLock ANONYMIZE(dataRaceCheck)(*_pResource)
#   define PPE_DATARACE_EXCLUSIVE_SCOPE(_pResource) PPE::FRWDataRaceCheckResource::FScopeLockExclusive ANONYMIZE(dataRaceCheck)(*_pResource)
#   define PPE_DATARACE_SHARED_SCOPE(_pResource) PPE::FRWDataRaceCheckResource::FScopeLockShared ANONYMIZE(dataRaceCheck)(*_pResource)
#else
#   define PPE_DATARACE_CHECK_SCOPE(_pResource) NOOP()
#   define PPE_DATARACE_EXCLUSIVE_SCOPE(_pResource) NOOP()
#   define PPE_DATARACE_SHARED_SCOPE(_pResource) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
