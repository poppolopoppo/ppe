#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEndianness {
    LittleEndian    = 0,
    BigEndian       = 1,
    NetworkEndian   = BigEndian,
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformEndian {
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
