#pragma once

#include "Core_fwd.h"

#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _Safe = (USE_PPE_DEBUG || USE_PPE_MEMORY_DEBUGGING)>
class TThreadSafe : Meta::FNonCopyable {
public:
    class FSharedLock : Meta::FNonCopyableNorMovable {
        const TThreadSafe& _data;
    public:
        explicit FSharedLock(const TThreadSafe& data) NOEXCEPT : _data(data) {
            _data._barrierRW.LockRead();
        }
        ~FSharedLock() NOEXCEPT {
            _data._barrierRW.UnlockRead();
        }
        const T& Value() const { return _data._value; }
        const T& operator *() const NOEXCEPT { return _data._value; }
        const T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    class FExclusiveLock : Meta::FNonCopyableNorMovable {
        TThreadSafe& _data;
    public:
        explicit FExclusiveLock(TThreadSafe& data) NOEXCEPT : _data(data) {
            _data._barrierRW.LockWrite();
        }
        ~FExclusiveLock() NOEXCEPT {
            _data._barrierRW.UnlockWrite();
        }
        T& Value() const { return _data._value; }
        T& operator *() const NOEXCEPT { return _data._value; }
        T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    TThreadSafe() = default;
#if USE_PPE_ASSERT
    ~TThreadSafe() {
        Verify(_barrierRW.TryLockWrite()); // has write lock(s) -> race condition !
    }
#endif

    explicit TThreadSafe(const T& value) : _value(value) {}
    explicit TThreadSafe(T&& rvalue) NOEXCEPT : _value(std::move(rvalue)) {}

    TThreadSafe(TThreadSafe&& rvalue) NOEXCEPT {
        _value = std::move(rvalue.LockShared().Value());
    }
    TThreadSafe& operator =(TThreadSafe&& ) = delete;

    FSharedLock LockShared() const { return FSharedLock(*this); }
    FExclusiveLock LockExclusive() { return FExclusiveLock(*this); }

    T& Value_NotThreadSafe() { return _value; }
    const T& Value_NotThreadSafe() const { return _value; }

private:
    mutable FReadWriteLock _barrierRW;

    T _value;
};
//----------------------------------------------------------------------------
template <typename T> // no locks => no thread-safety
class TThreadSafe<T, false> : Meta::FNonCopyable {
public:
    class FSharedLock : Meta::FNonCopyableNorMovable {
        const TThreadSafe& _data;
    public:
        explicit FSharedLock(const TThreadSafe& data) NOEXCEPT : _data(data) {}
        const T& Value() const { return _data._value; }
        const T& operator *() const NOEXCEPT { return _data._value; }
        const T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    class FExclusiveLock : Meta::FNonCopyableNorMovable {
        TThreadSafe& _data;
    public:
        explicit FExclusiveLock(TThreadSafe& data) NOEXCEPT : _data(data) {}
        T& Value() const { return _data._value; }
        T& operator *() const NOEXCEPT { return _data._value; }
        T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    TThreadSafe() = default;

    explicit TThreadSafe(const T& value) : _value(value) {}
    explicit TThreadSafe(T&& rvalue) NOEXCEPT : _value(std::move(rvalue)) {}

    TThreadSafe(TThreadSafe&& ) = default;
    TThreadSafe& operator =(TThreadSafe&& ) = delete;

    FSharedLock LockShared() const { return FSharedLock(*this); }
    FExclusiveLock LockExclusive() { return FExclusiveLock(*this); }

    T& Value_NotThreadSafe() { return _value; }
    const T& Value_NotThreadSafe() const { return _value; }

private:
    T _value;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
