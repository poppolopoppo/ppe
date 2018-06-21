#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryPool.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/AutoSingleton.h"
#include "Core/Meta/OneTimeInitialize.h"

#if USE_CORE_MEMORYDOMAINS
#include "Core/IO/Format.h"
#include "Core/IO/TextWriter.h"
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

#define USE_CORE_POOL_ALLOCATOR_SNAPPING (1)//%__NOCOMMIT%

#if USE_CORE_MEMORYDOMAINS && defined(WITH_CORE_POOL_ALLACATOR_TAGNAME)
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
        Format(Name, "{0}_{1}<{2}>", MakeCStringView(tagname), _ThreadLocal ? "TLS" : "TSF", _Size);
        RegisterTrackingData(&TrackingData);
    }

    ~TPoolTracking() {
        UnregisterTrackingData(&TrackingData);
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

    using singleton_type::Get;

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
#if USE_CORE_POOL_ALLOCATOR_SNAPPING
namespace details {
inline constexpr size_t MakePoolSizeClass(size_t snapped, size_t log2) {
	constexpr size_t POW_N = 2;
	constexpr size_t MinClassIndex = 19;
	return ((log2 << POW_N) + ((snapped - 1) >> (log2 - POW_N)) - MinClassIndex);
}
constexpr size_t GClassesSize[45] = {
	16,     0,      0,      0,      32,     0,
	48,     0,      64,     80,     96,     112,
	128,    160,    192,    224,    256,    320,
	384,    448,    512,    640,    768,    896,
	1024,   1280,   1536,   1792,   2048,   2560,
	3072,   3584,   4096,   5120,   6144,   7168,
	8192,   10240,  12288,  14336,  16384,  20480,
	24576,  28672,  32736,
};
inline constexpr size_t FloorLog2_constexpr(size_t n) {
	return ((n < 2) ? 0 : 1 + FloorLog2_constexpr(n / 2));
}
} //!details
inline constexpr size_t MemoryPoolSnapSize(size_t size) {
	return details::GClassesSize[details::MakePoolSizeClass(
		ROUND_TO_NEXT_16(size),
		details::FloorLog2_constexpr((ROUND_TO_NEXT_16(size) - 1) | 1) )];
}
#else
inline constexpr size_t MemoryPoolSnapSize(size_t size) { return size; }
#endif //!USE_CORE_POOL_ALLOCATOR_SNAPPING
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

#if defined(WITH_CORE_POOL_ALLOCATOR_TRACKING_DETAILS)
    static FMemoryTracking* TrackingData() {
        ONE_TIME_INITIALIZE_TPL(TPoolTracking<T COMMA _ThreadLocal>, sPoolTracking,
            typeid(T).name(), segregatedpool_type::Get().TrackingData());
        return &sPoolTracking.TrackingData;
    }
#elif defined(WITH_CORE_POOL_ALLOCATOR_TRACKING)
    static FMemoryTracking* TrackingData() { return segregatedpool_type::Get().TrackingData(); }
#else
    static constexpr FMemoryTracking* TrackingData() { return nullptr; }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
