#pragma once

#include "Core.h"

#include "HAL/PlatformAtomics.h"
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
class PPE_CORE_API FVirtualMemory {
public:
    static size_t   SizeInBytes(void* ptr);
    static bool     Protect(void* ptr, size_t sizeInBytes, bool read, bool write);

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_FWD_IFP , trackingData
#   define TRACKINGDATA_PRM_IFP(...) , MEMORYDOMAIN_TRACKING_DATA(__VA_ARGS__)
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_FWD_IFP
#   define TRACKINGDATA_PRM_IFP(...)
#endif

    static void*    Alloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP) { return Alloc(ALLOCATION_GRANULARITY, sizeInBytes TRACKINGDATA_FWD_IFP); }
    static void*    Alloc(size_t alignment, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     Free(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);

    static void*    PageReserve(size_t sizeInBytes);
    static void*    PageReserve(size_t alignment, size_t sizeInBytes);
    static void     PageCommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     PageDecommit(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     PageRelease(void* ptr, size_t sizeInBytes);

    static void*    InternalAlloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP);
    static void     InternalFree(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP);

#undef TRACKINGDATA_ARG_IFP
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
template <typename T>
struct TVirtualMemoryPool {
    STATIC_CONST_INTEGRAL(u32, PageSize, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(u32, BlockSize, sizeof(T));
    STATIC_CONST_INTEGRAL(u32, NumBlocksPerPage, PageSize / BlockSize);
    STATIC_ASSERT(NumBlocksPerPage* BlockSize == PageSize);

    struct ALIGN(BlockSize) FBlock {
        FBlock* NextBlock;
    };
    struct ALIGN(BlockSize) FPage {
        FPage* NextPage;
        FBlock* FreeBlocks;
        u32 NumFreeBlocks;
        FORCE_INLINE void ResetFreeBlocks() {
            FreeBlocks = nullptr;
            NumFreeBlocks = 1;
        }
    };

    std::mutex Barrier;
    FPage* Pages{ nullptr };
    std::atomic<FBlock*> FreeList{ nullptr };
#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData;
#endif

#if USE_PPE_MEMORYDOMAINS
    explicit TVirtualMemoryPool(FMemoryTracking& trackingData) NOEXCEPT
    :   TrackingData(trackingData)
    {}
#endif

    ~TVirtualMemoryPool() NOEXCEPT {
        ReleaseCacheMemory(); // release all pages, supposedly
        AssertRelease(nullptr == FreeList); // leaking ?
    }

    void* Allocate() NOEXCEPT {
        FBlock* blk = FreeList.load(std::memory_order_relaxed);
        while (Likely(blk)) {
            FBlock* nxt = blk->NextBlock;
            if (FreeList.compare_exchange_weak(blk, nxt, std::memory_order_release, std::memory_order_relaxed)) {
                ONLY_IF_MEMORYDOMAINS(TrackingData.AllocateUser(BlockSize));
                return blk;
            }

            FPlatformAtomics::ShortSyncWait();
        }

        return AllocateExternal();
    }

    void Deallocate(void* ptr) NOEXCEPT {
        Assert(ptr);

        ONLY_IF_MEMORYDOMAINS(TrackingData.DeallocateUser(BlockSize));

        FBlock* blk = reinterpret_cast<FBlock*>(ptr);
        blk->NextBlock = FreeList.load(std::memory_order_relaxed);
        for (;;) {
            if (FreeList.compare_exchange_weak(blk->NextBlock, blk, std::memory_order_release, std::memory_order_relaxed))
                return;

            FPlatformAtomics::ShortSyncWait();
        }
    }

    FORCE_INLINE static FPage* PageFromPtr(void* ptr) NOEXCEPT {
        Assert(ptr);
        return reinterpret_cast<FPage*>(Meta::RoundToPrev(ptr, PageSize));
    }

    void* AllocateExternal() NOEXCEPT;
    void ReleaseCacheMemory() NOEXCEPT;

};
//----------------------------------------------------------------------------
template <typename T>
void* TVirtualMemoryPool<T>::AllocateExternal() NOEXCEPT {
    Meta::FUniqueLock scopeLock(Barrier);

    // test again once locked, another thread might have already allocated a new page
    FBlock* blk = FreeList.load(std::memory_order_relaxed);
    if (blk) {
        FreeList = blk->NextBlock;
        return blk;
    }

    // look for a recycled block list if page list
    for (FPage* page = Pages; page; page = page->NextPage) {
        if (page->FreeBlocks) {
            Assert_NoAssume(nullptr == FreeList);
            blk = page->FreeBlocks;
            page->ResetFreeBlocks();
            FreeList = blk->NextBlock;
            return blk;
        }
    }

    // finally allocate a new page if everything else failed
    FPage* const page = reinterpret_cast<FPage*>(FVirtualMemory::InternalAlloc(PageSize
#if USE_PPE_MEMORYDOMAINS
        , TrackingData
#endif
    ));
    AssertRelease(page);
    Assert_NoAssume(Meta::IsAligned(PageSize, page));

    page->ResetFreeBlocks();
    page->NextPage = Pages;
    Pages = page;

    // prime all new blocks once while holding the lock
    FBlock* Head = reinterpret_cast<FBlock*>(page + 1);
    Assert_NoAssume(PageFromPtr(Head) == page);
    forrange(i, 0, NumBlocksPerPage - 2)
        Head[i].NextBlock = (Head + i + 1);

    // take the first block and append the rest to free list
    Head[NumBlocksPerPage - 2].NextBlock = FreeList.load(std::memory_order_relaxed);
    FreeList = Head->NextBlock;

    ONLY_IF_MEMORYDOMAINS(TrackingData.AllocateUser(BlockSize));

    return Head;
}
//----------------------------------------------------------------------------
template <typename T>
void TVirtualMemoryPool<T>::ReleaseCacheMemory() NOEXCEPT {
    const Meta::FLockGuard scopeLock(Barrier);

    // first check if there's actually something to salvage
    if (not Pages || not Pages->NextPage)
        return;

    // detach the free list (other thread can still pop blocks)
    FBlock* freeList = FreeList.load(std::memory_order_relaxed);
    while (freeList) {
        if (FreeList.compare_exchange_weak(freeList, nullptr, std::memory_order_release, std::memory_order_relaxed))
            break;

        FPlatformAtomics::ShortSyncWait();
    }

    if (nullptr == freeList)
        return; // *all* blocks are allocated

    // dispatch every free block to belonging page
    for (FBlock* blk = freeList; blk; ) {
        FBlock* const nxt = blk->NextBlock;

        FPage* const page = PageFromPtr(blk);
        ++page->NumFreeBlocks;
        Assert_NoAssume(page->NumFreeBlocks <= NumBlocksPerPage);
        blk->NextBlock = page->FreeBlocks;
        page->FreeBlocks = blk;

        blk = nxt;
    }

    // then release pages that are completely free
    FPage* remaining = nullptr;
    for (FPage* page = Pages, *prev = nullptr; page; ) {
        FPage* const nxt = page->NextPage;

        if (page->NumFreeBlocks == NumBlocksPerPage) {
            Assert(page->FreeBlocks);
            if (prev) // remove from page list
                prev->NextPage = nxt;
            else {
                Assert_NoAssume(page == Pages);
                Pages = nxt;
            }

            // release the block to the system
            FVirtualMemory::InternalFree(page, PageSize
#if USE_PPE_MEMORYDOMAINS
                , TrackingData
#endif
            );
        }
        else {
            remaining = page; // keep track of the last page which wasn't freed
            prev = page;
        }

        page = nxt;
    }

    // assign a new free list if any page remains
    if (remaining) {
        Assert(Pages);
        Assert(remaining->FreeBlocks);
        FreeList = remaining->FreeBlocks;
        remaining->ResetFreeBlocks();
    }
    else {
        FreeList = nullptr; // reset global free list head
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
