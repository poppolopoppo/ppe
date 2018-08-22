#pragma once

#include "Meta/TypeTraits.h"

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
#define ALIGN(_BOUNDARY) __declspec(align(_BOUNDARY))
//----------------------------------------------------------------------------
inline constexpr bool IsPow2(size_t u) { return ((u & (u - 1)) == 0 && u); }
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
inline constexpr bool IsAligned(const size_t alignment, const uintptr_t v) {
    return (0 == (v & (alignment - 1)));
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
template <typename T>
inline constexpr bool IsAligned(const size_t alignment, const T* ptr) {
    return (0 == (uintptr_t(ptr) & (alignment - 1)));
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
inline constexpr size_t RoundToNext(const size_t v, size_t alignment) {
    return ((0 == v) ? 0 : (v + alignment - 1) & ~(alignment - 1));
}
template <typename T> T* RoundToNext(const T* p, size_t alignment) {
    return (T*)RoundToNext(size_t(p), alignment);
}
//----------------------------------------------------------------------------
// /!\ Assumes <alignment> is a power of 2
inline constexpr size_t RoundToPrev(const size_t v, size_t alignment) {
    return ((0 == v) ? 0 : v & ~(alignment - 1));
}
template <typename T> T* RoundToPrev(const T* p, size_t alignment) {
    return (T*)RoundToPrev(size_t(p), alignment);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PAGE_SIZE (4096)
//----------------------------------------------------------------------------
#define ALLOCATION_BOUNDARY (16)
//----------------------------------------------------------------------------
#define ALLOCATION_GRANULARITY (65536)
//----------------------------------------------------------------------------
#define CACHELINE_SIZE (64)
//----------------------------------------------------------------------------
#define CACHELINE_ALIGNED ALIGN(CACHELINE_SIZE)
//----------------------------------------------------------------------------
#define ROUND_TO_NEXT_CACHELINE(v) ROUND_TO_NEXT_64(v)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// By default std allocations are already aligned on intptr_t (pointer size).
// Aligned allocations induce overhead so this will prevent us from calling it systematically.
template <size_t _Alignment>
struct TIsNaturalyAligned {
    // Power of 2 assertion guarantees <= test correctness :
    // if aligned on 8, also aligned on 4, 2 & 1
    static_assert(_Alignment && 0 == (_Alignment & (_Alignment - 1)), "_Alignment must be a power of 2");
    enum { value = (_Alignment <= sizeof(intptr_t)) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
