#pragma once

#include "Core.h"

#include "Container/SparseArray.h"
#include "Misc/EventHandle.h"
#include "Misc/Function.h"
#include "Thread/ThreadSafe.h"

#define PUBLIC_EVENT(_NAME, ...) \
    protected: \
        mutable ::PPE::TEvent< __VA_ARGS__ , false > CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent< __VA_ARGS__ , false >& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

#define THREADSAFE_EVENT(_NAME, ...) \
    protected: \
        mutable ::PPE::TEvent< __VA_ARGS__ , true > CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent< __VA_ARGS__ , true >& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// - TEvent<> is equivalent to C# events
// - FEventHandle handles registration lifetime
// - TPublicEvent<> is the public interface, while TEvent<> allows full control
// - TEvent<> can be optionally thread-safe (see TThreadSafeEvent<>)
//----------------------------------------------------------------------------
template <typename _Delegate, bool _ThreadSafe>
class TPublicEvent {
public:
    using FDelegate = _Delegate;
    using FHandle = FEventHandle;
    using FInvocationList = SPARSEARRAY_INSITU(Event, FDelegate);

    TPublicEvent() = default;

    TPublicEvent(const TPublicEvent&) = delete;
    TPublicEvent& operator =(const TPublicEvent&) = delete;

    TPublicEvent& operator =(TPublicEvent&&) = delete;

    TPublicEvent(TPublicEvent&& rvalue) NOEXCEPT
    :   _delegates(std::move(*rvalue._delegates.LockExclusive()))
    {}

    NODISCARD bool empty() const NOEXCEPT { return _delegates.LockShared()->empty(); }

    FHandle Add(FDelegate&& rfunc) {
        Assert(rfunc.Valid());
        const auto delegatesRW = _delegates.LockExclusive();
        return FHandle(delegatesRW->Emplace(std::move(rfunc)));
    }

    template <auto _OtherFunc, typename... _Extra>
    FHandle Bind(_Extra... extra) {
        return Add(FDelegate::template Bind<_OtherFunc>(std::forward<_Extra>(extra)...));
    }

    void Emplace(FDelegate&& rfunc) {
        Assert(rfunc.Valid());
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->EmplaceIt(std::move(rfunc));
    }

    template <auto _OtherFunc, typename... _Extra>
    void Emplace(_Extra... extra) {
        Emplace(FDelegate::template Bind<_OtherFunc>(std::forward<_Extra>(extra)...));
    }

    void Remove(FHandle& handle) {
        Assert(handle);
        const auto delegatesRW = _delegates.LockExclusive();
        VerifyRelease(delegatesRW->Remove(handle.Forget()));
    }

protected:
    TThreadSafe<FInvocationList, (!!_ThreadSafe
        ? EThreadBarrier::AtomicReadWriteLock
        : EThreadBarrier::DataRaceCheck )> _delegates;
};
//----------------------------------------------------------------------------
template <typename _Delegate, bool _ThreadSafe>
TPublicEvent<_Delegate, _ThreadSafe>& operator <<(TPublicEvent<_Delegate, _ThreadSafe>& publicEvent, _Delegate&& rcallback) {
    publicEvent.Emplace(std::move(rcallback));
    return publicEvent;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _ThreadSafe = false >
class TEvent;
template <typename T>
using TThreadSafeEvent = TEvent< T, true >;
//----------------------------------------------------------------------------
template <typename F, size_t _InSitu, typename _Ret, typename... _Args, bool _ThreadSafe>
class TEvent< TFunction<F, _InSitu, _Ret(_Args...)>, _ThreadSafe >
:   public TPublicEvent< TFunction<F, _InSitu, _Ret(_Args...)>, _ThreadSafe > {
public:
    using public_event_t = TPublicEvent< TFunction<F, _InSitu, _Ret(_Args...)>, _ThreadSafe >;

    using typename public_event_t::FDelegate;
    using typename public_event_t::FHandle;
    using typename public_event_t::FInvocationList;

    TEvent() = default;
    TEvent(TEvent&&) = default;

    using public_event_t::Add;
    using public_event_t::Emplace;
    using public_event_t::Remove;

    NODISCARD public_event_t& Public() NOEXCEPT { return *this; }

    PPE_FAKEBOOL_OPERATOR_DECL() {
        return _delegates.LockShared()->empty();
    }

    void operator ()(_Args... args) NOEXCEPT_IF(FDelegate::is_noexcept_v) {
        Invoke(std::forward<_Args>(args)...);
    }

    void Invoke(_Args... args) NOEXCEPT_IF(FDelegate::is_noexcept_v) {
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->RemoveIf([&](FDelegate& fn) -> bool {
            using result_t = decltype(fn(std::forward<_Args>(args)...));
            IF_CONSTEXPR(std::is_integral_v<result_t>) {
                if (not fn(std::forward<_Args>(args)...))
                    return true;
            }
            else {
                fn(std::forward<_Args>(args)...);
            }
            return false;
        });
    }

    void FireAndForget(_Args... args) NOEXCEPT_IF(FDelegate::is_noexcept_v) {
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->RemoveIf([&](const FDelegate& fn) -> bool {
            fn(std::forward<_Args>(args)...);
            return true; // remove all functions while iterating
        });
    }

    void Clear() {
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->Clear();
    }

private:
    using public_event_t::_delegates;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
