#pragma once

#include "Core/Meta/TypeTraits.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ROUND_TO_NEXT_4(v)   ((((size_t)(v)) + 3) & ~3)
#define ROUND_TO_NEXT_8(v)   ((((size_t)(v)) + 7) & ~7)
#define ROUND_TO_NEXT_16(v)  ((((size_t)(v)) + 15) & ~15)
#define ROUND_TO_NEXT_32(v)  ((((size_t)(v)) + 31) & ~31)
#define ROUND_TO_NEXT_64(v)  ((((size_t)(v)) + 63) & ~63)
#define ROUND_TO_NEXT_128(v) ((((size_t)(v)) + 127) & ~127)
#define ROUND_TO_NEXT_64K(v) ((((size_t)(v)) + 64*1024-1) & ~(64*1024-1))
//----------------------------------------------------------------------------
#if defined(ARCH_X64)
#   define ROUND_TO_NEXT_SIZE_T(v) ROUND_TO_NEXT_8(v)
#elif defined(ARCH_X86)
#   define ROUND_TO_NEXT_SIZE_T(v) ROUND_TO_NEXT_4(v)
#else
#   error "no support"
#endif
//----------------------------------------------------------------------------
#define IS_POW2(v) ((v) >= 2) && (((v) & ((v) - 1)) == 0)
//----------------------------------------------------------------------------
#define IS_ALIGNED(_BOUNDARY, _POINTER) (0 == (size_t(_POINTER) % (_BOUNDARY)) )
//----------------------------------------------------------------------------
#define ALIGN(_BOUNDARY) __declspec(align(_BOUNDARY))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CACHELINE_SIZE 64
//----------------------------------------------------------------------------
#define CACHELINE_ALIGNED ALIGN(CACHELINE_SIZE)
//----------------------------------------------------------------------------
#define ROUND_TO_NEXT_CACHELINE(v) ROUND_TO_NEXT_64(v)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PAGE_SIZE 4096
//----------------------------------------------------------------------------
#define PAGE_ALIGNED ALIGN(PAGE_SIZE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// By default std allocations are already aligned on size_t (pointer size).
// Aligned allocations induce overhead (decuded it from beeing obliged to call _mm_free or aligned_free),
// so this will prevent us from calling it systematically.
// NB: enable_if is used with overloading instead of return type to avoid confusing the compiler with __declspec(restrict).
template <size_t _Alignment>
struct IsNaturalyAligned {
    // Power of 2 assertion guarantees <= test correctness :
    // if aligned on 8, also aligned on 4, 2 & 1
    static_assert(_Alignment && 0 == (_Alignment & (_Alignment - 1)), "_Alignment must be a power of 2");
    enum { value = (_Alignment <= sizeof(size_t)) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
struct IsCacheLineAligned {
    // Power of 2 assertion guarantees <= test correctness :
    // if aligned on 8, also aligned on 4, 2 & 1
    static_assert(_Alignment && 0 == (_Alignment & (_Alignment - 1)), "_Alignment must be a power of 2");
    enum { value = (_Alignment == (CACHELINE_SIZE)) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _Defined = Meta::has_destructor<T>::value >
struct AlignmentOfIFP {
    enum : size_t { value = std::alignment_of<T>::value };
};
//----------------------------------------------------------------------------
template <typename T>
struct AlignmentOfIFP<T, false> {
    enum : size_t { value = sizeof(size_t) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
