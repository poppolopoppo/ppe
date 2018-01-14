#pragma once

#include <intrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <limits.h>

#if 0 && defined(CPP_VISUALSTUDIO)
#   pragma intrinsic( __lzcnt )
#   pragma intrinsic( _tzcnt_u32 )
#   pragma intrinsic( __popcnt )
#   ifdef ARCH_X64
#       pragma intrinsic( __lzcnt64 )
#       pragma intrinsic( _tzcnt_u64 )
#       pragma intrinsic( __popcnt64 )
#   endif
#   pragma intrinsic( _mm_lddqu_si128 )
#   pragma intrinsic( _mm_set1_epi8 )
#   pragma intrinsic( _mm_movemask_epi8 )
#   pragma intrinsic( _mm_cmpeq_epi8 )
#endif

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// lzcnt:   leading zero count
// tzcnt:   trailing zero count
// popcnt:  number of bits set to 1
//
#if defined(PLATFORM_WINDOWS)
FORCE_INLINE u32 lzcnt (u32 u) { return __lzcnt(u); }
FORCE_INLINE u32 tzcnt (u32 u) { return _tzcnt_u32(u); }
FORCE_INLINE u32 popcnt(u32 u) { return __popcnt(u); }
#   ifdef ARCH_X64
FORCE_INLINE u64 lzcnt (u64 u) { return __lzcnt64(u); }
FORCE_INLINE u64 tzcnt (u64 u) { return _tzcnt_u64(u); }
FORCE_INLINE u64 popcnt(u64 u) { return __popcnt64(u); }
#   endif
#elif defined(PLATFORM_LINUX)
//x ^ 31 = 31 - x, but gcc does not optimize 31 - __builtin_clz(x) to bsr(x), but generates 31 - (bsr(x) ^ 31)
FORCE_INLINE u32 lzcnt (u32 u) { return __builtin_clz(u); }
FORCE_INLINE u32 tzcnt (u32 u) { return __builtin_ctz(u); }
FORCE_INLINE u32 popcnt(u32 u) { return __builtin_popcount(u); }
#   ifdef ARCH_X64
FORCE_INLINE u64 lzcnt (u64 u) { return __builtin_clzll(u); }
FORCE_INLINE u64 tzcnt (u64 u) { return __builtin_ctzll(u); }
FORCE_INLINE u64 popcnt(u64 u) { return __builtin_popcount(u); }
#   endif
#else
#   error "unsupported platform !"
#endif
//----------------------------------------------------------------------------
template <typename T>
inline constexpr T popcnt_constexpr(T v) {
    // https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    v = v - ((v >> 1) & (T)~(T)0/3);                                // temp
    v = (v & (u64)~(u64)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);       // temp
    v = (v + (v >> 4)) & (T)~(T)0/255*15;                           // temp
    return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT;   // count
}
//----------------------------------------------------------------------------
// number of bits set to one (support u64 on ARCH_X86)
inline u64 popcnt64(u64 v) {
#ifdef ARCH_X86
    return popcnt_constexpr(v);
#else
    return popcnt(v);
#endif
}
//----------------------------------------------------------------------------
// load 16 bytes, compare them to match and return a 16 bits mask with 1 where equal
FORCE_INLINE size_t LoadCmpMoveMask8(u8 match, const void* data) {
    __m128i u8_16 = _mm_lddqu_si128((const __m128i*)data);
    return size_t(_mm_movemask_epi8(_mm_cmpeq_epi8(u8_16, _mm_set1_epi8(match))));
}
//----------------------------------------------------------------------------
// load 16 bytes, compare them to match and return a 16 bits mask with 1 where equal
FORCE_INLINE void LoadCmpMoveMask8(
    size_t* mask0, u8 match0,
    size_t* mask1, u8 match1,
    const void* data ) {
    __m128i u8_16 = _mm_lddqu_si128((const __m128i*)data);
    __m128i m0 = _mm_cmpeq_epi8(u8_16, _mm_set1_epi8(match0));
    __m128i m1 = _mm_cmpeq_epi8(u8_16, _mm_set1_epi8(match1));
    *mask0 = size_t(_mm_movemask_epi8(m0));
    *mask1 = size_t(_mm_movemask_epi8(m1));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline constexpr size_t IsPow2(size_t v) {
    return (v && !(v & (v - 1)));
}
//----------------------------------------------------------------------------
inline size_t FloorLog2(size_t value) {
    return lzcnt(value);
}
//----------------------------------------------------------------------------
inline size_t CeilLog2(size_t value) {
    return (IsPow2(value) ? 0 : 1) + FloorLog2(value);
}
//----------------------------------------------------------------------------
inline size_t NextPow2(size_t value) {
    return size_t(1u) << CeilLog2(value);
}
//----------------------------------------------------------------------------
inline size_t PrevOrEqualPow2(size_t value) {
    return size_t(1u) << FloorLog2(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t N>
struct TLog2 {
    STATIC_ASSERT(N > 0);
    STATIC_CONST_INTEGRAL(size_t, value, 1 + TLog2< (N >> 1) >::value);
};
//----------------------------------------------------------------------------
template <>
struct TLog2<1> {
    STATIC_CONST_INTEGRAL(size_t, value, 0);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
