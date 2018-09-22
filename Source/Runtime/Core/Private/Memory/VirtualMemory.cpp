#include "stdafx.h"

#include "Memory/VirtualMemory.h"

#include "Container/CompressedRadixTrie.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#if USE_PPE_MEMORYDOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

#ifdef PLATFORM_WINDOWS
#   define USE_VMALLOC_SIZE_PTRIE          1// This is faster than ::VirtualQuery()
#else
#   define USE_VMALLOC_SIZE_PTRIE          1// No support on other platforms
#endif

#if (defined(WITH_PPE_ASSERT) || USE_PPE_MEMORY_DEBUGGING)
#   define USE_VMCACHE_PAGE_PROTECT     1// Crash when using a VM cached block
#else
#   define USE_VMCACHE_PAGE_PROTECT     0
#endif

#if USE_VMALLOC_SIZE_PTRIE
#   include "Thread/AtomicSpinLock.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_VMALLOC_SIZE_PTRIE
namespace {
//----------------------------------------------------------------------------
class FVMAllocSizePTrie_ {
public:
    static FVMAllocSizePTrie_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FVMAllocSizePTrie_, GInstance);
        return GInstance;
    }

    void Register(void* ptr, size_t sizeInBytes) {
        _allocs.Insert(uintptr_t(ptr), uintptr_t(sizeInBytes));
    }

    size_t Fetch(void* ptr) const {
        return size_t(_allocs.Lookup(uintptr_t(ptr)));
    }

    size_t Erase(void* ptr) {
        return size_t(_allocs.Erase(uintptr_t(ptr)));
    }

private:
    FCompressedRadixTrie _allocs;

#if USE_PPE_MEMORYDOMAINS
    FVMAllocSizePTrie_() : _allocs(MEMORYDOMAIN_TRACKING_DATA(SizePtrie)) {}
#else
    FVMAllocSizePTrie_() {}
#endif
    ~FVMAllocSizePTrie_() {}
};
//----------------------------------------------------------------------------
} //!namespace
#endif //!USE_VMALLOC_SIZE_PTRIE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FVirtualMemory::SizeInBytes(void* ptr) {
    if (nullptr == ptr)
        return 0;

    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));

#if USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = FVMAllocSizePTrie_::Get().Fetch(ptr);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, regionSize));

    return regionSize;

#else
    return FPlatformMemory::RegionSize(ptr);

#endif
}
//----------------------------------------------------------------------------
bool FVirtualMemory::Protect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

    return FPlatformMemory::PageProtect(ptr, sizeInBytes, read, write);
}
//----------------------------------------------------------------------------
// Keep allocations aligned to OS granularity
void* FVirtualMemory::Alloc(size_t alignment, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));
    Assert(Meta::IsPow2(alignment));

    void* const p = FPlatformMemory::VirtualAlloc(alignment, sizeInBytes);

#if USE_VMALLOC_SIZE_PTRIE
    FVMAllocSizePTrie_::Get().Register(p, sizeInBytes);
#endif
#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.Allocate(1, sizeInBytes);
#endif

    return p;
}
//----------------------------------------------------------------------------
void FVirtualMemory::Free(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert(sizeInBytes);

#if USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = FVMAllocSizePTrie_::Get().Erase(ptr);
    Assert(regionSize == sizeInBytes);
#endif

#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.Deallocate(1, sizeInBytes);
#endif

    FPlatformMemory::VirtualFree(ptr, sizeInBytes);
}
//----------------------------------------------------------------------------
// Won't register in FVMAllocSizePTrie_
void* FVirtualMemory::InternalAlloc(size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, sizeInBytes));

    void* const ptr = FPlatformMemory::VirtualAlloc(sizeInBytes);
    AssertRelease(ptr);

#if USE_PPE_MEMORYDOMAINS
    Assert(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.Allocate(1, sizeInBytes);
#endif

    return ptr;
}
//----------------------------------------------------------------------------
// Won't register in FVMAllocSizePTrie_
void FVirtualMemory::InternalFree(void* ptr, size_t sizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, sizeInBytes));

    FPlatformMemory::VirtualFree(ptr, sizeInBytes);

#if USE_PPE_MEMORYDOMAINS
    Assert(trackingData.IsChildOf(FMemoryTracking::ReservedMemory()));
    trackingData.Deallocate(1, sizeInBytes);
#endif
}
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
    constexpr size_t alignment = FPlatformMemory::AllocationGranularity;
    Assert(Meta::IsAligned(alignment, sizeInBytes));

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

                Assert_NoAssume(Meta::IsAligned(alignment, result));

#if USE_VMCACHE_PAGE_PROTECT
                // Restore read+write access to pages, so it won't crash in the client
                FVirtualMemory::Protect(result, cachedBlockSize, true, true);
#endif

                return result;
            }

            // unlock from here on to diminish thread contention and avoid potential recursive lock in ReleaseAll()
            scopeLock.unlock();

            if (void* result = FVirtualMemory::Alloc(alignment, sizeInBytes TRACKINGDATA_ARG_FWD)) {
                Assert(Meta::IsAligned(alignment, result));
                return result;
            }

            // Are we holding on too much mem? Release it all.
            ReleaseAll(first TRACKINGDATA_ARG_FWD);
        }
    }

    // allocate new blocks outside the barrier to diminish thread contention

    void* result = FVirtualMemory::Alloc(alignment, sizeInBytes TRACKINGDATA_ARG_FWD);
    Assert_NoAssume(Meta::IsAligned(alignment, result));

    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    Assert(ptr);

    if (0 == sizeInBytes)
        sizeInBytes = FVirtualMemory::SizeInBytes(ptr);

    Assert(Meta::IsAligned(FPlatformMemory::AllocationGranularity, sizeInBytes));

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
        Assert(Meta::IsAligned(FPlatformMemory::AllocationGranularity, first->Ptr));

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
