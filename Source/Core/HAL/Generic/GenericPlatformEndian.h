#pragma once

#include "Core/HAL/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEndianness {
    LittleEndian    = 0,
    BigEndian       = 1,
    NetworkEndian   = BigEndian,
};
//----------------------------------------------------------------------------
struct CORE_API FGenericPlatformEndian {
public: // must be defined for every platform

    STATIC_CONST_INTEGRAL(EEndianness, Endianness, EEndianness::LittleEndian);

    static FORCE_INLINE bool SwapEndianness(bool i) NOEXCEPT { return i; }

    static FORCE_INLINE u8 SwapEndianness(u8 u) NOEXCEPT { return u; }

    static FORCE_INLINE u16 SwapEndianness(u16 u) NOEXCEPT {
        return ((((u) >> 8) & 0xff) | (((u) & 0xff) << 8));
    }

    static FORCE_INLINE u32 SwapEndianness(u32 u) NOEXCEPT {
        return ((((u) & 0xff000000) >> 24)  |
                (((u) & 0x00ff0000) >> 8)   |
                (((u) & 0x0000ff00) << 8)   |
                (((u) & 0x000000ff) << 24)  );
    }

    static FORCE_INLINE u64 SwapEndianness(u64 u) NOEXCEPT {
        return ((((u) & 0xff00000000000000ull) >> 56) |
                (((u) & 0x00ff000000000000ull) >> 40) |
                (((u) & 0x0000ff0000000000ull) >> 24) |
                (((u) & 0x000000ff00000000ull) >> 8)  |
                (((u) & 0x00000000ff000000ull) << 8)  |
                (((u) & 0x0000000000ff0000ull) << 24) |
                (((u) & 0x000000000000ff00ull) << 40) |
                (((u) & 0x00000000000000ffull) << 56) );
    }

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
    static FORCE_INLINE void SwapInPlace(T* p, TCheckOverload<T, u8>* = nullptr) NOEXCEPT {
        *(u8*)p = FPlatformEndian::SwapEndianness(*(const u8*)p);
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T* p, TCheckOverload<T, u16>* = nullptr) NOEXCEPT {
        *(u16*)p = FPlatformEndian::SwapEndianness(*(const u16*)p);
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T* p, TCheckOverload<T, u32>* = nullptr) NOEXCEPT {
        *(u32*)p = FPlatformEndian::SwapEndianness(*(const u32*)p);
    }

    template <typename T>
    static FORCE_INLINE void SwapInPlace(T* p, TCheckOverload<T, u64>* = nullptr) NOEXCEPT {
        *(u64*)p = FPlatformEndian::SwapEndianness(*(const u64*)p);
    }

    //------------------------------------------------------------------------
    // These helpers will dodge endianness swapping if it's already match
    // current platform's :

    template <typename T>
    static void BigEndianInPlace(T* p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::BigEndian)
            FPlatformEndian::SwapInPlace(p);
    }

    template <typename T>
    static void LittleEndianInPlace(T* p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::LittleEndian)
            FPlatformEndian::SwapInPlace(p);
    }

    template <typename T>
    static void NetworkEndianInPlace(T* p) NOEXCEPT {
        IF_CONSTEXPR(FPlatformEndian::Endianness != EEndianness::NetworkEndian)
            FPlatformEndian::SwapInPlace(p);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
