#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/MemoryDomain.h"

#define WITH_CORE_POOL_ALLOCATOR

namespace Core {
class MemoryTrackingData;
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
        __assume(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr); \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {} \
    \
    static void Pool_ReleaseMemory(); \
    static const MemoryTrackingData *Pool_TrackingData()
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL() \
public: \
    static void Pool_ReleaseMemory(); \
    static const MemoryTrackingData *Pool_TrackingData()
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
