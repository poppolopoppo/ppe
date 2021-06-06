#pragma once

#include "Core_fwd.h"

#include "Thread/AtomicSpinLock.h"
#include "Thread/CriticalSection.h"
#include "Thread/DataRaceCheck.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EThreadBarrier {
    None = 0,
    // blocking mutexes
    CriticalSection,
    RWLock,
    // atomic spin locking
    AtomicSpinLock,
    AtomicReadWriteLock,
    AtomicTicketRWLock,
#if !USE_PPE_FINAL_RELEASE
    // data race debugging
    DataRaceCheck,
    RWDataRaceCheck,
#endif
};
//----------------------------------------------------------------------------
CONSTEXPR bool EThreadBarrier_Safe(EThreadBarrier barrier) NOEXCEPT {
    switch (barrier) {
    case EThreadBarrier::None:
        return false;
    case EThreadBarrier::CriticalSection:
    case EThreadBarrier::RWLock:
    case EThreadBarrier::AtomicSpinLock:
    case EThreadBarrier::AtomicReadWriteLock:
    case EThreadBarrier::AtomicTicketRWLock:
#if !USE_PPE_FINAL_RELEASE
    case EThreadBarrier::DataRaceCheck:
    case EThreadBarrier::RWDataRaceCheck:
#endif
        return true;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
template <typename T, EThreadBarrier _Barrier>
class TThreadSafe;
//----------------------------------------------------------------------------
// Use a CRTP helper to factorize the code bellow
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Impl>
class TThreadSafeCRTP_ : Meta::FNonCopyable {
public:
    class FSharedLock : Meta::FNonCopyableNorMovable {
        const TThreadSafeCRTP_& _data;
    public:
        explicit FSharedLock(const TThreadSafeCRTP_& data) NOEXCEPT : _data(data) {
            reinterpret_cast<const _Impl*>(&_data)->AcquireReader();
        }
        ~FSharedLock() NOEXCEPT {
            reinterpret_cast<const _Impl*>(&_data)->ReleaseReader();
        }
        const T& Value() const { return _data._value; }
        const T& operator *() const NOEXCEPT { return _data._value; }
        const T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    class FExclusiveLock : Meta::FNonCopyableNorMovable {
        TThreadSafeCRTP_& _data;
    public:
        explicit FExclusiveLock(TThreadSafeCRTP_& data) NOEXCEPT : _data(data) {
            reinterpret_cast<_Impl*>(&_data)->AcquireWriter();
        }
        ~FExclusiveLock() NOEXCEPT {
            reinterpret_cast<_Impl*>(&_data)->ReleaseWriter();
        }
        T& Value() const { return _data._value; }
        T& operator *() const NOEXCEPT { return _data._value; }
        T* operator ->() const NOEXCEPT { return (&_data._value); }
    };

    TThreadSafeCRTP_() = default;

    explicit TThreadSafeCRTP_(const T& value) : _value(value) {}
    explicit TThreadSafeCRTP_(T&& rvalue) NOEXCEPT : _value(std::move(rvalue)) {}

    TThreadSafeCRTP_(TThreadSafeCRTP_&& ) = default;
    TThreadSafeCRTP_& operator =(TThreadSafeCRTP_&& ) = delete;

    FSharedLock LockShared() const { return FSharedLock(*this); }
    FExclusiveLock LockExclusive() { return FExclusiveLock(*this); }

    T& Value_NotThreadSafe() { return _value; }
    const T& Value_NotThreadSafe() const { return _value; }

private:
    T _value;
};
} //!details
//----------------------------------------------------------------------------
// None
//----------------------------------------------------------------------------
template <typename T> // no locks => no thread-safety
class TThreadSafe<T, EThreadBarrier::None> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::None>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::None>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT {}
    void ReleaseReader() const NOEXCEPT {}

    void AcquireWriter() NOEXCEPT {}
    void ReleaseWriter() NOEXCEPT {}
};
//----------------------------------------------------------------------------
// CriticalSection
//----------------------------------------------------------------------------
template <typename T>
class TThreadSafe<T, EThreadBarrier::CriticalSection> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::CriticalSection>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::CriticalSection>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { _barrierCS.Lock(); }
    void ReleaseReader() const NOEXCEPT { _barrierCS.Unlock(); }

    void AcquireWriter() NOEXCEPT { _barrierCS.Lock(); }
    void ReleaseWriter() NOEXCEPT { _barrierCS.Unlock(); }

private:
    mutable FCriticalSection _barrierCS;
};
//----------------------------------------------------------------------------
// RWLock
//----------------------------------------------------------------------------
template <typename T>
class TThreadSafe<T, EThreadBarrier::RWLock> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWLock>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWLock>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { _barrierRW.LockRead(); }
    void ReleaseReader() const NOEXCEPT { _barrierRW.UnlockRead(); }

    void AcquireWriter() NOEXCEPT { _barrierRW.LockWrite(); }
    void ReleaseWriter() NOEXCEPT { _barrierRW.UnlockWrite(); }

private:
    FReadWriteLock _barrierRW;
};
//----------------------------------------------------------------------------
// AtomicSpinLock
//----------------------------------------------------------------------------
template <typename T>
class TThreadSafe<T, EThreadBarrier::AtomicSpinLock> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicSpinLock>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicSpinLock>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { _spinLock.Lock(); }
    void ReleaseReader() const NOEXCEPT { _spinLock.Unlock(); }

    void AcquireWriter() NOEXCEPT { _spinLock.Lock(); }
    void ReleaseWriter() NOEXCEPT { _spinLock.Unlock(); }

private:
    mutable FAtomicSpinLock _spinLock;
};
//----------------------------------------------------------------------------
// AtomicReadWriteLock
//----------------------------------------------------------------------------
template <typename T>
class TThreadSafe<T, EThreadBarrier::AtomicReadWriteLock> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicReadWriteLock>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicReadWriteLock>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { _spinLockRW.AcquireReader(); }
    void ReleaseReader() const NOEXCEPT { _spinLockRW.ReleaseReader(); }

    void AcquireWriter() NOEXCEPT { _spinLockRW.AcquireWriter(); }
    void ReleaseWriter() NOEXCEPT { _spinLockRW.ReleaseWriter(); }

private:
    mutable FAtomicReadWriteLock _spinLockRW;
};
//----------------------------------------------------------------------------
// AtomicReadWriteLock
//----------------------------------------------------------------------------
template <typename T>
class TThreadSafe<T, EThreadBarrier::AtomicTicketRWLock> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicTicketRWLock>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::AtomicTicketRWLock>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { _spinLockRW.AcquireReader(); }
    void ReleaseReader() const NOEXCEPT { _spinLockRW.ReleaseReader(); }

    void AcquireWriter() NOEXCEPT { _spinLockRW.AcquireWriter(); }
    void ReleaseWriter() NOEXCEPT { _spinLockRW.ReleaseWriter(); }

private:
    mutable FAtomicReadWriteLock _spinLockRW;
};
//----------------------------------------------------------------------------
// DataRaceCheck
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename T>
class TThreadSafe<T, EThreadBarrier::DataRaceCheck> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::DataRaceCheck>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::DataRaceCheck>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { VerifyRelease(_dataRaceCheck.Lock()); }
    void ReleaseReader() const NOEXCEPT { _dataRaceCheck.Unlock(); }

    void AcquireWriter() NOEXCEPT { VerifyRelease(_dataRaceCheck.Lock()); }
    void ReleaseWriter() NOEXCEPT { _dataRaceCheck.Unlock(); }

private:
    mutable FDataRaceCheck _dataRaceCheck;
};
#endif
//----------------------------------------------------------------------------
// DataRaceCheckRW
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename T>
class TThreadSafe<T, EThreadBarrier::RWDataRaceCheck> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWDataRaceCheck>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWDataRaceCheck>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    void AcquireReader() const NOEXCEPT { VerifyRelease(_dataRaceCheckRW.LockShared()); }
    void ReleaseReader() const NOEXCEPT { _dataRaceCheckRW.UnlockShared(); }

    void AcquireWriter() NOEXCEPT { VerifyRelease(_dataRaceCheckRW.LockExclusive()); }
    void ReleaseWriter() NOEXCEPT { _dataRaceCheckRW.UnlockExclusive(); }

private:
    mutable FRWDataRaceCheck _dataRaceCheckRW;
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
