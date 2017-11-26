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
#   define POOL_MAX_CHUNKSIZE (2048ul << 10ul) /*2048k */
#   define POOL_MIN_CHUNKSIZE (  64ul << 10ul) /*  64k */
#else
#   define POOL_MAX_CHUNKSIZE (1024ul << 10ul) /*1024k */
#   define POOL_MIN_CHUNKSIZE (  64ul << 10ul) /*  64k */
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
        ? ROUND_TO_NEXT_32(sizeof(_Type)) \
        : ROUND_TO_NEXT_16(sizeof(_Type)) )
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
struct TPoolTracking {
    typedef TPoolTracking<T, _ThreadLocal, _Size> self_type;

    Core::FMemoryTracking TrackingData;
    char Name[256];

    TPoolTracking(const char* tagname, FMemoryTracking* parent = nullptr)
    :   TrackingData(&Name[0], parent) {
        Format(Name, "{0}<{1},{2}>", tagname, _ThreadLocal, _Size);
        RegisterAdditionalTrackingData(&TrackingData);
    }

    ~TPoolTracking() {
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
,           typename _MemoryPool
,           template <class > class _AutoSingleton = Meta::TAutoSingleton >
class TSegregatedMemoryPool :
    private _MemoryPool
,   private _AutoSingleton<TSegregatedMemoryPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> > {
public:
    typedef TSegregatedMemoryPool<_Tag, _BlockSize, _MemoryPool, _AutoSingleton> self_type;
    typedef _AutoSingleton<self_type> singleton_type;
    typedef _MemoryPool memorypool_type;

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
    TPoolTracking<_Tag, std::is_same<FMemoryPool, _MemoryPool>::value, _BlockSize> _poolTracking;
public:
    FMemoryTracking* TrackingData() { return &_poolTracking.TrackingData; }
#else
    FMemoryTracking* TrackingData() const { return nullptr; }
#endif

public:
    TSegregatedMemoryPool()
    :   memorypool_type(PoolBlockSize, PoolMinChunkSize, PoolMaxChunkSize)
#ifdef WITH_CORE_POOL_ALLOCATOR_TRACKING
    ,   _poolTracking(_Tag::Name())
#endif
    {
        _Tag::Register(this);
    }

    ~TSegregatedMemoryPool() {
        _Tag::Unregister(this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
class TTypedSegregatedMemoryPool {
public:
    typedef TTypedSegregatedMemoryPool<_Tag, T, _ThreadLocal> self_type;

    TTypedSegregatedMemoryPool() = delete;
    ~TTypedSegregatedMemoryPool() = delete;

    enum : size_t { BlockSize = SNAP_SIZE_FOR_POOL_SEGREGATION(T) };

    typedef typename std::conditional<_ThreadLocal,
        TSegregatedMemoryPool<_Tag, BlockSize, FMemoryPoolThreadLocal, Meta::TThreadLocalAutoSingleton >,
        TSegregatedMemoryPool<_Tag, BlockSize, FMemoryPoolThreadSafe, Meta::TAutoSingleton >
    >::type     segregatedpool_type;

    FORCE_INLINE static void* Allocate() { return segregatedpool_type::Instance().Allocate(TrackingData()); }
    FORCE_INLINE static void Deallocate(void* ptr) { segregatedpool_type::Instance().Deallocate(ptr, TrackingData()); }
    FORCE_INLINE static void Clear_UnusedMemory() { segregatedpool_type::Instance().Clear_UnusedMemory(); }

#if defined(WITH_CORE_POOL_ALLOCATOR_TRACKING_DETAILS)
    static FMemoryTracking* TrackingData() {
        ONE_TIME_INITIALIZE_TPL(TPoolTracking<T COMMA _ThreadLocal>, sPoolTracking,
            typeid(T).name(), segregatedpool_type::Instance().TrackingData());
        return &sPoolTracking.TrackingData;
    }
#elif defined(WITH_CORE_POOL_ALLOCATOR_TRACKING)
    static FMemoryTracking* TrackingData() { return segregatedpool_type::Instance().TrackingData(); }
#else
    static constexpr FMemoryTracking* TrackingData() { return nullptr; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
