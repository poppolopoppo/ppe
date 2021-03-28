#pragma once

#include "ResourceId.h"
#include "RHI_fwd.h"

#include <atomic>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class CACHELINE_ALIGNED TResourceProxy final : Meta::FNonCopyableNorMovable {
public:
    enum class EState : u32 {
        Initial = 0,
        Failed,
        Created,
    };

    using FResource = T;
    using FInstanceID = FRawImageID::instance_t;

    TResourceProxy() = default;

#if USE_PPE_DEBUG
    ~TResourceProxy() {
        Assert_NoAssume(IsDestroyed());
        Assert_NoAssume(RefCount() == 0);
    }
#endif

    bool IsCreated() const { return (State_() == EState::Created); }
    bool IsDestroyed() const { return (State_() <= EState::Failed); }

    FInstanceID InstanceId() const { return {_instanceId.load(std::memory_order_relaxed)}; }
    int RefCount() const { return _refCounter.load(std::memory_order_relaxed); }

    FResource& Data() { return _data; }
    const FResource& Data() const { return _data; }

    void AddRef() const {
        _refCounter.fetch_add(1, std::memory_order_relaxed);
    }
    NODISCARD bool RemoveRef(int refCount) const {
        return (_refCounter.fetch_sub(refCount, std::memory_order_relaxed) == refCount);
    }

    template <typename... _Args>
    bool Create(_Args&&... args);
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
    EState State_() const { return _state.load(std::memory_order_relaxed); }

    std::atomic<u32> _instanceId{ 0 };
    std::atomic<EState> _state{ EState::Initial };

    FResource _data;

    // reference counter may be used for cached resources like samples, pipeline layout and other
    mutable std::atomic<int> _refCounter{ 0 };
};
//----------------------------------------------------------------------------
template <typename T>
template <typename... _Args>
bool TResourceProxy<T>::Create(_Args&&... args) {
    Assert_NoAssume(IsDestroyed());
    Assert_NoAssume(RefCount() == 0);

    const bool result = _data.Create(std::forward<_Args>(args...));

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

    _data.TearDown(std::forward<_Args>(args));

    // update atomics and flush cache
    _refCounter.store( 0, std::memory_order_relaxed );
    _state.store( EState::Initial, std::memory_order_relaxed );
    _instanceId.fetch_add( 1, std::memory_order_release );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
