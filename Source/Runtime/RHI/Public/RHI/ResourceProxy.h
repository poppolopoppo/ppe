#pragma once

#include "ResourceId.h"
#include "RHI_fwd.h"

#include <atomic>

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

    explicit FResourceBaseProxy(u16 instanceId)
    :   _instanceId(instanceId) {
        Assert_NoAssume(_instanceId); // 0 is considered invalid
    }

#if USE_PPE_DEBUG
    ~FResourceBaseProxy() {
        Assert_NoAssume(IsDestroyed());
        Assert_NoAssume(RefCount() == 0);
    }
#endif

    bool Valid() const { return IsCreated(); }
    bool IsCreated() const { return (State_() == EState::Created); }
    bool IsDestroyed() const { return (State_() <= EState::Failed); }

    FInstanceID InstanceID() const { return _instanceId; }
    int RefCount() const { return _refCounter.load(std::memory_order_relaxed); }

    void AddRef() const {
        _refCounter.fetch_add(1, std::memory_order_relaxed);
    }
    NODISCARD bool RemoveRef(int refCount) const {
        return (_refCounter.fetch_sub(refCount, std::memory_order_relaxed) == refCount);
    }

protected:
    EState State_() const { return _state.load(std::memory_order_relaxed); }


    const u16 _instanceId;
    std::atomic<EState> _state{ EState::Initial };

    // reference counter may be used for cached resources like samples, pipeline layout and other
    mutable std::atomic<int> _refCounter{ 0 };
};
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <typename T>
class CACHELINE_ALIGNED TResourceProxy final : public FResourceBaseProxy {
public:
    using value_type = T;

    TResourceProxy() NOEXCEPT;

    TResourceProxy(const TResourceProxy& ) = delete;
    TResourceProxy(TResourceProxy&& rvalue) NOEXCEPT;
    TResourceProxy& operator =(TResourceProxy&& rvalue) NOEXCEPT = delete;

    explicit TResourceProxy(T&& rdata) NOEXCEPT;

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
:   FResourceBaseProxy(NextInstanceID_())
,   _data(std::move(rvalue._data)) {
    Assert_NoAssume(rvalue._refCounter.load(std::memory_order_relaxed) == 0);
}
//----------------------------------------------------------------------------
template <typename T>
TResourceProxy<T>::TResourceProxy(T&& rdata) NOEXCEPT
:   FResourceBaseProxy(NextInstanceID_())
,   _data(std::move(rdata))
{}
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

    const bool result = _data.Construct(std::forward<_Args>(args)...);

    // read state and flush cache
    _state.store(result ? EState::Created : EState::Failed, std::memory_order_release);

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

    _data.TearDown(std::forward<_Args>(args)...);

    // update atomics and flush cache
    _refCounter.store( 0, std::memory_order_relaxed );
    _state.store( EState::Initial, std::memory_order_relaxed );
    ONLY_IF_ASSERT(const_cast<u16&>(_instanceId) = 0); // invalidate all references
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
