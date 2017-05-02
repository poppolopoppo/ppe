#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualMemory {
public:
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

    void* Allocate(size_t sizeInBytes, FFreePageBlock* first);
    void Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes);
    void ReleaseAll(FFreePageBlock* first);
};
//----------------------------------------------------------------------------
template <size_t _CacheBlocksCapacity, size_t _MaxCacheSizeInBytes>
class TVirtualMemoryCache : private FVirtualMemoryCache {
public:
    TVirtualMemoryCache() { STATIC_ASSERT(_MaxCacheSizeInBytes > 0 && IS_ALIGNED(64 * 1024, _MaxCacheSizeInBytes)); }
    ~TVirtualMemoryCache() { ReleaseAll(); }

    FORCE_INLINE void* Allocate(size_t sizeInBytes) { return FVirtualMemoryCache::Allocate(sizeInBytes, _freePageBlocks); }
    void Free(void* ptr, size_t sizeInBytes) { FVirtualMemoryCache::Free(ptr, sizeInBytes, _freePageBlocks, _CacheBlocksCapacity, _MaxCacheSizeInBytes); }
    void ReleaseAll() { FVirtualMemoryCache::ReleaseAll(_freePageBlocks); }

private:
    FFreePageBlock _freePageBlocks[_CacheBlocksCapacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
