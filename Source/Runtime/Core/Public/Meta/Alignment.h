#pragma once

#include "Meta/Aliases.h"
#include "Meta/TypeTraits.h"

#include <new>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ROUND_TO_NEXT_4(v)   ((((uintptr_t)(v)) + 3) & ~3)
#define ROUND_TO_NEXT_8(v)   ((((uintptr_t)(v)) + 7) & ~7)
#define ROUND_TO_NEXT_16(v)  ((((uintptr_t)(v)) + 15) & ~15)
#define ROUND_TO_NEXT_32(v)  ((((uintptr_t)(v)) + 31) & ~31)
#define ROUND_TO_NEXT_64(v)  ((((uintptr_t)(v)) + 63) & ~63)
#define ROUND_TO_NEXT_128(v) ((((uintptr_t)(v)) + 127) & ~127)
#define ROUND_TO_NEXT_64K(v) ((((uintptr_t)(v)) + 64*1024-1) & ~(64*1024-1))
//----------------------------------------------------------------------------
#if 0
#   define ALIGN(_BOUNDARY) __declspec(align(_BOUNDARY))
#else
#   define ALIGN(_BOUNDARY) alignas(_BOUNDARY)
#endif
//----------------------------------------------------------------------------
inline CONSTEXPR bool IsPow2(size_t u) { return ((u & (u - 1)) == 0 && u); }
//----------------------------------------------------------------------------
template <size_t _Pow>
inline CONSTEXPR bool IsPowOf(size_t u) {
    STATIC_ASSERT(IsPow2(_Pow) && _Pow > 2);
    for (; u >= _Pow; u /= _Pow);
    return (u == 1);
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
inline CONSTEXPR bool IsAligned(const size_t alignment, const uintptr_t v) {
    return (0 == (v & (alignment - 1)));
}
template <typename T>
inline bool IsAligned(const size_t alignment, const T* ptr) NOEXCEPT {
    return (0 == (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)));
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR T RoundToNext(const T v, TDontDeduce<T> alignment) {
    return ((0 == v) ? 0 : (v + alignment - static_cast<T>(1)) & ~(alignment - static_cast<T>(1)));
}
template <typename T, class = Meta::TEnableIf<std::is_integral_v<T>> >
inline CONSTEXPR T RoundToPrev(const T v, TDontDeduce<T> alignment) {
    return ((0 == v) ? 0 : v & ~(alignment - static_cast<T>(1)));
}
//----------------------------------------------------------------------------
template <typename T>
T* RoundToNext(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToNext(reinterpret_cast<uintptr_t>(p), alignment));
}
template <typename T>
T* RoundToPrev(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToPrev(reinterpret_cast<uintptr_t>(p), alignment));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PAGE_SIZE (4096u)
//----------------------------------------------------------------------------
#define ALLOCATION_BOUNDARY (16u)
//----------------------------------------------------------------------------
#define ALLOCATION_GRANULARITY (65536u)
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17 && !defined(PLATFORM_LINUX)
//  https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
#   define CACHELINE_SIZE (std::hardware_destructive_interference_size)
#   define ROUND_TO_NEXT_CACHELINE(v) ::PPE::Meta::RoundToNext((v), CACHELINE_SIZE)
#else
#   define CACHELINE_SIZE (64u)
#   define ROUND_TO_NEXT_CACHELINE(v) ROUND_TO_NEXT_64(v)
#endif
//----------------------------------------------------------------------------
#define CACHELINE_ALIGNED ALIGN(CACHELINE_SIZE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// By default std allocations are already aligned on ALLOCATION_BOUNDARY (16).
// Aligned allocations induce overhead so this will prevent us from calling it systematically.
//----------------------------------------------------------------------------
template <size_t _Alignment>
struct need_alignment_t {
    STATIC_ASSERT(IsPow2(_Alignment));
    STATIC_ASSERT(IsAligned(ALLOCATION_BOUNDARY, _Alignment));
    STATIC_CONST_INTEGRAL(bool, value, _Alignment > ALLOCATION_BOUNDARY);
};
//----------------------------------------------------------------------------
template <size_t _Alignment>
CONSTEXPR bool need_alignment_v = need_alignment_t<_Alignment>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
