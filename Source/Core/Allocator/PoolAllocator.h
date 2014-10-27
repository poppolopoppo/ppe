#pragma once

#include "Core/Core.h"

#define WITH_CORE_POOL_ALLOCATOR

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL(T) \
public: \
    static void Pool_ReleaseMemory(); \
    \
    void* operator new(size_t size); \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        __assume(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr); \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL(T) \
public: \
    static void Pool_ReleaseMemory()
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
