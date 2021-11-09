#pragma once

// simple wrapper for target platform

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(PlatformAtomics)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPlatformAtomics : public PPE_HAL_TARGETALIAS(PlatformAtomics) {
public: // generic helpers
    template <typename T>
    static FORCE_INLINE T* ExchangePtr(volatile T * *dst, T * exg) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), dst));
        return (T*)PPE_HAL_TARGETALIAS(PlatformAtomics)::Exchange((volatile intptr_t*)dst, (intptr_t)exg);
    }

    template <typename T>
    static FORCE_INLINE T* CompareExchangePtr(volatile T * *dst, T * exg, T * cmp) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), dst));
        return (T*)PPE_HAL_TARGETALIAS(PlatformAtomics)::CompareExchange((volatile intptr_t*)dst, (intptr_t)exg, (intptr_t)cmp);
    }

    template <typename T>
    static FORCE_INLINE T* FetchPtr(const T * *src) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), src));
        return (T*)PPE_HAL_TARGETALIAS(PlatformAtomics)::Fetch((volatile const intptr_t*)src);
    }

    template <typename T>
    static FORCE_INLINE T* FetchPtr_Relaxed(const T * *src) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), src));
        return (T*)PPE_HAL_TARGETALIAS(PlatformAtomics)::Fetch_Relaxed((volatile const intptr_t*)src);
    }

    template <typename T>
    static FORCE_INLINE void StorePtr(T * *dst, T * val) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), dst));
        PPE_HAL_TARGETALIAS(PlatformAtomics)::Store((volatile intptr_t*)dst, (intptr_t)val);
    }

    template <typename T>
    static FORCE_INLINE void StorePtr_Relaxed(T * *dst, T * val) NOEXCEPT {
        Assert(Meta::IsAlignedPow2(sizeof(void*), dst));
        PPE_HAL_TARGETALIAS(PlatformAtomics)::Store_Relaxed((volatile intptr_t*)dst, (intptr_t)val);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
