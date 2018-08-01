#pragma once

#include "Core.h"

#include "Allocator/PoolAllocatorTag.h"
#include "Memory/MemoryDomain.h"

#if not USE_PPE_MEMORY_DEBUGGING
#   define WITH_PPE_POOL_ALLOCATOR
#endif

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_PPE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL() \
public: \
    void* operator new(size_t size); \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr); \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {} \
    \
    static void Pool_ReleaseMemory(); \
    static const FMemoryTracking *Pool_TrackingData()
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL() \
public: \
    void* operator new(size_t size); \
    void operator delete(void* ptr); \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {} \
    static void Pool_ReleaseMemory(); \
    static const FMemoryTracking *Pool_TrackingData()
//----------------------------------------------------------------------------
#endif //!WITH_PPE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
