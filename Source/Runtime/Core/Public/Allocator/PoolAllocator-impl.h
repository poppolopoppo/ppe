#pragma once

#include "Allocator/PoolAllocator.h"

#ifdef WITH_PPE_POOL_ALLOCATOR
#include "Memory/SegregatedMemoryPool.h"
#else
#include "Allocator/TrackingMalloc.h"
#include "Memory/MemoryTracking.h"
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_PPE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Tag, _Type, _Prefix, _Pool) \
    _Prefix void *_Type::operator new(size_t size) { \
        NOOP(size); \
        Assert(sizeof(_Type) == size); \
        return _Pool::Allocate(); \
    } \
    _Prefix void _Type::operator delete(void *ptr) { \
        if (ptr) _Pool::Deallocate(ptr); \
    } \
    _Prefix void _Type::Pool_ReleaseMemory() { \
        _Pool::Clear_UnusedMemory(); \
    } \
    _Prefix const FMemoryTracking *_Type::Pool_TrackingData() { \
        return _Pool::TrackingData(); \
    }
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Tag, _Type, _Prefix, _Pool) \
    _Prefix void *_Type::operator new(size_t ) { return tracking_malloc(MEMORYDOMAIN_TRACKING_DATA(PooledMemory), sizeof(_Type)); } \
    _Prefix void _Type::operator delete(void *ptr) { tracking_free(ptr); } \
    _Prefix void _Type::Pool_ReleaseMemory() {} \
    _Prefix const FMemoryTracking *_Type::Pool_TrackingData() { return nullptr; }
//----------------------------------------------------------------------------
#endif //!WITH_PPE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// _Tag enables user to control segregation
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Tag, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix), \
        PPE::TTypedSegregatedMemoryPool<POOL_TAG(_Tag) COMMA COMMA_PROTECT(_Type) COMMA false>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Tag, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix), \
        PPE::TTypedSegregatedMemoryPool<POOL_TAG(_Tag) COMMA COMMA_PROTECT(_Type) COMMA true>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// PoolTag::Default regroups all default pool allocated instances
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Default, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix))
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Default, COMMA_PROTECT(_Type), COMMA_PROTECT(_Prefix))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
