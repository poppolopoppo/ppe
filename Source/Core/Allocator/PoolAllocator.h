#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/MemoryDomain.h"

#if not USE_CORE_MEMORY_DEBUGGING
#   define WITH_CORE_POOL_ALLOCATOR
#endif

namespace Core {
class FMemoryTrackingData;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL() \
public: \
    void* operator new(size_t size); \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        Likely(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr); \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {} \
    \
    static void Pool_ReleaseMemory(); \
    static const FMemoryTrackingData *Pool_TrackingData()
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL() \
public: \
    static void Pool_ReleaseMemory(); \
    static const FMemoryTrackingData *Pool_TrackingData()
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
