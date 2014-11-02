#pragma once

#include "Core/Allocator/PoolAllocator.h"

#ifdef WITH_CORE_POOL_ALLOCATOR

#include "Core/Allocator/ThreadLocalAllocator.h"
#include "Core/Memory/MemoryPool.h"
#include "Core/Meta/AutoSingleton.h"
#include "Core/Meta/OneTimeInitialize.h"

#ifdef ARCH_X64
#   define POOL_CHUNKSIZE (32ul << 10ul) /* 32k */
#else
#   define POOL_CHUNKSIZE (16ul << 10ul) /* 16k  */
#endif

#ifdef _DEBUG
#   define WITH_CORE_POOL_ALLOCATOR_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#   include <typeinfo>
#endif

#define WITH_CORE_POOL_ALLOCATOR_SNAPPING

#ifdef WITH_CORE_POOL_ALLOCATOR_SNAPPING
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) \
    (sizeof(_Type) >= 128 \
        ? ((sizeof(_Type)) + 32 & ~31) \
        : (sizeof(_Type) >= 16 \
            ? ((sizeof(_Type)) + 15 & ~15) \
            : ((sizeof(_Type)) + 7 & ~7)) )
#else
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) sizeof(_Type)
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR_VERBOSE
    template <typename _Tag, typename _Type, bool _ThreadLocal>
    struct SegregatedPoolVerbose {
        SegregatedPoolVerbose() {
            LOG(Information, L"[SEGPOOL] Starting segregated pool for <{0}> and tag <{1}> ({2} block size{3})",
                typeid(_Type).name(), typeid(_Tag).name(), sizeof(_Type),
                _ThreadLocal ? L", thread local" : L"");
        }
        ~SegregatedPoolVerbose() {}
    };
#   define CORE_POOL_ALLOCATOR_VERBOSE(_Tag, _Type, _ThreadLocal) \
    ONE_TIME_DEFAULT_INITIALIZE(Core::SegregatedPoolVerbose<_Tag COMMA _Type COMMA _ThreadLocal>, sPoolVerbose)
#else
#   define CORE_POOL_ALLOCATOR_VERBOSE(_Tag, _Type, _ThreadLocal) NOOP
#endif //!WITH_CORE_POOL_ALLOCATOR_VERBOSE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DefaultPoolSegregationTag {};
//----------------------------------------------------------------------------
template <typename _Tag, size_t _BlockSize, typename _Allocator = ALLOCATOR(Pool, u64) >
class SegregatedPool :
    public MemoryPool<true, _Allocator>
,   Meta::AutoSingleton<SegregatedPool<_Tag, _BlockSize, _Allocator> > {
public:
    typedef SegregatedPool<_Tag, _BlockSize, _Allocator> self_type;
    typedef MemoryPool<true, _Allocator> memorypool_type;
    typedef Meta::AutoSingleton<self_type> singleton_type;

    friend class Meta::AutoSingleton<SegregatedPool<_Tag, _BlockSize, _Allocator> >;

    enum : size_t {
        ThisBlockSize = _BlockSize,
        ThisChunksize = POOL_CHUNKSIZE
    };

    using singleton_type::Instance;

private:
    explicit SegregatedPool()
        : memorypool_type(ThisBlockSize, ThisChunksize)
        , singleton_type(this) {}
};
//----------------------------------------------------------------------------
template <typename _Tag, typename T, typename _Allocator = ALLOCATOR(Pool, u64) >
class TypedSegregatedPool {
public:
    TypedSegregatedPool() = delete;
    ~TypedSegregatedPool() = delete;

    typedef SegregatedPool<_Tag, SNAP_SIZE_FOR_POOL_SEGREGATION(T), typename _Allocator::template rebind<u64>::other>
            segregatedpool_type;

#pragma warning( push )
#pragma warning( disable: 4503 )
#pragma warning( disable: 4640 )
    static segregatedpool_type& Instance() {
        CORE_POOL_ALLOCATOR_VERBOSE(_Tag, T, false);
        return segregatedpool_type::Instance();
    }
#pragma warning( pop )
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, size_t _BlockSize, typename _Allocator = THREAD_LOCAL_ALLOCATOR(Pool, u64) >
class ThreadLocalSegregatedPool :
    public MemoryPool<false, _Allocator>
,   Meta::AutoSingletonThreadLocal<ThreadLocalSegregatedPool<_Tag, _BlockSize, _Allocator> > {
public:
    typedef ThreadLocalSegregatedPool<_Tag, _BlockSize, _Allocator> self_type;
    typedef MemoryPool<false, _Allocator> memorypool_type;
    typedef Meta::AutoSingletonThreadLocal<self_type> singleton_type;

    friend class Meta::AutoSingletonThreadLocal<ThreadLocalSegregatedPool<_Tag, _BlockSize, _Allocator> >;

    enum : size_t {
        ThisBlockSize = _BlockSize,
        ThisChunksize = POOL_CHUNKSIZE
    };

    using singleton_type::Instance;

private:
    explicit ThreadLocalSegregatedPool()
        : memorypool_type(ThisBlockSize, ThisChunksize)
        , singleton_type(this) {}
};
//----------------------------------------------------------------------------
template <typename _Tag, typename T, typename _Allocator = THREAD_LOCAL_ALLOCATOR(Pool, u64) >
class TypedThreadLocalSegregatedPool {
public:
    TypedThreadLocalSegregatedPool() = delete;
    ~TypedThreadLocalSegregatedPool() = delete;

    typedef ThreadLocalSegregatedPool<_Tag, SNAP_SIZE_FOR_POOL_SEGREGATION(T), typename _Allocator::template rebind<u64>::other>
            segregatedpool_type;

#pragma warning( push )
#pragma warning( disable: 4503 )
    static segregatedpool_type& Instance() {
        CORE_POOL_ALLOCATOR_VERBOSE(_Tag, T, true);
        return segregatedpool_type::Instance();
    }
#pragma warning( pop )
};
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
        _Pool::Instance().Clear_UnusedMemory(); \
    } \
    _Prefix NO_INLINE void *_Type::operator new(size_t size) { \
        Assert(sizeof(_Type) == size); \
        return _Pool::Instance().Allocate(); \
    } \
    _Prefix NO_INLINE void _Type::operator delete(void *ptr) { \
        if (ptr) _Pool::Instance().Deallocate(ptr); \
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
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedSegregatedPool<_Tag COMMA _Type>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedThreadLocalSegregatedPool<_Tag COMMA _Type>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// DefaultPoolSegregationTag regroups all default pool allocated instances
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Core::DefaultPoolSegregationTag, _Type, _Prefix)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Core::DefaultPoolSegregationTag, _Type, _Prefix)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
