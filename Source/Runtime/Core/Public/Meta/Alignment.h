#pragma once

#include "Meta/Aliases.h"
#include "Meta/TypeTraits.h"

#include <cassert> // for early assertions
#include <bit> // for std::bit_cast<>
#include <new>

#include "Assert.h"

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
template <typename T>
NODISCARD inline CONSTEXPR CONSTF Meta::TEnableIf<std::is_unsigned_v<T>, bool> IsPow2OrZero(T u) {
    return ((u & (u - 1)) == 0);
}
template <typename T>
NODISCARD inline CONSTEXPR CONSTF Meta::TEnableIf<std::is_unsigned_v<T>, bool> IsPow2(T u) {
    return ((u & (u - 1)) == 0 && u);
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD inline CONSTEXPR CONSTF Meta::TEnableIf<std::is_unsigned_v<T>, T> IsPow2(T value, T mod) {
    Assert_NoAssume(IsPow2(mod));
    return (value & (mod - 1u));
}
//----------------------------------------------------------------------------
template <size_t _Pow>
NODISCARD inline CONSTEXPR CONSTF bool IsPowOf(size_t u) {
    STATIC_ASSERT(IsPow2(_Pow) && _Pow > 2);
    for (; u >= _Pow; u /= _Pow);
    return (u == 1);
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
NODISCARD inline CONSTEXPR CONSTF bool IsAlignedPow2(const size_t alignment, const uintptr_t v) {
    Assert(Meta::IsPow2(alignment));
    return (0 == (v & (alignment - 1)));
}
template <typename T>
NODISCARD inline CONSTF bool IsAlignedPow2(const size_t alignment, const T* ptr) NOEXCEPT {
    Assert(Meta::IsPow2(alignment));
    return (0 == (std::bit_cast<uintptr_t>(ptr) & (alignment - 1)));
}
template <typename T, class = std::enable_if_t<std::is_integral_v<T>> >
NODISCARD inline CONSTEXPR T RoundToNextPow2(const T v, TDontDeduce<T> alignment) {
    Assert(Meta::IsPow2(alignment));
    return ((0 == v) ? 0 : (v + alignment - static_cast<T>(1)) & ~(alignment - static_cast<T>(1)));
}
template <typename T, class = std::enable_if_t<std::is_integral_v<T>> >
NODISCARD inline CONSTEXPR T RoundToPrevPow2(const T v, TDontDeduce<T> alignment) {
    Assert(Meta::IsPow2(alignment));
    return ((0 == v) ? 0 : v & ~(alignment - static_cast<T>(1)));
}
template <typename T>
NODISCARD inline T* RoundToNextPow2(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToNextPow2(std::bit_cast<uintptr_t>(p), alignment));
}
template <typename T>
NODISCARD inline T* RoundToPrevPow2(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToPrevPow2(std::bit_cast<uintptr_t>(p), alignment));
}
//----------------------------------------------------------------------------
// works for every alignment value
NODISCARD inline CONSTEXPR CONSTF bool IsAligned(size_t alignment, uintptr_t value) {
    Assume(alignment > 0);
    return (value % alignment == 0);
}
template <typename T>
NODISCARD inline CONSTF bool IsAligned(size_t alignment, T* ptr) NOEXCEPT {
    return IsAligned(alignment, std::bit_cast<uintptr_t>(ptr));
}
template <typename T>
NODISCARD inline CONSTEXPR T RoundToNext(T integral, std::enable_if_t<std::is_integral_v<T>, T> alignment) {
    Assume(alignment > 0);
    Assume(integral >= 0);
    return (((integral + alignment - 1) / alignment) * alignment);
}
template <typename T>
NODISCARD inline CONSTEXPR T RoundToPrev(T integral, std::enable_if_t<std::is_integral_v<T>, T> alignment) {
    Assume(alignment > 0);
    return ((integral / alignment) * alignment);
}
template <typename T>
NODISCARD inline T* RoundToNext(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToNext(std::bit_cast<uintptr_t>(p), alignment));
}
template <typename T>
NODISCARD inline T* RoundToPrev(const T* p, size_t alignment) NOEXCEPT {
    return reinterpret_cast<T*>(RoundToPrev(std::bit_cast<uintptr_t>(p), alignment));
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
#   define ROUND_TO_NEXT_CACHELINE(v) ::PPE::Meta::RoundToNextPow2((v), CACHELINE_SIZE)
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
    STATIC_ASSERT(IsAlignedPow2(ALLOCATION_BOUNDARY, _Alignment));
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
