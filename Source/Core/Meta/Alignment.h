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
#define IS_ALIGNED(_BOUNDARY, _POINTER) (0 == (size_t(_POINTER) % (_BOUNDARY)) )
//----------------------------------------------------------------------------
#define ALIGN(_BOUNDARY) __declspec(align(_BOUNDARY))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CACHELINE_SIZE (64)
//----------------------------------------------------------------------------
#define CACHELINE_ALIGNED ALIGN(CACHELINE_SIZE)
//----------------------------------------------------------------------------
#define ROUND_TO_NEXT_CACHELINE(v) ROUND_TO_NEXT_64(v)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PAGE_SIZE (4096)
//----------------------------------------------------------------------------
#define PAGE_ALIGNED ALIGN(PAGE_SIZE)
//----------------------------------------------------------------------------
#define HUGE_PAGE_SIZE (65536)
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
template <size_t _Alignment>
struct TIsCacheLineAligned {
    // Power of 2 assertion guarantees <= test correctness :
    // if aligned on 8, also aligned on 4, 2 & 1
    static_assert(_Alignment && 0 == (_Alignment & (_Alignment - 1)), "_Alignment must be a power of 2");
    enum { value = (_Alignment == (CACHELINE_SIZE)) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _Defined = Meta::has_destructor<T>::value >
struct TAlignmentOfIFP {
    enum : size_t { value = std::alignment_of<T>::value };
};
//----------------------------------------------------------------------------
template <typename T>
struct TAlignmentOfIFP<T, false> {
    enum : size_t { value = alignof(intptr_t) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
