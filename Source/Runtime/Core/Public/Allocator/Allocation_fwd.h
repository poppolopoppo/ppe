#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryDomain_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Forward declaration of various allocators
//----------------------------------------------------------------------------
template <size_t _BlockSize>
class TBitmapAllocator;
template <typename _Cascade, typename _Batch>
class TCascadedAllocator;
template <typename _Primary, typename _Fallback>
class TFallbackAllocator;
template <size_t _SizeInBytes>
class TInSituAllocator;
template <size_t _SizeInBytes>
class TInSituStackAllocator;
class FMallocator;
template <typename _Allocator, size_t _MinSize>
class TMinSizeAllocator;
template <size_t _Threshold, typename _Under, typename _Above>
class TSegregateAllocator;
template <typename _Allocator>
class TSlabHeap;
template <typename _Allocator>
class TPoolingSlabHeap;
template <typename _Allocator>
class TSlabAllocator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Default allocator definition
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Domain, typename _Allocator>
class TTrackingAllocator;
template <typename _Tag, typename _Allocator>
using TDecorateAllocator = TTrackingAllocator< _Tag, _Allocator >;
#else
template <typename _Tag, typename _Allocator>
using TDecorateAllocator = _Allocator;
#endif //!USE_PPE_MEMORYDOMAINS
//----------------------------------------------------------------------------
template <typename _Tag>
using TDefaultAllocator = TDecorateAllocator<_Tag, FMallocator>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses cascaded bitmap allocators for small allocations, or uses malloc()
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, typename T>
using TBatchAllocator = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, typename T>
using TBatchAllocator = TCascadedAllocator<
    TBitmapAllocator<sizeof(T)>,
    TDefaultAllocator<_Tag>
>;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
// Uses an in situ only for one allocation of _Sz bytes, then uses malloc()
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, size_t _Sz>
using TRawInlineAllocator = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, size_t _Sz>
using TRawInlineAllocator = TSegregateAllocator<
    _Sz,
    TInSituAllocator<_Sz>,
    TDefaultAllocator<_Tag> >;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
// Uses an in situ only for one allocation of N blocks, then uses malloc()
//----------------------------------------------------------------------------
template <typename _Tag, typename T, size_t N>
using TInlineAllocator = TRawInlineAllocator<_Tag, sizeof(T) * N>;
//----------------------------------------------------------------------------
// Uses an in situ stack up to N blocks, then uses malloc()
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, typename T, size_t N>
using TInlineStackAllocator = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, typename T, size_t N>
using TInlineStackAllocator = TFallbackAllocator<
    TInSituStackAllocator<sizeof(T) * N>,
    TDefaultAllocator<_Tag>
>;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
// Uses malloc() but allocations are snapped to be superior to N
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, typename T, size_t N>
using TDefaultAllocatorMinSize = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, typename T, size_t N>
using TDefaultAllocatorMinSize = TMinSizeAllocator<
    TDefaultAllocator<_Tag>,
    sizeof(T) * N >;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
// Uses only an in situ storage, no other fallback (can't grow)
//----------------------------------------------------------------------------
template <typename T, size_t N>
using TFixedSizeAllocator = TInSituAllocator<sizeof(T) * N>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain) \
    ::PPE::TDefaultAllocator< MEMORYDOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
#define ALLOCATOR_MINSIZE(_Domain, T, N) \
    ::PPE::TDefaultAllocatorMinSize< MEMORYDOMAIN_TAG(_Domain), T, N >
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATOR(_Domain, _Alignment) \
    ::PPE::TDecorateAllocator< MEMORYDOMAIN_TAG(_Domain), ::PPE::TAlignedMallocator<_Alignment> >
//----------------------------------------------------------------------------
#define BATCH_ALLOCATOR(_Domain, T) \
    ::PPE::TBatchAllocator< MEMORYDOMAIN_TAG(_Domain), T >
//----------------------------------------------------------------------------
#define INLINE_ALLOCATOR(_Domain, T, N) \
    ::PPE::TInlineAllocator< MEMORYDOMAIN_TAG(_Domain), T, N >
//----------------------------------------------------------------------------
#define INLINE_STACK_ALLOCATOR(_Domain, T, N) \
    ::PPE::TInlineStackAllocator< MEMORYDOMAIN_TAG(_Domain), T, N >
//----------------------------------------------------------------------------
#define STACKLOCAL_ALLOCATOR() \
    ::PPE::FStackLocalAllocator// don't decorate to avoid double logging with "Alloca" domain
//----------------------------------------------------------------------------
#define FIXEDSIZE_ALLOCATOR(T, N) \
    ::PPE::TFixedSizeAllocator< T, N >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
