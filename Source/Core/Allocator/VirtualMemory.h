#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define VIRTUALMEMORYCACHE(_DOMAIN, _NUM_BLOCKS, _MAX_SIZE_IN_BYTES) \
    Core::TVirtualMemoryCache<_NUM_BLOCKS, _MAX_SIZE_IN_BYTES, MEMORY_DOMAIN_TAG(_DOMAIN)>
#else
#   define VIRTUALMEMORYCACHE(_DOMAIN, _NUM_BLOCKS, _MAX_SIZE_IN_BYTES) \
    Core::TVirtualMemoryCache<_NUM_BLOCKS, _MAX_SIZE_IN_BYTES>
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualMemory {
public:
    static size_t AllocSizeInBytes(void* ptr);
    static void* AlignedAlloc(size_t alignment, size_t sizeInBytes);
    static void  AlignedFree(void* ptr, size_t sizeInBytes);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualMemoryCache {
protected:
    struct FFreePageBlock {
        void* Ptr = nullptr;
        size_t SizeInBytes = 0;
    };

    FVirtualMemoryCache();

    FVirtualMemoryCache(const FVirtualMemoryCache&) = delete;
    FVirtualMemoryCache& operator =(const FVirtualMemoryCache&) = delete;

    size_t FreePageBlockCount;
    size_t TotalCacheSizeInBytes;

#ifdef USE_MEMORY_DOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTrackingData& trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#endif

    size_t RegionSize(void* ptr) const { return FVirtualMemory::AllocSizeInBytes(ptr); }
    void* Allocate(size_t sizeInBytes, FFreePageBlock* first TRACKINGDATA_ARG_IFP);
    void Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP);
    void ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP);

#undef TRACKINGDATA_ARG_IFP
};
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
template <size_t _CacheBlocksCapacity, size_t _MaxCacheSizeInBytes, typename _DomainType >
#else
template <size_t _CacheBlocksCapacity, size_t _MaxCacheSizeInBytes>
#endif
class TVirtualMemoryCache : private FVirtualMemoryCache {
public:
    TVirtualMemoryCache() { STATIC_ASSERT(_MaxCacheSizeInBytes > 0 && Meta::IsAligned(64 * 1024, _MaxCacheSizeInBytes)); }
    ~TVirtualMemoryCache() { ReleaseAll(); }

#ifdef USE_MEMORY_DOMAINS
    typedef _DomainType domain_type;
    FMemoryTrackingData& TrackingData() const { return domain_type::TrackingData; }
#   define TRACKINGDATA_ARG_IFP , TrackingData()
#else
#   define TRACKINGDATA_ARG_IFP
#endif

    size_t RegionSize(void* ptr) const { return FVirtualMemoryCache::RegionSize(ptr); }
    FORCE_INLINE void* Allocate(size_t sizeInBytes) { return FVirtualMemoryCache::Allocate(sizeInBytes, _freePageBlocks TRACKINGDATA_ARG_IFP); }
    void Free(void* ptr, size_t sizeInBytes = 0) { FVirtualMemoryCache::Free(ptr, sizeInBytes, _freePageBlocks, _CacheBlocksCapacity, _MaxCacheSizeInBytes TRACKINGDATA_ARG_IFP); }
    void ReleaseAll() {  FVirtualMemoryCache::ReleaseAll(_freePageBlocks TRACKINGDATA_ARG_IFP); }

#undef TRACKINGDATA_ARG_IFP

private:
    FFreePageBlock _freePageBlocks[_CacheBlocksCapacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
