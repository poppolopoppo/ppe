#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/BuddyAllocator.h"
#include "Core/Allocator/HeapAllocator.h"
#include "Core/Allocator/Mallocator.h"
#include "Core/Allocator/New.h"
#include "Core/Allocator/PoolAllocator.h"
#include "Core/Allocator/StackAllocator.h"
#include "Core/Allocator/ThreadLocalAllocator.h"
#include "Core/Allocator/TrackingAllocator.h"
#include "Core/Memory/MemoryDomain.h"

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
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
#define CLASS_MEMORY_TRACKING_DEF(T, _Domain) \
    static Core::MemoryTrackingData& Class_TrackingData() { \
        return MEMORY_DOMAIN_TRACKING_DATA(_Domain); \
    } \
    \
    void* operator new(size_t size) { \
        Assert(sizeof(T) == size); \
        Class_TrackingData().Allocate(1, sizeof(T)); \
        return Core::malloc(size); \
    } \
    void operator delete(void* ptr) { \
        Class_TrackingData().Deallocate(1, sizeof(T)); \
        Core::free(ptr); \
    } \
    \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        __assume(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define CLASS_MEMORY_TRACKING_DEF(T, _Domain) \
    static MemoryTrackingData& Class_TrackingData() { \
        return MEMORY_DOMAIN_TRACKING_DATA(_Domain); \
    }
//----------------------------------------------------------------------------
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
