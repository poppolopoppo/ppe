#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Misc/TargetPlatform.h"

#if     defined(PLATFORM_WINDOWS)
#   define WITH_VISUAL_INTRINSIC_BYTESWAP
#   include <intrin.h>
#elif   defined(CPP_GCC) || defined (CPP_CLANG)
#   define WITH_GCC_BUILTIN_BSWAP
#else
#   error "unsupported platform !"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE i8 SwapEndianness(i8 val) { return val; }
FORCE_INLINE u8 SwapEndianness(u8 val) { return val; }
//----------------------------------------------------------------------------
inline u16 SwapEndianness(u16 val) {
#if     defined(WITH_VISUAL_INTRINSIC_BYTESWAP)
    return _byteswap_ushort(val);
#else
    return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));
#endif
}
inline i16 SwapEndianness(i16 val) { return static_cast<i16>(SwapEndianness(static_cast<u16>(val))); }
//----------------------------------------------------------------------------
inline u32 SwapEndianness(u32 val) {
#if     defined(WITH_VISUAL_INTRINSIC_BYTESWAP)
    return _byteswap_ulong(val);
#elif   defined(WITH_GCC_BUILTIN_BSWAP)
    STATIC_ASSERT(sizeof(u32) == sizeof(int32_t));
    return static_cast<u32>(__builtin_bswap32(static_cast<int32_t>(val)));
#else
    return ((((val) & 0xff000000) >> 24)    |
            (((val) & 0x00ff0000) >> 8)     |
            (((val) & 0x0000ff00) << 8)     |
            (((val) & 0x000000ff) << 24)    );
#endif
}
inline i32 SwapEndianness(i32 val) { return static_cast<i32>(SwapEndianness(static_cast<u32>(val))); }
//----------------------------------------------------------------------------
inline u64 SwapEndianness(u64 val) {
#if     defined(WITH_VISUAL_INTRINSIC_BYTESWAP)
    return _byteswap_uint64(val);
#elif   defined(WITH_GCC_BUILTIN_BSWAP)
    STATIC_ASSERT(sizeof(u64) == sizeof(int64_t));
    return static_cast<u64>(__builtin_bswap64(static_cast<int64_t>(val)));
#else
    return ((((val) & 0xff00000000000000ull) >> 56) |
            (((val) & 0x00ff000000000000ull) >> 40) |
            (((val) & 0x0000ff0000000000ull) >> 24) |
            (((val) & 0x000000ff00000000ull) >> 8)  |
            (((val) & 0x00000000ff000000ull) << 8)  |
            (((val) & 0x0000000000ff0000ull) << 24) |
            (((val) & 0x000000000000ff00ull) << 40) |
            (((val) & 0x00000000000000ffull) << 56) );
#endif
}
inline i64 SwapEndianness(i64 val) { return static_cast<i64>(SwapEndianness(static_cast<u64>(val))); }
//----------------------------------------------------------------------------
inline float SwapEndianness(float val) {
    const u32 r = SwapEndianness(*reinterpret_cast<const u32*>(&val));
    return *reinterpret_cast<const float*>(&r);
}
inline double SwapEndianness(double val) {
    const u64 r = SwapEndianness(*reinterpret_cast<const u64*>(&val));
    return *reinterpret_cast<const double*>(&r);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void InplaceSwapEndianness(const TMemoryView<T>& data) {
    for (T& val : data)
        val = SwapEndianness(val);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, EEndianness _From, EEndianness _To>
struct TSwapEndianness_ {
    FORCE_INLINE T operator ()(T val)
    {
        return SwapEndianness(val);
    }
};
template <typename T, EEndianness _FromTo>
struct TSwapEndianness_<T, _FromTo, _FromTo> {
    FORCE_INLINE T operator ()(T val)
    {
        return val;
    }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename T>
inline T BigEndian(T val) {
    return details::TSwapEndianness_<T, EEndianness::Current, EEndianness::BigEndian>()(val);
}
//----------------------------------------------------------------------------
template <typename T>
inline T LittleEndian(T val) {
    return details::TSwapEndianness_<T, EEndianness::Current, EEndianness::LittleEndian>()(val);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T NetworkEndian(T val) {
    return LittleEndian(val);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
