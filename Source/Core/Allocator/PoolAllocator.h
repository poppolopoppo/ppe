#pragma once

#include "Core/Core.h"

#define WITH_CORE_POOL_ALLOCATOR

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define POOLTAG_DEF(_Name) namespace PoolTag { \
    struct _Name { static const char *Name() { return STRINGIZE(_Name); } }; \
    }
//----------------------------------------------------------------------------
#define POOLTAG(_Name) Core::PoolTag::_Name
//----------------------------------------------------------------------------
POOLTAG_DEF(Default) // Default tag for Pool segregation
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define POOL_ALLOCATED_TYPE_DEBUG(_Type) \
    static const char *Pool_Name() { return #_Type; }
#else
#   define POOL_ALLOCATED_TYPE_DEBUG(_Type) \
    static const char *Pool_Name() { return nullptr; }
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DECL(T) \
public: \
    POOL_ALLOCATED_TYPE_DEBUG(T) \
    \
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
    POOL_ALLOCATED_TYPE_DEBUG(T) \
    \
    static void Pool_ReleaseMemory()
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
