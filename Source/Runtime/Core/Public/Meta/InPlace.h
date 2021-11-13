#pragma once

#include "Meta/Aliases.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Assert.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TInPlace {
    POD_STORAGE(T) Storage;
    ARG0_IF_ASSERT(bool Available{ false };)

    TInPlace() = default;
#if USE_PPE_ASSERT
    ~TInPlace() {
        Assert_NoAssume(not Available);
    }
#endif

    void* data() { return (&Storage); }
    const void* data() const { return (&Storage); }

    T* Get() {
        Assert_NoAssume(Available);
        Assert_NoAssume(Meta::IsAlignedPow2(alignof(T), data()));
        return static_cast<T*>(data());
    }
    const T* Get() const { return const_cast<TInPlace*>(this)->Get(); }

    template <typename... _Args>
    TInPlace& Construct(_Args&&... args) {
        ARG0_IF_ASSERT(
            Assert_NoAssume(not Available);
            Available = true;
        )
        Meta::Construct(static_cast<T*>(data()), std::forward<_Args>(args)...);
        return (*this);
    }

    void Destroy() {
        ARG0_IF_ASSERT(
            Assert_NoAssume(Available);
            Available = false;
        )
        Meta::Destroy(static_cast<T*>(data()));
    }

    void Discard() {
#if USE_PPE_ASSERT
        Assert_NoAssume(Available);
        Available = false;
        FPlatformMemory::Memdeadbeef(&Storage, sizeof(Storage));
#endif
    }

    T* operator ->() { return Get(); }
    const T* operator ->() const { return Get(); }

    T& operator *() { return *Get(); }
    const T& operator *() const { return *Get(); }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
