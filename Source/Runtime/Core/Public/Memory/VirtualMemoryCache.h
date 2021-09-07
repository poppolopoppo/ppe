#pragma once

#include "Core_fwd.h"

#include "Memory/VirtualMemory.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"

#include <mutex>

#if USE_PPE_MEMORYDOMAINS
#   define VIRTUALMEMORYCACHE(_DOMAIN, _NUM_BLOCKS, _MAX_SIZE_IN_BYTES) \
    PPE::TVirtualMemoryCache<_NUM_BLOCKS, _MAX_SIZE_IN_BYTES, MEMORYDOMAIN_TAG(_DOMAIN)>
#else
#   define VIRTUALMEMORYCACHE(_DOMAIN, _NUM_BLOCKS, _MAX_SIZE_IN_BYTES) \
    PPE::TVirtualMemoryCache<_NUM_BLOCKS, _MAX_SIZE_IN_BYTES>
#endif

namespace PPE {
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
    ~FVirtualMemoryCache();

    FVirtualMemoryCache(const FVirtualMemoryCache&) = delete;
    FVirtualMemoryCache& operator =(const FVirtualMemoryCache&) = delete;

    std::mutex Barrier;
    size_t FreePageBlockCount;
    size_t TotalCacheSizeInBytes;

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#endif

    size_t RegionSize(void* ptr) const { return FVirtualMemory::SizeInBytes(ptr); }
    void* Allocate(size_t sizeInBytes, FFreePageBlock* first, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP);
    void Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP);
    void ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP);

#undef TRACKINGDATA_ARG_IFP
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <size_t _CacheBlocksCapacity, size_t _MaxCacheSizeInBytes, typename _DomainType >
#else
template <size_t _CacheBlocksCapacity, size_t _MaxCacheSizeInBytes>
#endif
class TVirtualMemoryCache : private FVirtualMemoryCache {
public:
    TVirtualMemoryCache() { STATIC_ASSERT(_MaxCacheSizeInBytes > 0 && Meta::IsAligned(64 * 1024, _MaxCacheSizeInBytes)); }
    ~TVirtualMemoryCache() { ReleaseAll(); }

#if USE_PPE_MEMORYDOMAINS
    typedef _DomainType domain_type;
    FMemoryTracking& TrackingData() const { return domain_type::TrackingData(); }
#   define TRACKINGDATA_ARG_IFP , TrackingData()
#else
#   define TRACKINGDATA_ARG_IFP
#endif

    size_t RegionSize(void* ptr) const { return FVirtualMemoryCache::RegionSize(ptr); }
    FORCE_INLINE void* Allocate(size_t sizeInBytes) { return FVirtualMemoryCache::Allocate(sizeInBytes, _freePageBlocks, _MaxCacheSizeInBytes TRACKINGDATA_ARG_IFP); }
    void Free(void* ptr, size_t sizeInBytes = 0) { FVirtualMemoryCache::Free(ptr, sizeInBytes, _freePageBlocks, _CacheBlocksCapacity, _MaxCacheSizeInBytes TRACKINGDATA_ARG_IFP); }
    void ReleaseAll() {  FVirtualMemoryCache::ReleaseAll(_freePageBlocks TRACKINGDATA_ARG_IFP); }

#undef TRACKINGDATA_ARG_IFP

private:
    FFreePageBlock _freePageBlocks[_CacheBlocksCapacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
