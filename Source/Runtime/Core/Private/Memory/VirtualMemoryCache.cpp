// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Memory/VirtualMemoryCache.h"

#if (USE_PPE_ASSERT || USE_PPE_MEMORY_DEBUGGING)
#   define USE_VMCACHE_PAGE_PROTECT     1// Crash when using a VM cached block
#else
#   define USE_VMCACHE_PAGE_PROTECT     0
#endif

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualMemoryCache::FVirtualMemoryCache()
    : FreePageBlockCount(0)
    , TotalCacheSizeInBytes(0)
{}
//----------------------------------------------------------------------------
FVirtualMemoryCache::~FVirtualMemoryCache() {
    Assert_NoAssume(0 == FreePageBlockCount);
    Assert_NoAssume(0 == TotalCacheSizeInBytes);
}
//----------------------------------------------------------------------------
void* FVirtualMemoryCache::Allocate(size_t sizeInBytes, FFreePageBlock* first, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    const size_t alignment = FPlatformMemory::AllocationGranularity;
    Assert(Meta::IsAlignedPow2(alignment, sizeInBytes));

    if ((sizeInBytes <= maxCacheSizeInBytes / 4) && FreePageBlockCount) {
        Meta::FUniqueLock scopeLock(Barrier);

        if (FreePageBlockCount) { // check again with the lock this time
            Assume(first);

            FFreePageBlock* cachedBlock = nullptr;

            FFreePageBlock* const last = first + FreePageBlockCount;
            for (FFreePageBlock* block = first; block != last; ++block) {
                // look for exact matches first, these are aligned to the page size, so it should be quite common to hit these on small pages sizes
                if (block->SizeInBytes == sizeInBytes) {
                    cachedBlock = block;
                    break;
                }
            }

#if 0 // better release the chunks which are too large, since the client can't use the extra memory
            if (nullptr == cachedBlock) {
                const size_t sizeTimes4 = sizeInBytes * 4;

                for (FFreePageBlock* block = first; block != last; ++block) {
                    // is it possible (and worth i.e. <25% overhead) to use this block
                    if (block->SizeInBytes >= sizeInBytes && block->SizeInBytes * 3 <= sizeTimes4) {
                        cachedBlock = block;
                        break;
                    }
                }
            }
#endif

            if (nullptr != cachedBlock) {
                void* const result = cachedBlock->Ptr;
                const size_t cachedBlockSize = cachedBlock->SizeInBytes;
                Assert(nullptr != result);

                FreePageBlockCount--;
                TotalCacheSizeInBytes -= cachedBlockSize;

                if (cachedBlock + 1 != last)
                    FPlatformMemory::Memmove(cachedBlock, cachedBlock + 1, sizeof(FFreePageBlock) * (last - cachedBlock - 1));

                Assert_NoAssume(Meta::IsAlignedPow2(alignment, result));

#if USE_VMCACHE_PAGE_PROTECT
                // Restore read+write access to pages, so it won't crash in the client
                FVirtualMemory::Protect(result, cachedBlockSize, true, true);
#endif

                return result;
            }

            // unlock from here on to diminish thread contention and avoid potential recursive lock in ReleaseAll()
            scopeLock.unlock();

            if (void* result = FVirtualMemory::Alloc(alignment, sizeInBytes TRACKINGDATA_ARG_FWD)) {
                Assert(Meta::IsAlignedPow2(alignment, result));
                return result;
            }

            // Are we holding on too much mem? Release it all.
            ReleaseAll(first TRACKINGDATA_ARG_FWD);
        }
    }

    // allocate new blocks outside the barrier to diminish thread contention

    void* result = FVirtualMemory::Alloc(alignment, sizeInBytes TRACKINGDATA_ARG_FWD);
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, result));

    ONLY_IF_ASSERT(FPlatformMemory::Memset(result, 0xCC, sizeInBytes)); // initialize the memory block before yielding

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);

    if (0 == sizeInBytes)
        sizeInBytes = FVirtualMemory::SizeInBytes(ptr);

    Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, sizeInBytes));

    // free the block without locking to diminish thread contention
    if (sizeInBytes > maxCacheSizeInBytes / 4 ||
        FreePageBlockCount + 1 > cacheBlocksCapacity ||
        TotalCacheSizeInBytes + sizeInBytes > maxCacheSizeInBytes ) {
        FVirtualMemory::Free(ptr, sizeInBytes TRACKINGDATA_ARG_FWD);
        return;
    }

    const Meta::FLockGuard scopeLock(Barrier);

    while (FreePageBlockCount >= cacheBlocksCapacity || TotalCacheSizeInBytes + sizeInBytes > maxCacheSizeInBytes) {
        Assert(FreePageBlockCount);

        FVirtualMemory::Free(first->Ptr, first->SizeInBytes TRACKINGDATA_ARG_FWD);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#if USE_PPE_DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif

        if (--FreePageBlockCount)
            FPlatformMemory::Memmove(first, first + 1, sizeof(FFreePageBlock) * FreePageBlockCount);
    }

    ONLY_IF_ASSERT(FPlatformMemory::Memset(ptr, 0xDD, sizeInBytes)); // trash the memory block before caching

    first[FreePageBlockCount] = FFreePageBlock{ ptr, sizeInBytes };
    TotalCacheSizeInBytes += sizeInBytes;
    FreePageBlockCount++;

#if USE_VMCACHE_PAGE_PROTECT
    // Forbid access to cached pages, so it will crash in the client it there's necrophilia attempt (read+write)
    FVirtualMemory::Protect(ptr, sizeInBytes, false, false);
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP) {
    const Meta::FLockGuard scopeLock(Barrier);

    for (   FFreePageBlock* const last = (first + FreePageBlockCount);
            first != last;
            ++first ) {
        Assert(Meta::IsAlignedPow2(FPlatformMemory::AllocationGranularity, first->Ptr));

        FVirtualMemory::Free(first->Ptr, first->SizeInBytes TRACKINGDATA_ARG_FWD);

#if USE_PPE_DEBUG
        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif
    }

#if USE_PPE_DEBUG
    Assert(0 == TotalCacheSizeInBytes);
#else
    TotalCacheSizeInBytes = 0;
#endif
    FreePageBlockCount = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef TRACKINGDATA_ARG_IFP
#undef TRACKINGDATA_ARG_FWD
