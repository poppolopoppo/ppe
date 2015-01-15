#pragma once

#include "Core/Allocator/PoolAllocator.h"

#ifdef WITH_CORE_POOL_ALLOCATOR

#include "Core/Allocator/ThreadLocalAllocator.h"
#include "Core/IO/Format.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryPool.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Meta/AutoSingleton.h"
#include "Core/Meta/OneTimeInitialize.h"

#include <type_traits>

#ifdef ARCH_X64
#   define POOL_BASE_CHUNKSIZE (32ul << 10ul) /* 32k */
#else
#   define POOL_BASE_CHUNKSIZE (16ul << 10ul) /* 16k  */
#endif

#ifdef _DEBUG
#   define WITH_CORE_POOL_ALLOCATOR_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#   include <typeinfo>
#endif

#define WITH_CORE_POOL_ALLOCATOR_SNAPPING

#ifdef WITH_CORE_POOL_ALLOCATOR_SNAPPING
/*#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) \
    (sizeof(_Type) >= 128 \
        ? ((sizeof(_Type)) + 32 & ~31) \
        : (sizeof(_Type) >= 16 \
            ? ((sizeof(_Type)) + 15 & ~15) \
            : ((sizeof(_Type)) + 7 & ~7)) )*/
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) \
    (sizeof(_Type) >= 16 \
        ? ((sizeof(_Type)) + 15 & ~15) \
        : ((sizeof(_Type)) + 7 & ~7) )
#else
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) sizeof(_Type)
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
// Outside of Core:: to reduce typename signature length
template <typename T, bool _ThreadLocal, size_t _Size = sizeof(T) >
struct PoolTracking {
    typedef PoolTracking<T, _ThreadLocal, _Size> self_type;

    char Name[64];
    Core::MemoryTrackingData TrackingData;

    PoolTracking(   const char *tagname,
                    Core::MemoryTrackingData *parent = nullptr,
                    Core::MemoryTrackingData **dst = nullptr )
    :   TrackingData(Name, parent) {
        Format(Name, "{0}<{1},{2}>", tagname, _ThreadLocal, _Size);
        Core::RegisterAdditionalTrackingData(TrackingData);
        if (dst) *dst = &TrackingData;
    }
};
//----------------------------------------------------------------------------
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <  typename _Tag, size_t _BlockSize
,           typename _MemoryPool = MemoryPool<true>
,           template <class > class _AutoSingleton = Meta::AutoSingleton >
class SegregatedPool :
    public _MemoryPool
,   public _AutoSingleton<SegregatedPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> > {
public:
    typedef SegregatedPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> self_type;
    typedef _MemoryPool memorypool_type;
    typedef _AutoSingleton<self_type> singleton_type;

    friend class singleton_type;

    enum : size_t {
        PoolBlockSize = _BlockSize,
        PoolChunkSize = POOL_BASE_CHUNKSIZE
    };

    using singleton_type::Instance;

#ifdef USE_MEMORY_DOMAINS
private:
    PoolTracking<_Tag, !memorypool_type::IsLocked, _BlockSize> _poolTracking;
public:
    MemoryTrackingData *TrackingData() { return &_poolTracking.TrackingData; }
#else
    MemoryTrackingData *TrackingData() const { return nullptr; }
#endif

private:
    SegregatedPool()
    :   memorypool_type(PoolBlockSize, PoolChunkSize)
    ,   singleton_type(this)
#ifdef USE_MEMORY_DOMAINS
    ,   _poolTracking(_Tag::Name())
#endif
    {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
class TypedSegregatedPool {
public:
    typedef TypedSegregatedPool<_Tag, T, _ThreadLocal> self_type;

    TypedSegregatedPool() = delete;
    ~TypedSegregatedPool() = delete;

    enum : size_t { BlockSize = SNAP_SIZE_FOR_POOL_SEGREGATION(T) };

    typedef typename std::conditional<_ThreadLocal,
        SegregatedPool<_Tag, BlockSize, MemoryPool<false, THREAD_LOCAL_ALLOCATOR(Pool, size_t)>, Meta::AutoSingletonThreadLocal >,
        SegregatedPool<_Tag, BlockSize, MemoryPool<true, ALLOCATOR(Pool, size_t)>, Meta::AutoSingleton >
    >::type     segregatedpool_type;

    static void *Allocate();
    static void Deallocate(void *ptr);
    static void Clear_UnusedMemory();

#ifdef USE_MEMORY_DOMAINS
    static MemoryTrackingData *TrackingData;
#endif
};
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
template <typename _Tag, typename T, bool _ThreadLocal >
MemoryTrackingData *TypedSegregatedPool<_Tag, T, _ThreadLocal>::TrackingData = nullptr;
#endif
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void *TypedSegregatedPool<_Tag, T, _ThreadLocal>::Allocate() {
#ifdef USE_MEMORY_DOMAINS
    ONE_TIME_INITIALIZE(PoolTracking<T COMMA _ThreadLocal>, sTracking, T::Pool_Name(), segregatedpool_type::Instance().TrackingData(), &TrackingData);
    return segregatedpool_type::Instance().Allocate(&sTracking.TrackingData);
#else
    return segregatedpool_type::Instance().Allocate();
#endif
}
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void TypedSegregatedPool<_Tag, T, _ThreadLocal>::Deallocate(void *ptr) {
#ifdef USE_MEMORY_DOMAINS
    segregatedpool_type::Instance().Deallocate(ptr, TrackingData);
#else
    segregatedpool_type::Instance().Deallocate(ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void TypedSegregatedPool<_Tag, T, _ThreadLocal>::Clear_UnusedMemory() {
    segregatedpool_type::Instance().Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!WITH_CORE_POOL_ALLOCATOR

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, _Pool) \
    _Prefix NO_INLINE void _Type::Pool_ReleaseMemory() { \
        _Pool::Clear_UnusedMemory(); \
    } \
    _Prefix NO_INLINE void *_Type::operator new(size_t size) { \
        Assert(sizeof(_Type) == size); \
        return _Pool::Allocate(); \
    } \
    _Prefix NO_INLINE void _Type::operator delete(void *ptr) { \
        if (ptr) _Pool::Deallocate(ptr); \
    }
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, _Pool) \
    _Prefix NO_INLINE void _Type::Pool_ReleaseMemory() {}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// _Tag enables user to control segregation
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedSegregatedPool<_Tag COMMA _Type COMMA false>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedSegregatedPool<_Tag COMMA _Type COMMA true>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// PoolTag::Default regroups all default pool allocated instances
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_TAGGED_DEF(POOLTAG(Default), _Type, _Prefix)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(POOLTAG(Default), _Type, _Prefix)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
