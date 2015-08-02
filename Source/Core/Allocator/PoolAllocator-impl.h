#pragma once

#include "Core/Allocator/PoolAllocator.h"

#ifdef WITH_CORE_POOL_ALLOCATOR
#include "Core/Memory/SegregatedMemoryPool.h"
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, _Pool) \
    _Prefix void *_Type::operator new(size_t size) { \
        Assert(sizeof(_Type) == size); \
        return _Pool::Allocate(); \
    } \
    _Prefix void _Type::operator delete(void *ptr) { \
        if (ptr) _Pool::Deallocate(ptr); \
    } \
    _Prefix void _Type::Pool_ReleaseMemory() { \
        _Pool::Clear_UnusedMemory(); \
    } \
    _Prefix const MemoryTrackingData *_Type::Pool_TrackingData() { \
        return _Pool::TrackingData(); \
    }
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, _Pool) \
    _Prefix void _Type::Pool_ReleaseMemory() {} \
    _Prefix const MemoryTrackingData *_Type::Pool_TrackingData() { return nullptr; }
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// _Tag enables user to control segregation
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix), Core::TypedSegregatedMemoryPool<POOLTAG(_Tag) COMMA COMMA_PROTECT(_Type) COMMA false>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix), Core::TypedSegregatedMemoryPool<POOLTAG(_Tag) COMMA COMMA_PROTECT(_Type) COMMA true>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// PoolTag::Default regroups all default pool allocated instances
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Default, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix))
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Default, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
