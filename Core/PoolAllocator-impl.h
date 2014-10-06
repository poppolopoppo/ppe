#pragma once

#include "PoolAllocator.h"

#ifdef WITH_CORE_POOL_ALLOCATOR

#include "AutoSingleton.h"
#include "MemoryPool.h"
#include "ThreadLocalAllocator.h"

#ifdef ARCH_X64
#   define POOL_CHUNKSIZE (32ul << 10ul) /* 32k */
#else
#   define POOL_CHUNKSIZE (16ul << 10ul) /* 16k  */
#endif

#ifdef _DEBUG
#   define WITH_CORE_POOL_ALLOCATOR_VERBOSE
#   include "Logger.h"
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
    struct SeggregatedPoolVerbose {
        SeggregatedPoolVerbose() {
            LOG(Information, L"[SEGPOOL] Starting seggregated pool for <{0}> and tag <{1}> ({2} block size{3})",
                typeid(_Type).name(), typeid(_Tag).name(), sizeof(_Type),
                _ThreadLocal ? L", thread local" : L"");
        }
        ~SeggregatedPoolVerbose() {}
    };
#   define CORE_POOL_ALLOCATOR_VERBOSE(_Tag, _Type, _ThreadLocal) \
    static const Core::SeggregatedPoolVerbose<_Tag, _Type, _ThreadLocal> sPoolVerbose
#else
#   define CORE_POOL_ALLOCATOR_VERBOSE(_Tag, _Type, _ThreadLocal) NOOP
#endif //!WITH_CORE_POOL_ALLOCATOR_VERBOSE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DefaultPoolSeggreationTag {};
//----------------------------------------------------------------------------
template <typename _Tag, size_t _BlockSize, typename _Allocator = ALLOCATOR(Pool, u64) >
class SeggregatedPool :
    public MemoryPool<true, _Allocator>
,   Meta::AutoSingleton<SeggregatedPool<_Tag, _BlockSize, _Allocator> > {
public:
    typedef SeggregatedPool<_Tag, _BlockSize, _Allocator> self_type;
    typedef MemoryPool<true, _Allocator> memorypool_type;
    typedef Meta::AutoSingleton<self_type> singleton_type;

    friend class Meta::AutoSingleton<SeggregatedPool<_Tag, _BlockSize, _Allocator> >;

    enum : size_t {
        ThisBlockSize = _BlockSize,
        ThisChunksize = POOL_CHUNKSIZE
    };

    using singleton_type::Instance;

private:
    explicit SeggregatedPool()
        : memorypool_type(ThisBlockSize, ThisChunksize)
        , singleton_type(this) {}
};
//----------------------------------------------------------------------------
template <typename _Tag, typename T, typename _Allocator = ALLOCATOR(Pool, u64) >
class TypedSeggregatedPool {
public:
    TypedSeggregatedPool() = delete;
    ~TypedSeggregatedPool() = delete;

    typedef SeggregatedPool<_Tag, SNAP_SIZE_FOR_POOL_SEGREGATION(T), typename _Allocator::template rebind<u64>::other>
            seggregatedpool_type;

#pragma warning( push )
#pragma warning( disable: 4503 )
    static seggregatedpool_type& Instance() {
        CORE_POOL_ALLOCATOR_VERBOSE(_Tag, T, false);
        return seggregatedpool_type::Instance();
    }
#pragma warning( pop )
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, size_t _BlockSize, typename _Allocator = THREAD_LOCAL_ALLOCATOR(Pool, u64) >
class ThreadLocalSeggregatedPool :
    public MemoryPool<false, _Allocator>
,   Meta::AutoSingletonThreadLocal<ThreadLocalSeggregatedPool<_Tag, _BlockSize, _Allocator> > {
public:
    typedef ThreadLocalSeggregatedPool<_Tag, _BlockSize, _Allocator> self_type;
    typedef MemoryPool<false, _Allocator> memorypool_type;
    typedef Meta::AutoSingletonThreadLocal<self_type> singleton_type;

    friend class Meta::AutoSingletonThreadLocal<ThreadLocalSeggregatedPool<_Tag, _BlockSize, _Allocator> >;

    enum : size_t {
        ThisBlockSize = _BlockSize,
        ThisChunksize = POOL_CHUNKSIZE
    };

    using singleton_type::Instance;

private:
    explicit ThreadLocalSeggregatedPool()
        : memorypool_type(ThisBlockSize, ThisChunksize)
        , singleton_type(this) {}
};
//----------------------------------------------------------------------------
template <typename _Tag, typename T, typename _Allocator = THREAD_LOCAL_ALLOCATOR(Pool, u64) >
class TypedThreadLocalSeggregatedPool {
public:
    TypedThreadLocalSeggregatedPool() = delete;
    ~TypedThreadLocalSeggregatedPool() = delete;

    typedef ThreadLocalSeggregatedPool<_Tag, SNAP_SIZE_FOR_POOL_SEGREGATION(T), typename _Allocator::template rebind<u64>::other>
            seggregatedpool_type;

#pragma warning( push )
#pragma warning( disable: 4503 )
    static seggregatedpool_type& Instance() {
        CORE_POOL_ALLOCATOR_VERBOSE(_Tag, T, true);
        return seggregatedpool_type::Instance();
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
// _Tag enables user to control seggregation
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedSeggregatedPool<_Tag COMMA _Type>)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(_Tag, _Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_DEF_IMPL_(_Type, _Prefix, Core::TypedThreadLocalSeggregatedPool<_Tag COMMA _Type>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// DefaultPoolSeggreationTag regroups all default pool allocated instances
//----------------------------------------------------------------------------
#define SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Core::DefaultPoolSeggreationTag, _Type, _Prefix)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_DEF(_Type, _Prefix) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Core::DefaultPoolSeggreationTag, _Type, _Prefix)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
