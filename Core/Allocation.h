#pragma once

#include "Core.h"

#include "Alloca.h"
#include "BuddyAllocator.h"
#include "HeapAllocator.h"
#include "Mallocator.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "ThreadLocalAllocator.h"
#include "TrackingAllocator.h"
#include "MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEFAULT_ALLOCATOR ::Core::Mallocator
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
template <typename _Allocator, typename _Tag>
using DecorateAllocator = TagTrackingAllocator< _Allocator, _Tag >;
#else
template <typename _Allocator, typename _Tag>
using DecorateAllocator = _Allocator;
#endif
//----------------------------------------------------------------------------
template <typename T, typename _Tag = MEMORY_DOMAIN_TAG(Global)>
using Allocator = DecorateAllocator< DEFAULT_ALLOCATOR<T>, _Tag >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain, T) \
    ::Core::Allocator<T, MEMORY_DOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
#define DECORATE_ALLOCATOR(_Domain, _Allocator) \
    ::Core::DecorateAllocator< _Allocator, MEMORY_DOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATOR(_Domain, T, _Alignment) \
    DECORATE_ALLOCATOR(_Domain, ::Core::Mallocator<T COMMA _Alignment>)
//----------------------------------------------------------------------------
#define BUDDY_ALLOCATOR(_Domain, T) \
    DECORATE_ALLOCATOR(_Domain, ::Core::BuddyAllocator<T>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_ALLOCATOR(_Domain, T) \
    DECORATE_ALLOCATOR(_Domain, ::Core::ThreadLocalAllocator<T>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
