#pragma once

// simple wrapper for target platform

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(PlatformEndian)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPlatformEndian : public PPE_HAL_TARGETALIAS(PlatformEndian) {
public: // generic helpers

    //------------------------------------------------------------------------
    // Swapping in place avoids a nasty bug with floating point values !
    //  - When swapping a float it's liking to become a DEN or a NAN.
    //  - When assigning this degenerated value to a FPU register it will
    //    detect the issue and clean the degenerated value, leading the
    //    method to return a wrong value.
    //  - The solution to avoid is to avoid completely FPU register by
    //    aliasing value to an integral type with the same stride.
    // So please be sure to use these methods exclusively :

    template <typename T, typename _Alias>
    using TCheckOverload = Meta::TEnableIf<
        (std::is_integral_v<T> || std::is_floating_point_v<T>) &&
        (sizeof(T) == sizeof(_Alias))
    >;

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T * p, TCheckOverload<T, u8> * = nullptr) NOEXCEPT {
        *reinterpret_cast<u8*>(p) = PPE_HAL_TARGETALIAS(PlatformEndian)::SwapEndianness(*reinterpret_cast<const u8*>(p));
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T * p, TCheckOverload<T, u16> * = nullptr) NOEXCEPT {
        *reinterpret_cast<u16*>(p) = PPE_HAL_TARGETALIAS(PlatformEndian)::SwapEndianness(*reinterpret_cast<const u16*>(p));
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T * p, TCheckOverload<T, u32> * = nullptr) NOEXCEPT {
        *reinterpret_cast<u32*>(p) = PPE_HAL_TARGETALIAS(PlatformEndian)::SwapEndianness(*reinterpret_cast<const u32*>(p));
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T * p, TCheckOverload<T, u64> * = nullptr) NOEXCEPT {
        *reinterpret_cast<u64*>(p) = PPE_HAL_TARGETALIAS(PlatformEndian)::SwapEndianness(*reinterpret_cast<const u64*>(p));
    }

    template <typename T, size_t _Dim>
    static FORCE_INLINE void SwapInPlace(T(&arr)[_Dim]) NOEXCEPT {
        for (size_t i = 0; i < _Dim; ++i)
            SwapInPlace(&arr[i]);
    }

    //------------------------------------------------------------------------
    // These helpers will dodge endianness swapping if it's already match
    // current platform's :

    template <typename T>
    using TEnableIfSwappable = decltype(FPlatformEndian::SwapInPlace(std::declval<T>()))*;

    template <typename T, TEnableIfSwappable<T> = nullptr>
    static FORCE_INLINE void SwapInPlaceIFP(T p, EEndianness target) NOEXCEPT {
        if (Unlikely(FPlatformEndian::Endianness != target))
            SwapInPlace(p);
    }

    template <typename T, TEnableIfSwappable<T*> = nullptr>
    static FORCE_INLINE void SwapInPlaceIFP(T* p, size_t num, EEndianness target) NOEXCEPT {
        if (Unlikely(FPlatformEndian::Endianness != target)) {
            forrange(it, p, p + num)
                SwapInPlace(it);
        }
    }

    template <typename T, TEnableIfSwappable<T> = nullptr>
    static void BigEndianInPlace(T p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::BigEndian)
            SwapInPlace(p);
    }

    template <typename T, TEnableIfSwappable<T> = nullptr>
    static void LittleEndianInPlace(T p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::LittleEndian)
            SwapInPlace(p);
    }

    template <typename T, TEnableIfSwappable<T> = nullptr>
    static void NetworkEndianInPlace(T p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::NetworkEndian)
            SwapInPlace(p);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE