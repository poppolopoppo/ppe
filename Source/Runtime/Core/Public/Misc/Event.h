#pragma once

#include "Core.h"

#include "Container/SparseArray.h"
#include "Misc/EventHandle.h"
#include "Misc/Function.h"
#include "Thread/ThreadSafe.h"

#if USE_PPE_ASSERT
#   include "Meta/ThreadResource.h"
#endif

#define PUBLIC_EVENT(_NAME, ...) \
    protected: \
        ::PPE::TEvent< __VA_ARGS__ , false > CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent< __VA_ARGS__ , false >& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

#define THREADSAFE_EVENT(_NAME, ...) \
    protected: \
        ::PPE::TEvent< __VA_ARGS__ , true > CONCAT(_, _NAME); \
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

    TPublicEvent() NOEXCEPT {}

    TPublicEvent(const TPublicEvent&) = delete;
    TPublicEvent& operator =(const TPublicEvent&) = delete;

    TPublicEvent(TPublicEvent&&) = delete;
    TPublicEvent& operator =(TPublicEvent&&) = delete;

    bool empty() const { return _delegates.LockShared()->empty(); }

    NODISCARD FHandle Add(FDelegate&& rfunc) {
        Assert(rfunc);
        const auto delegatesRW = _delegates.LockExclusive();
        return FHandle(delegatesRW->Emplace(std::move(rfunc)));
    }

    template <auto _OtherFunc, typename... _Extra>
    NODISCARD FHandle Bind(_Extra... extra) {
        return Add(FDelegate::template Bind<_OtherFunc>(std::forward<_Extra>(extra)...));
    }

    void Emplace(FDelegate&& rfunc) {
        Assert(rfunc);
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->Emplace(std::move(rfunc));
    }

    void FireAndForget(FDelegate&& rfunc) {
        Assert(rfunc);
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->EmplaceIt(std::move(rfunc))->SetFireAndForget(true);
    }

    void Remove(FHandle& handle) {
        Assert(handle);
        const auto delegatesRW = _delegates.LockExclusive();
        VerifyRelease(delegatesRW->Remove(handle.Forget()));
    }

    size_t RemoveAll(const FDelegate& func) {
        Assert(func);
        const auto delegatesRW = _delegates.LockExclusive();
        return delegatesRW->RemoveIf([&func](const FDelegate& registered) {
            return (registered == func);
        });
    }

protected:
    TThreadSafe<FInvocationList, (!!_ThreadSafe
        ? EThreadBarrier::RWLock
        : EThreadBarrier::DataRaceCheck )> _delegates;
};
//----------------------------------------------------------------------------
template <typename _Delegate, bool _ThreadSafe, typename T, class = Meta::TEnableIf<_Delegate::template is_callable_v<T>> >
TPublicEvent<_Delegate, _ThreadSafe>& operator <<(TPublicEvent<_Delegate, _ThreadSafe>& publicEvent, T&& rcallback) {
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
template <typename _Ret, typename... _Args, size_t _InSitu, bool _ThreadSafe>
class TEvent< TFunction<_Ret(_Args...), _InSitu>, _ThreadSafe >
:   public TPublicEvent< TFunction<_Ret(_Args...), _InSitu>, _ThreadSafe > {
public:
    using public_event_t = TPublicEvent< TFunction<_Ret(_Args...), _InSitu>, _ThreadSafe >;

    using typename public_event_t::FDelegate;
    using typename public_event_t::FHandle;
    using typename public_event_t::FInvocationList;

    TEvent() = default;

    using public_event_t::Add;
    using public_event_t::Emplace;
    using public_event_t::Remove;
    using public_event_t::RemoveAll;

    public_event_t& Public() NOEXCEPT { return *this; }

    PPE_FAKEBOOL_OPERATOR_DECL() {
        return _delegates.LockShared()->empty();
    }

    void operator ()(_Args... args) {
        Invoke(std::forward<_Args>(args)...);
    }

    void Invoke(_Args... args) {
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->RemoveIf([&](FDelegate& fn) -> bool {
            fn(std::forward<_Args>(args)...);
            return fn.IsFireAndForget();
        });
    }

    void FireAndForget(_Args... args) {
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
template <typename _Ret, typename... _Args, size_t _InSitu, bool _ThreadSafe>
class TEvent< TFunction<_Ret(_Args...) NOEXCEPT, _InSitu>, _ThreadSafe >
:   public TPublicEvent< TFunction<_Ret(_Args...) NOEXCEPT, _InSitu>, _ThreadSafe > {
public:
    using public_event_t = TPublicEvent< TFunction<_Ret(_Args...) NOEXCEPT, _InSitu>, _ThreadSafe >;

    using typename public_event_t::FDelegate;
    using typename public_event_t::FHandle;
    using typename public_event_t::FInvocationList;

    TEvent() = default;

    using public_event_t::Add;
    using public_event_t::Emplace;
    using public_event_t::Remove;
    using public_event_t::RemoveAll;

    public_event_t& Public() NOEXCEPT { return *this; }

    PPE_FAKEBOOL_OPERATOR_DECL() {
        return (not _delegates.LockShared()->empty());
    }

    void operator ()(_Args... args) NOEXCEPT {
        Invoke(std::forward<_Args>(args)...);
    }

    void Invoke(_Args... args) NOEXCEPT {
        const auto delegatesRW = _delegates.LockExclusive();
        delegatesRW->RemoveIf([&](FDelegate& fn) -> bool {
            fn(std::forward<_Args>(args)...);
            return fn.IsFireAndForget();
        });
    }

    void FireAndForget(_Args... args) NOEXCEPT {
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
