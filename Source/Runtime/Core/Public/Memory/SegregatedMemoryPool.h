#pragma once

#include "Core.h"

#include "Allocator/MallocBinned.h" // SnapSizeConstexpr
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryPool.h"
#include "Memory/MemoryTracking.h"
#include "Memory/UniquePtr.h"
#include "Meta/AutoSingleton.h"
#include "Meta/OneTimeInitialize.h"

#if USE_PPE_MEMORYDOMAINS
#include "IO/Format.h"
#include "IO/TextWriter.h"
#endif

#include <type_traits>

#ifdef ARCH_X64
#   define POOL_MAX_CHUNKSIZE (2048ul << 10ul) /*2048k */
#   define POOL_MIN_CHUNKSIZE (  64ul << 10ul) /*  64k */
#else
#   define POOL_MAX_CHUNKSIZE (1024ul << 10ul) /*1024k */
#   define POOL_MIN_CHUNKSIZE (  64ul << 10ul) /*  64k */
#endif

#if USE_PPE_DEBUG
#   define WITH_PPE_POOL_ALLOCATOR_VERBOSE
#   include "Diagnostic/Logger.h"
#   include <typeinfo>
#endif

#define USE_PPE_POOL_ALLOCATOR_SNAPPING (1)//%__NOCOMMIT%

#if USE_PPE_MEMORYDOMAINS && defined(WITH_PPE_POOL_ALLACATOR_TAGNAME)
#   define WITH_PPE_POOL_ALLOCATOR_TRACKING //%__NOCOMMIT%
#endif

#if defined(WITH_PPE_POOL_ALLOCATOR_TRACKING)
//#   define WITH_PPE_POOL_ALLOCATOR_TRACKING_DETAILS //%__NOCOMMIT%
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_PPE_POOL_ALLOCATOR_TRACKING
//----------------------------------------------------------------------------
template <typename T, bool _ThreadLocal, size_t _Size = sizeof(T) >
struct TPoolTracking {
    typedef TPoolTracking<T, _ThreadLocal, _Size> self_type;

    PPE::FMemoryTracking TrackingData;
    char Name[256];

    TPoolTracking(const char* tagname, FMemoryTracking* parent = nullptr)
    :   TrackingData(&Name[0], parent) {
        Format(Name, "{0}_{1}<{2}>", MakeCStringView(tagname), _ThreadLocal ? "TLS" : "TSF", _Size);
        RegisterTrackingData(&TrackingData);
    }

    ~TPoolTracking() {
        UnregisterTrackingData(&TrackingData);
    }
};
//----------------------------------------------------------------------------
#endif //!WITH_PPE_POOL_ALLOCATOR_TRACKING
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

    using singleton_type::Get;

#ifdef WITH_PPE_POOL_ALLOCATOR_TRACKING
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
#ifdef WITH_PPE_POOL_ALLOCATOR_TRACKING
    ,   _poolTracking(_Tag::Name(), &MEMORYDOMAIN_TRACKING_DATA(PooledMemory))
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
#if USE_PPE_POOL_ALLOCATOR_SNAPPING
inline constexpr size_t MemoryPoolSnapSize(size_t size) {
    return FMallocBinned::SnapSizeConstexpr(size);
}
#else
inline constexpr size_t MemoryPoolSnapSize(size_t size) { return size; }
#endif //!USE_PPE_POOL_ALLOCATOR_SNAPPING
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
class TTypedSegregatedMemoryPool {
public:
    typedef TTypedSegregatedMemoryPool<_Tag, T, _ThreadLocal> self_type;

    TTypedSegregatedMemoryPool() = delete;
    ~TTypedSegregatedMemoryPool() = delete;

    static constexpr size_t BlockSize = MemoryPoolSnapSize(sizeof(T));
    STATIC_ASSERT(sizeof(T) <= BlockSize);

    typedef typename std::conditional<_ThreadLocal,
        TSegregatedMemoryPool<_Tag, BlockSize, FMemoryPoolThreadLocal, Meta::TThreadLocalAutoSingleton >,
        TSegregatedMemoryPool<_Tag, BlockSize, FMemoryPoolThreadSafe, Meta::TAutoSingleton >
    >::type     segregatedpool_type;

    FORCE_INLINE static void* Allocate() { return segregatedpool_type::Get().Allocate(TrackingData()); }
    FORCE_INLINE static void Deallocate(void* ptr) { segregatedpool_type::Get().Deallocate(ptr, TrackingData()); }
    FORCE_INLINE static void Clear_UnusedMemory() { segregatedpool_type::Get().Clear_UnusedMemory(); }

#if defined(WITH_PPE_POOL_ALLOCATOR_TRACKING_DETAILS)
    static FMemoryTracking* TrackingData() {
        ONE_TIME_INITIALIZE_TPL(TPoolTracking<T COMMA _ThreadLocal>, sPoolTracking,
            PPE_TYPEID_NAME(T), segregatedpool_type::Get().TrackingData());
        return &sPoolTracking.TrackingData;
    }
#elif defined(WITH_PPE_POOL_ALLOCATOR_TRACKING)
    static FMemoryTracking* TrackingData() { return segregatedpool_type::Get().TrackingData(); }
#else
    static constexpr FMemoryTracking* TrackingData() { return nullptr; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
