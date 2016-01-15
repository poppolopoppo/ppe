#pragma once

#include "Core/Core.h"

#include "Core/Allocator/ThreadLocalAllocator.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryPool.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/AutoSingleton.h"
#include "Core/Meta/OneTimeInitialize.h"

#ifdef USE_MEMORY_DOMAINS
#include "Core/IO/Format.h"
#endif

#include <type_traits>

#ifdef ARCH_X64
#   define POOL_MAX_CHUNKSIZE (128ul << 10ul) /* 128k */
#   define POOL_MIN_CHUNKSIZE (  8ul << 10ul) /*   8k */
#else
#   define POOL_MAX_CHUNKSIZE ( 64ul << 10ul) /*  64k */
#   define POOL_MIN_CHUNKSIZE (  4ul << 10ul) /*   4k */
#endif

#ifdef _DEBUG
#   define WITH_CORE_POOL_ALLOCATOR_VERBOSE
#   include "Core/Diagnostic/Logger.h"
#   include <typeinfo>
#endif

#define WITH_CORE_POOL_ALLOCATOR_SNAPPING //%__NOCOMMIT%
#ifdef WITH_CORE_POOL_ALLOCATOR_SNAPPING
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) \
    (sizeof(_Type) >= 128 \
        ? ((sizeof(_Type)) + 31 & ~31) \
        : (sizeof(_Type) >= 16 \
            ? ((sizeof(_Type)) + 15 & ~15) \
            : ((sizeof(_Type)) +  7 &  ~7)) )
#else
#   define SNAP_SIZE_FOR_POOL_SEGREGATION(_Type) sizeof(_Type)
#endif

#if defined(USE_MEMORY_DOMAINS) && defined(WITH_CORE_POOL_ALLACATOR_TAGNAME)
#   define WITH_CORE_POOL_ALLOCATOR_TRACKING //%__NOCOMMIT%
#endif

#if defined(WITH_CORE_POOL_ALLOCATOR_TRACKING)
//#   define WITH_CORE_POOL_ALLOCATOR_TRACKING_DETAILS //%__NOCOMMIT%
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLOCATOR_TRACKING
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal, size_t _Size = sizeof(T) >
struct PoolTracking {
    typedef PoolTracking<T, _ThreadLocal, _Size> self_type;

    Core::MemoryTrackingData TrackingData;
    char Name[128];

    PoolTracking(const char* tagname, MemoryTrackingData* parent = nullptr)
    :   TrackingData(&Name[0], parent) {
        Format(Name, "{0}<{1},{2}>", tagname, _ThreadLocal, _Size);
        RegisterAdditionalTrackingData(&TrackingData);
    }

    ~PoolTracking() {
        UnregisterAdditionalTrackingData(&TrackingData);
    }
};
//----------------------------------------------------------------------------
#endif //!WITH_CORE_POOL_ALLOCATOR_TRACKING
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <  typename _Tag
,           size_t _BlockSize
,           typename _MemoryPool = MemoryPool<true>
,           template <class > class _AutoSingleton = Meta::AutoSingleton >
class SegregatedMemoryPool :
    private _MemoryPool
,   private _AutoSingleton<SegregatedMemoryPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> > {
public:
    typedef SegregatedMemoryPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> self_type;
    typedef _AutoSingleton<self_type> singleton_type;
    typedef _MemoryPool memorypool_type;

    friend class singleton_type;

    enum : size_t {
        PoolBlockSize       = _BlockSize,
        PoolMinChunkSize    = POOL_MIN_CHUNKSIZE,
        PoolMaxChunkSize    = POOL_MAX_CHUNKSIZE
    };

    using memorypool_type::Allocate;
    using memorypool_type::Deallocate;
    using memorypool_type::Clear_UnusedMemory;

    using singleton_type::Instance;

#ifdef WITH_CORE_POOL_ALLOCATOR_TRACKING
private:
    PoolTracking<_Tag, (false == memorypool_type::IsLocked), _BlockSize> _poolTracking;
public:
    MemoryTrackingData* TrackingData() { return &_poolTracking.TrackingData; }
#else
    MemoryTrackingData* TrackingData() const { return nullptr; }
#endif

private:
    SegregatedMemoryPool()
    :   memorypool_type(PoolBlockSize, PoolMinChunkSize, PoolMaxChunkSize)
#ifdef WITH_CORE_POOL_ALLOCATOR_TRACKING
    ,   _poolTracking(_Tag::Name())
#endif
    {
        _Tag::Register(this);
    }

    ~SegregatedMemoryPool() {
        _Tag::Unregister(this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
class TypedSegregatedMemoryPool {
public:
    typedef TypedSegregatedMemoryPool<_Tag, T, _ThreadLocal> self_type;

    TypedSegregatedMemoryPool() = delete;
    ~TypedSegregatedMemoryPool() = delete;

    enum : size_t { BlockSize = SNAP_SIZE_FOR_POOL_SEGREGATION(T) };

    typedef typename std::conditional<_ThreadLocal,
        SegregatedMemoryPool<_Tag, BlockSize, MemoryPool<false, THREAD_LOCAL_ALLOCATOR(Pool, size_t) >, Meta::ThreadLocalAutoSingleton >,
        SegregatedMemoryPool<_Tag, BlockSize, MemoryPool<true , ALLOCATOR(Pool, size_t)              >, Meta::AutoSingleton >
    >::type     segregatedpool_type;

    FORCE_INLINE static void* Allocate() { return segregatedpool_type::Instance().Allocate(TrackingData()); }
    FORCE_INLINE static void Deallocate(void* ptr) { segregatedpool_type::Instance().Deallocate(ptr, TrackingData()); }
    FORCE_INLINE static void Clear_UnusedMemory() { segregatedpool_type::Instance().Clear_UnusedMemory(); }

#if defined(WITH_CORE_POOL_ALLOCATOR_TRACKING_DETAILS)
    static MemoryTrackingData* TrackingData() {
        ONE_TIME_INITIALIZE_TPL(PoolTracking<T COMMA _ThreadLocal>, sPoolTracking,
            typeid(T).name(), segregatedpool_type::Instance().TrackingData());
        return &sPoolTracking.TrackingData;
    }
#elif defined(WITH_CORE_POOL_ALLOCATOR_TRACKING)
    static MemoryTrackingData* TrackingData() { return segregatedpool_type::Instance().TrackingData(); }
#else
    static constexpr MemoryTrackingData* TrackingData() { return nullptr; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
