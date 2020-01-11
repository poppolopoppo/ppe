#pragma once

#include "HAL/Generic/GenericPlatformEndian.h"

#if PLATFORM_LINUX

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformEndian : FGenericPlatformEndian {
public:

    STATIC_CONST_INTEGRAL(EEndianness, Endianness, EEndianness::LittleEndian);

    static FORCE_INLINE bool SwapEndianness(bool i) NOEXCEPT { return i; }
    static FORCE_INLINE u8 SwapEndianness(u8 u) NOEXCEPT { return u; }
    static FORCE_INLINE u16 SwapEndianness(u16 u) NOEXCEPT { return ::__builtin_bswap16(u); }
    static FORCE_INLINE u32 SwapEndianness(u32 u) NOEXCEPT { return ::__builtin_bswap32(u); }
    static FORCE_INLINE u64 SwapEndianness(u64 u) NOEXCEPT { return ::__builtin_bswap64(u); }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
