#pragma once

#include "ResourceId.h"
#include "RHI_fwd.h"

#include <atomic>

#if USE_PPE_RHITRACE
#   include "Meta/TypeInfo.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FResourceBaseProxy : Meta::FNonCopyable {
public:
    enum class EState : u32 {
        Initial = 0,
        Failed,
        Created,
    };

    using FInstanceID = FRawImageID::FInstanceID;

    explicit FResourceBaseProxy(FInstanceID instanceId) : _instanceId(instanceId) {}

#if USE_PPE_DEBUG
    ~FResourceBaseProxy() {
        Assert_NoAssume(IsDestroyed());
        Assert_NoAssume(RefCount() == 0);
    }
#endif

    bool Valid() const { return IsCreated(); }
    bool IsCreated() const { return (State_() == EState::Created); }
    bool IsDestroyed() const { return (State_() <= EState::Failed); }

    FInstanceID InstanceID() const { return checked_cast<FInstanceID>(_instanceId.load(std::memory_order_relaxed)); }
    int RefCount() const { return _refCounter.load(std::memory_order_relaxed); }

#if !USE_PPE_RHITRACE
    FORCE_INLINE void AddRef(int refCount = 1) const NOEXCEPT {
        Assert(refCount > 0);
        _refCounter.fetch_add(refCount, std::memory_order_relaxed);
    }
    NODISCARD FORCE_INLINE bool RemoveRef(int refCount) const NOEXCEPT {
        if (_refCounter.fetch_sub(refCount, std::memory_order_relaxed) == refCount) {
            std::atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
        return false;
    }
#endif

protected:
    EState State_() const { return _state.load(std::memory_order_relaxed); }

    // instance counter used to detect deprecated handles
    std::atomic<u32> _instanceId{ 0 };
    std::atomic<EState> _state{ EState::Initial };
    // reference counter may be used for cached resources like samples, pipeline layout and other
    mutable std::atomic<int> _refCounter{ 0 };

#if USE_PPE_RHIDEBUG
    template <typename T>
    using if_has_debugname_ = decltype(std::declval<T&>().DebugName());
#endif
};
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <typename T>
class CACHELINE_ALIGNED TResourceProxy final : public FResourceBaseProxy {
public:
    using value_type = T;

    TResourceProxy() NOEXCEPT;
    TResourceProxy(TResourceProxy&& rvalue) NOEXCEPT;

    TResourceProxy(const TResourceProxy& ) = delete;
    TResourceProxy& operator =(TResourceProxy&& rvalue) NOEXCEPT = delete;

    template <typename... _Args>
    explicit TResourceProxy(_Args&&... args);

    value_type& Data() { return _data; }
    const value_type& Data() const { return _data; }

    template <typename... _Args>
    NODISCARD bool Construct(_Args&&... args);
    template <typename... _Args>
    void TearDown(_Args&&... args);
    template <typename... _Args>
    void TearDown_Force(_Args&&... args);

    bool operator ==(const TResourceProxy& other) const { return (_data == other._data); }
    bool operator !=(const TResourceProxy& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const TResourceProxy& proxy) {
        return hash_value(proxy._data);
    }

#if USE_PPE_RHIDEBUG
    auto DebugName() const {
        IF_CONSTEXPR(Meta::has_defined_v< if_has_debugname_, T>) {
            return _data.DebugName();
        } else {
            return "anonymous";
        }
    }
#endif

#if USE_PPE_RHITRACE
    FORCE_INLINE void AddRef(int refCount = 1) const {
        Assert(refCount > 0);
        auto x = _refCounter.fetch_add(refCount, std::memory_order_relaxed);
        Unused(x);
        RHI_TRACE("AddRef", Meta::type_info<T>.name, InstanceID(), DebugName(), x, refCount);
        ONLY_IF_RHIDEBUG(if (x > 100) PPE_DEBUG_BREAK());
    }
    NODISCARD FORCE_INLINE bool RemoveRef(int refCount) const {
        auto x = _refCounter.fetch_sub(refCount, std::memory_order_relaxed);
        RHI_TRACE("RemoveRef", Meta::type_info<T>.name, InstanceID(), DebugName(), x, refCount);
        if (x == refCount) {
            std::atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
        return false;
    }
#endif

private:
    value_type _data;

    static u16 NextInstanceID_() NOEXCEPT {
        ONE_TIME_INITIALIZE(std::atomic<unsigned>, GInstanceID);
        u16 instanceId;
        do {
            instanceId = static_cast<u16>(GInstanceID.fetch_add(1, std::memory_order_relaxed));
        } while (0 == instanceId);
        return instanceId;
    }
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <typename T>
TResourceProxy<T>::TResourceProxy() NOEXCEPT
:   FResourceBaseProxy(NextInstanceID_())
{}
//----------------------------------------------------------------------------
template <typename T>
TResourceProxy<T>::TResourceProxy(TResourceProxy&& rvalue) NOEXCEPT
:   FResourceBaseProxy(rvalue.InstanceID())
,   _data(std::move(rvalue._data)) {
    Assert_NoAssume(rvalue._refCounter.load(std::memory_order_relaxed) == 0);
    rvalue._instanceId.store(0, std::memory_order_release);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename... _Args>
TResourceProxy<T>::TResourceProxy(_Args&&... args)
:   FResourceBaseProxy(NextInstanceID_())
,   _data(std::forward<_Args>(args)...)
{}
//----------------------------------------------------------------------------
template <typename T>
template <typename... _Args>
bool TResourceProxy<T>::Construct(_Args&&... args) {
    Assert_NoAssume(IsDestroyed());
    Assert_NoAssume(RefCount() == 0);

    STATIC_ASSERT(not IsRefCountable<T>::value);

    const bool result = _data.Construct(std::forward<_Args>(args)...);

    // read state and flush cache
    _state.store(result ? EState::Created : EState::Failed, std::memory_order_release);

    RHI_TRACE("Construct", Meta::type_info<T>.name, InstanceID(), DebugName(), RefCount(), result);

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename... _Args>
void TResourceProxy<T>::TearDown(_Args&&... args) {
    Assert_NoAssume(0 == RefCount());
    TearDown_Force(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename... _Args>
void TResourceProxy<T>::TearDown_Force(_Args&&... args) {
    Assert(not IsDestroyed());

    RHI_TRACE("TearDown", Meta::type_info<T>.name, InstanceID(), DebugName(), RefCount());

    _data.TearDown(std::forward<_Args>(args)...);

    // update atomics and flush cache
    _refCounter.store( 0, std::memory_order_relaxed );
    _state.store( EState::Initial, std::memory_order_relaxed );
    _instanceId.store(0, std::memory_order_release);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
