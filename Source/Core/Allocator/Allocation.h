#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/Mallocator.h"
#include "Core/Allocator/ThreadLocalAllocator.h"
#include "Core/Allocator/TrackingAllocator.h"
#include "Core/Memory/MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEFAULT_ALLOCATOR ::Core::TMallocator
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = TTrackingAllocator< _Tag, _Allocator >;
#else
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = _Allocator;
#endif
//----------------------------------------------------------------------------
template <typename T, typename _Tag = MEMORY_DOMAIN_TAG(Global)>
using TAllocator = TDecorateAllocator< DEFAULT_ALLOCATOR<T>, _Tag >;
//----------------------------------------------------------------------------
#define DECORATE_ALLOCATOR(_Domain, _Allocator) \
    ::Core::TDecorateAllocator< COMMA_PROTECT(_Allocator), MEMORY_DOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain, T) \
    ::Core::TAllocator<COMMA_PROTECT(T), MEMORY_DOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATOR(_Domain, T, _Alignment) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TMallocator<COMMA_PROTECT(T) COMMA _Alignment>)
//----------------------------------------------------------------------------
#define BUDDY_ALLOCATOR(_Domain, T) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TBuddyAllocator<COMMA_PROTECT(T)>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_ALLOCATOR(_Domain, T) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TThreadLocalAllocator<COMMA_PROTECT(T)>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
