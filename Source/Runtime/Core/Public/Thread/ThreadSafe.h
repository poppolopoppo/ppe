#pragma once

#include "Core_fwd.h"

#include "Thread/ThreadSafe_fwd.h"

#include "Thread/AtomicSpinLock.h"
#include "Thread/CriticalSection.h"
#include "Thread/DataRaceCheck.h"
#include "Thread/ReadWriteLock.h"

#include "Meta/PointerWFlags.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use a CRTP helper to factorize the code bellow
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Impl>
class TThreadSafeCRTP_ : Meta::FNonCopyable {
public:
    using value_type = T;

    class FSharedLock : Meta::FNonCopyableNorMovable {
        Meta::TPointerWFlags<const TThreadSafeCRTP_> _data;
    public:
        explicit FSharedLock(const TThreadSafeCRTP_& data) NOEXCEPT {
            _data.Reset(std::addressof(data),
                reinterpret_cast<const _Impl&>(data).AcquireReader() );
        }
        ~FSharedLock() NOEXCEPT {
            if (_data.Flag0()/* need to call Release ? */)
                reinterpret_cast<const _Impl*>(_data.Get())->ReleaseReader();
        }
        const auto& Value() const { return _data->_value; }
        auto* Get() const { return std::addressof(operator*()); }
        auto& operator *() const NOEXCEPT { return Meta::DerefPtr(_data->_value); }
        auto* operator ->() const NOEXCEPT { return std::addressof(operator*()); }
    };

    class FExclusiveLock : Meta::FNonCopyableNorMovable {
        Meta::TPointerWFlags<TThreadSafeCRTP_> _data;
    public:
        explicit FExclusiveLock(TThreadSafeCRTP_& data) NOEXCEPT {
            _data.Reset(std::addressof(data),
                reinterpret_cast<_Impl&>(data).AcquireWriter() );
        }
        ~FExclusiveLock() NOEXCEPT {
            if (_data.Flag0()/* need to call Release ? */)
                reinterpret_cast<_Impl*>(_data.Get())->ReleaseWriter();
        }
        auto& Value() const { return _data->_value; }
        auto* Get() const { return std::addressof(operator*()); }
        auto& operator *() const NOEXCEPT { return Meta::DerefPtr(_data->_value); }
        auto* operator ->() const NOEXCEPT { return std::addressof(operator*()); }
    };

    TThreadSafeCRTP_() = default;

    explicit TThreadSafeCRTP_(const T& value) : _value(value) {}
    explicit TThreadSafeCRTP_(T&& rvalue) NOEXCEPT : _value(std::move(rvalue)) {}

    template <typename... _Args>
    explicit TThreadSafeCRTP_(_Args&&... args) : _value(std::forward<_Args>(args)...) {}

    TThreadSafeCRTP_(TThreadSafeCRTP_&& ) = default;
    TThreadSafeCRTP_& operator =(TThreadSafeCRTP_&& ) = delete;

    NODISCARD FSharedLock LockShared() const { return FSharedLock(*this); }
    NODISCARD FExclusiveLock LockExclusive() { return FExclusiveLock(*this); }

    NODISCARD T& Value_Unsafe() { return _value; }
    NODISCARD const T& Value_Unsafe() const { return _value; }

private:
    T _value;
};
} //!details
//----------------------------------------------------------------------------
// Dummy
//----------------------------------------------------------------------------
template <typename T, EThreadBarrier _Barrier> // only for code completion in templates
class TThreadSafe : public details::TThreadSafeCRTP_<T, TThreadSafe<T, _Barrier>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, _Barrier>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    NODISCARD bool AcquireReader() const NOEXCEPT = delete;
    void ReleaseReader() const NOEXCEPT = delete;

    NODISCARD bool AcquireWriter() NOEXCEPT = delete;
    void ReleaseWriter() NOEXCEPT = delete;
};
//----------------------------------------------------------------------------
// None
//----------------------------------------------------------------------------
template <typename T> // no locks => no thread-safety
class TThreadSafe<T, EThreadBarrier::None> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::None>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::None>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    NODISCARD bool AcquireReader() const NOEXCEPT { return true; }
    void ReleaseReader() const NOEXCEPT {}

    NODISCARD bool AcquireWriter() NOEXCEPT { return true; }
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

    NODISCARD bool AcquireReader() const NOEXCEPT { _barrierCS.Lock(); return true; }
    void ReleaseReader() const NOEXCEPT { _barrierCS.Unlock(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { _barrierCS.Lock(); return true; }
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

    NODISCARD bool AcquireReader() const NOEXCEPT { _barrierRW.LockRead(); return true; }
    void ReleaseReader() const NOEXCEPT { _barrierRW.UnlockRead(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { _barrierRW.LockWrite(); return true; }
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

    NODISCARD bool AcquireReader() const NOEXCEPT { _spinLock.Lock(); return true; }
    void ReleaseReader() const NOEXCEPT { _spinLock.Unlock(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { _spinLock.Lock(); return true; }
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

    NODISCARD bool AcquireReader() const NOEXCEPT { _spinLockRW.AcquireReader(); return true; }
    void ReleaseReader() const NOEXCEPT { _spinLockRW.ReleaseReader(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { _spinLockRW.AcquireWriter(); return true; }
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

    NODISCARD bool AcquireReader() const NOEXCEPT { _spinLockRW.AcquireReader(); return true; }
    void ReleaseReader() const NOEXCEPT { _spinLockRW.ReleaseReader(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { _spinLockRW.AcquireWriter(); return true; }
    void ReleaseWriter() NOEXCEPT { _spinLockRW.ReleaseWriter(); }

private:
    mutable FAtomicReadWriteLock _spinLockRW;
};
//----------------------------------------------------------------------------
// DataRaceCheck
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
template <typename T>
class TThreadSafe<T, EThreadBarrier::DataRaceCheck> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::DataRaceCheck>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::DataRaceCheck>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    NODISCARD bool AcquireReader() const NOEXCEPT { return _dataRaceCheck.Lock(); }
    void ReleaseReader() const NOEXCEPT { _dataRaceCheck.Unlock(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { return _dataRaceCheck.Lock(); }
    void ReleaseWriter() NOEXCEPT { _dataRaceCheck.Unlock(); }

private:
    mutable FDataRaceCheck _dataRaceCheck;
};
#endif
//----------------------------------------------------------------------------
// DataRaceCheckRW
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
template <typename T>
class TThreadSafe<T, EThreadBarrier::RWDataRaceCheck> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWDataRaceCheck>> {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::RWDataRaceCheck>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    NODISCARD bool AcquireReader() const NOEXCEPT { return _dataRaceCheckRW.LockShared(); }
    void ReleaseReader() const NOEXCEPT { _dataRaceCheckRW.UnlockShared(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { return _dataRaceCheckRW.LockExclusive(); }
    void ReleaseWriter() NOEXCEPT { _dataRaceCheckRW.UnlockExclusive(); }

private:
    mutable FRWDataRaceCheck _dataRaceCheckRW;
};
#endif
//----------------------------------------------------------------------------
// ThreadLocal
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG || USE_PPE_FASTDEBUG
template <typename T>
class TThreadSafe<T, EThreadBarrier::ThreadLocal> : public details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::ThreadLocal>>,
    Meta::FThreadResource {
public:
    using parent_type = details::TThreadSafeCRTP_<T, TThreadSafe<T, EThreadBarrier::ThreadLocal>>;

    using parent_type::parent_type;
    using parent_type::operator=;

    NODISCARD bool AcquireReader() const NOEXCEPT { THIS_THREADRESOURCE_CHECKACCESS(); return true; }
    void ReleaseReader() const NOEXCEPT { THIS_THREADRESOURCE_CHECKACCESS(); }

    NODISCARD bool AcquireWriter() NOEXCEPT { THIS_THREADRESOURCE_CHECKACCESS(); return true; }
    void ReleaseWriter() NOEXCEPT { THIS_THREADRESOURCE_CHECKACCESS(); }
};
#endif
//----------------------------------------------------------------------------
// Helper
//----------------------------------------------------------------------------
template <EThreadBarrier _Barrier, typename T>
NODISCARD TThreadSafe<T, _Barrier> MakeThreadSafe(T&& rvalue) NOEXCEPT {
    return TThreadSafe<T, _Barrier>( std::forward<T>(rvalue) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
