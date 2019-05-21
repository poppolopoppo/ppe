#pragma once

#include "Core_fwd.h"

#include "Allocator/Alloca.h"
#include "Allocator/AllocatorHelpers.h"
#include "Allocator/BitmapAllocator.h"
#include "Allocator/CascadedAllocator.h"
#include "Allocator/InSituAllocator.h"
#include "Allocator/Mallocator.h"
#include "Allocator/New.h"
#include "Allocator/StackLocalAllocator.h"
#include "Allocator/TrackingAllocator.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
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
// Uses an in situ only one allocation of N blocks, then uses malloc()
//----------------------------------------------------------------------------
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Tag, typename T, size_t N>
using TInlineAllocator = TDefaultAllocator<_Tag>;
#else
template <typename _Tag, typename T, size_t N>
using TInlineAllocator = TSegregatorAllocator<
    sizeof(T) * N,
    TInSituAllocator<sizeof(T) * N>,
    TDefaultAllocator<_Tag> >;
#endif //!USE_PPE_MEMORY_DEBUGGING
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain) \
    ::PPE::TDefaultAllocator< MEMORYDOMAIN_TAG(_Domain) >
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
#define STACKLOCAL_ALLOCATOR() \
    ::PPE::FStackLocalAllocator// don't decorate to avoid double logging with "Alloca" domain
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
