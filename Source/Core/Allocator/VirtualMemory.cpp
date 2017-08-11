#include "stdafx.h"

#include "VirtualMemory.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Misc/TargetPlatform.h"

#if     defined(PLATFORM_WINDOWS)
#   include "Misc/Platform_Windows.h"
#else
#   error "unsupported platform"
#endif

#ifdef USE_MEMORY_DOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTrackingData& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FVirtualMemory::AllocSizeInBytes(void* ptr) {
#if     defined(PLATFORM_WINDOWS)
//  https://msdn.microsoft.com/en-us/library/windows/desktop/aa366902(v=vs.85).aspx
    ::MEMORY_BASIC_INFORMATION info;
    if (0 == ::VirtualQuery(ptr, &info, sizeof(info)))
        AssertNotReached();

    Assert(ptr == info.BaseAddress);

    return info.RegionSize;
#else
#   error "not implemented !"
#endif
}
//----------------------------------------------------------------------------
// Keep allocations aligned to OS granularity
// https://github.com/r-lyeh/ltalloc/blob/master/ltalloc.cc
void* FVirtualMemory::AlignedAlloc(size_t alignment, size_t sizeInBytes) {
    Assert(Meta::IsPow2(alignment));

#if     defined(PLATFORM_WINDOWS)
    // Optimistically try mapping precisely the right amount before falling back to the slow method :
    void* p = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    if (not IS_ALIGNED(alignment, p)) {
        const size_t allocationGranularity = FPlatform::SystemInfo.AllocationGranularity;

        // Fill "bubbles" (reserve unaligned regions) at the beginning of virtual address space, otherwise there will be always falling back to the slow method
        if ((uintptr_t)p < 16 * 1024 * 1024)
            ::VirtualAlloc(p, alignment - ((uintptr_t)p & (alignment - 1)), MEM_RESERVE, PAGE_NOACCESS);

        do {
            p = ::VirtualAlloc(NULL, sizeInBytes + alignment - allocationGranularity, MEM_RESERVE, PAGE_NOACCESS);
            if (nullptr == p)
                return nullptr;

            ::VirtualFree(p, 0, MEM_RELEASE);// Unfortunately, WinAPI doesn't support release a part of allocated region, so release a whole region

            p = ::VirtualAlloc(
                (void*)(((uintptr_t)p + (alignment - 1)) & ~(alignment - 1)),
                sizeInBytes,
                MEM_RESERVE|MEM_COMMIT,
                PAGE_READWRITE );

        } while (nullptr == p);
    }

    return p;

#elif   defined(PLATFORM_LINUX)
    void* p = (void*)(((uintptr_t)::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0) + 1)&~1);//with the conversion of MAP_FAILED to 0

    if (not IS_ALIGNED(alignment, p)) {
        p = VMALLOC(size + alignment - page_size());
        if (p/* != MAP_FAILED*/) {
            uintptr_t ap = ((uintptr_t)p + (alignment - 1)) & ~(alignment - 1);
            uintptr_t diff = ap - (uintptr_t)p;
            if (diff) VMFREE(p, diff);
            diff = alignment - page_size() - diff;
            assert((intptr_t)diff >= 0);
            if (diff) VMFREE((void*)(ap + size), diff);
            return (void*)ap;
        }
    }

    return p;

#else
#   error "unsupported platform"
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemory::AlignedFree(void* ptr, size_t sizeInBytes) {
#if     defined(PLATFORM_WINDOWS)
    UNUSED(sizeInBytes);

    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
        throw FLastErrorException();

#elif   defined(PLATFORM_LINUX)

#else
#   error "unsupported platform"
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVirtualMemoryCache::FVirtualMemoryCache()
    : FreePageBlockCount(0)
    , TotalCacheSizeInBytes(0) {}
//----------------------------------------------------------------------------
void* FVirtualMemoryCache::Allocate(size_t sizeInBytes, FFreePageBlock* first TRACKINGDATA_ARG_IFP) {
    const size_t alignment = FPlatform::SystemInfo.AllocationGranularity;
    Assert(IS_ALIGNED(alignment, sizeInBytes));

    if (FreePageBlockCount) {
        FFreePageBlock* cachedBlock = nullptr;

        FFreePageBlock* const last = first + FreePageBlockCount;
        for (FFreePageBlock* block = first; block != last; ++block) {
            // look for exact matches first, these are aligned to the page size, so it should be quite common to hit these on small pages sizes
            if (block->SizeInBytes == sizeInBytes) {
                cachedBlock = block;
                break;
            }
        }

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

        if (nullptr != cachedBlock) {
            void* result = cachedBlock->Ptr;
            Assert(nullptr != result);

            FreePageBlockCount--;
            TotalCacheSizeInBytes -= cachedBlock->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
            // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
            trackingData.Deallocate(cachedBlock->SizeInBytes / alignment, alignment);
#endif

            if (cachedBlock + 1 != last)
                ::memmove(cachedBlock, cachedBlock + 1, sizeof(FFreePageBlock) * (last - cachedBlock - 1));

            Assert(IS_ALIGNED(alignment, result));
            return result;
        }

        if (void* result = FVirtualMemory::AlignedAlloc(alignment, sizeInBytes)) {
            Assert(IS_ALIGNED(alignment, result));
            return result;
        }

        // Are we holding on to much mem? Release it all.
        ReleaseAll(first TRACKINGDATA_ARG_FWD);
    }

    void* result = FVirtualMemory::AlignedAlloc(alignment, sizeInBytes);
    Assert(IS_ALIGNED(alignment, result));
    Assert(FVirtualMemory::AllocSizeInBytes(result) == sizeInBytes);

    return result;
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    if (0 == sizeInBytes)
        sizeInBytes = FVirtualMemory::AllocSizeInBytes(ptr);

    const size_t alignment = FPlatform::SystemInfo.AllocationGranularity;
    Assert(IS_ALIGNED(alignment, sizeInBytes));

    if (sizeInBytes > maxCacheSizeInBytes) {
        FVirtualMemory::AlignedFree(ptr, sizeInBytes);
        return;
    }

    while (FreePageBlockCount >= cacheBlocksCapacity || TotalCacheSizeInBytes + sizeInBytes > maxCacheSizeInBytes) {
        Assert(FreePageBlockCount);

        FVirtualMemory::AlignedFree(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        trackingData.Deallocate(first->SizeInBytes / alignment, alignment);
#endif

#ifdef _DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif

        if (--FreePageBlockCount)
            ::memmove(first, first + 1, sizeof(FFreePageBlock) * FreePageBlockCount);
    }

    first[FreePageBlockCount] = { ptr, sizeInBytes };
    TotalCacheSizeInBytes += sizeInBytes;
    FreePageBlockCount++;

#ifdef USE_MEMORY_DOMAINS
    // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
    trackingData.Allocate(sizeInBytes / alignment, alignment);
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP) {
    for (   FFreePageBlock* const last = (first + FreePageBlockCount);
            first != last;
            ++first ) {
        Assert(IS_ALIGNED(FPlatform::SystemInfo.AllocationGranularity, first->Ptr));

        FVirtualMemory::AlignedFree(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        const size_t alignment = FPlatform::SystemInfo.AllocationGranularity;
        trackingData.Deallocate(first->SizeInBytes / alignment, alignment);
#endif

#ifdef _DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif
    }
    Assert(0 == TotalCacheSizeInBytes);
    FreePageBlockCount = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#undef TRACKINGDATA_ARG_IFP
