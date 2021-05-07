#pragma once

#include "Core.h"

#include "Container/SparseArray.h"
#include "Misc/EventHandle.h"
#include "Misc/Function.h"
#include "Thread/AtomicSpinLock.h"

#if USE_PPE_ASSERT
#   include "Meta/ThreadResource.h"
#endif

#define PUBLIC_EVENT(_NAME, ...) \
    private: \
        ::PPE::TEvent< __VA_ARGS__ , false > CONCAT(_, _NAME); \
    public: \
        ::PPE::TPublicEvent< __VA_ARGS__ , false >& _NAME() { \
            return CONCAT(_, _NAME).Public(); \
        }

#define THREADSAFE_EVENT(_NAME, ...) \
    private: \
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
namespace details {
template <bool _ThreadSafe>
class TEventTraits_
#if USE_PPE_ASSERT
    : Meta::FThreadResource
#endif
{
public:
    struct FScopeLock
    {
#if USE_PPE_ASSERT
        const TEventTraits_& Traits;
        FScopeLock(const TEventTraits_& traits)
        :   Traits(traits) {
            THREADRESOURCE_CHECKACCESS(&Traits);
        }
        ~FScopeLock() {
            THREADRESOURCE_CHECKACCESS(&Traits);
        }
#else
        CONSTEXPR FScopeLock(const TEventTraits_&) NOEXCEPT {}
#endif
    };
};
template <>
class TEventTraits_<true> {
public:
    mutable FAtomicSpinLock Barrier;
    struct FScopeLock : FAtomicSpinLock::FScope {
        FScopeLock(const TEventTraits_& traits) NOEXCEPT
        :   FAtomicSpinLock::FScope(traits.Barrier)
        {}
    };
};
template <typename _Delegate, typename _Traits>
class TPublicEvent_ : protected _Traits {
public:
    using FDelegate = _Delegate;
    using FHandle = FEventHandle;
    using FInvocationList = SPARSEARRAY_INSITU(Event, FDelegate);

    TPublicEvent_() NOEXCEPT {}

    TPublicEvent_(const TPublicEvent_&) = delete;
    TPublicEvent_& operator =(const TPublicEvent_&) = delete;

    TPublicEvent_(TPublicEvent_&&) = delete;
    TPublicEvent_& operator =(TPublicEvent_&&) = delete;

    bool empty() const { return _delegates.empty(); }

    FHandle Add(FDelegate&& rfunc) {
        Assert(rfunc);
        const FScopeLock scopeLock(*this);
        return FHandle(_delegates.Emplace(std::move(rfunc)));
    }

    void Emplace(FDelegate&& rfunc) {
        Assert(rfunc);
        const FScopeLock scopeLock(*this);
        _delegates.Emplace(std::move(rfunc));
    }

    void FireAndForget(FDelegate&& rfunc) {
        Assert(rfunc);
        const FScopeLock scopeLock(*this);
        const auto it = _delegates.EmplaceIt(std::move(rfunc));
        it->SetFireAndForget(true);
    }

    void Remove(FHandle& handle) {
        Assert(handle);
        const FScopeLock scopeLock(*this);
        VerifyRelease(_delegates.Remove(handle.Forget()));
    }

protected:
    using typename _Traits::FScopeLock;
    FInvocationList _delegates;
};
} //!details
//----------------------------------------------------------------------------
template <typename _Delegate, bool _ThreadSafe = false >
using TPublicEvent = details::TPublicEvent_<_Delegate, details::TEventTraits_<_ThreadSafe> >;
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

    public_event_t& Public() { return *this; }

    PPE_FAKEBOOL_OPERATOR_DECL() {
        const FScopeLock scopeLock(*this);
        return _delegates.empty();
    }

    void operator ()(_Args... args) {
        Invoke(std::forward<_Args>(args)...);
    }

    void Invoke(_Args... args) {
        const FScopeLock scopeLock(*this);
        _delegates.RemoveIf([&](const FDelegate& fn) -> bool {
            fn(std::forward<_Args>(args)...);
            return fn.FireAndForget();
        });
    }

    void FireAndForget(_Args... args) {
        const FScopeLock scopeLock(*this);
        _delegates.RemoveIf([&](const FDelegate& fn) -> bool {
            fn(std::forward<_Args>(args)...);
            return true; // remove all functions while iterating
        });
    }

    void Clear() {
        const FScopeLock scopeLock(*this);
        _delegates.Clear();
    }

private:
    using typename public_event_t::FScopeLock;
    using public_event_t::_delegates;
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
} //!namespace PPE
