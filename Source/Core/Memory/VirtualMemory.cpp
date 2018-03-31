#include "stdafx.h"

#include "VirtualMemory.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Container/CompressedRadixTrie.h"
#include "Core/Misc/TargetPlatform.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

#if     defined(PLATFORM_WINDOWS)
#   include "Misc/Platform_Windows.h"
#   define VMALLOC(_SizeInBytes) ::VirtualAlloc(nullptr, (_SizeInBytes), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)
#   define VMFREE(_Ptr, _SizeInBytes) ::VirtualFree((_Ptr), 0, MEM_RELEASE)
#elif   defined(PLATFORM_LINUX)
#   include "Misc/Platform_Linux.h"
#   define VMALLOC(_SizeInBytes) (void*)(((uintptr_t)mmap(NULL, (_SizeInBytes), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0)+1)&~1)//with the conversion of MAP_FAILED to 0
#   define VMFREE(_Ptr, _SizeInBytes) munmap(_Ptr), (_SizeInBytes))
#else
#   error "unsupported platform"
#endif

#ifdef USE_MEMORY_DOMAINS
#   define TRACKINGDATA_ARG_IFP , FMemoryTracking& trackingData
#   define TRACKINGDATA_ARG_FWD , trackingData
#else
#   define TRACKINGDATA_ARG_IFP
#   define TRACKINGDATA_ARG_FWD
#endif

#define USE_VMALLOC_SIZE_PTRIE          1// This is faster than ::VirtualQuery()

#if (defined(WITH_CORE_ASSERT) || USE_CORE_MEMORY_DEBUGGING)
#   define USE_VMCACHE_PAGE_PROTECT     1// Crash when using a VM cached block
#else
#   define USE_VMCACHE_PAGE_PROTECT     0
#endif

#if USE_VMALLOC_SIZE_PTRIE
#   include "Core/Thread/AtomicSpinLock.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_VMALLOC_SIZE_PTRIE
namespace {
//----------------------------------------------------------------------------
// Compressed radix trie method from :
// https://github.com/r-lyeh/ltalloc/blob/4ad53ea91c359a07f97de65d93fb8a7d279354bd/ltalloc.cc
//----------------------------------------------------------------------------
static FCompressedRadixTrie& VirtualMemoryAllocs_() {
    ONE_TIME_DEFAULT_INITIALIZE(FCompressedRadixTrie, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
static void VMRegisterBlockSize_(void* ptr, size_t sizeInBytes) {
    VirtualMemoryAllocs_().Insert(uintptr_t(ptr), uintptr_t(sizeInBytes));
}
//----------------------------------------------------------------------------
static size_t VMFetchBlockSize_(void* ptr) {
    return VirtualMemoryAllocs_().Lookup(uintptr_t(ptr));
}
//----------------------------------------------------------------------------
static size_t VMReleaseBlockSize_(void* ptr) {
    return VirtualMemoryAllocs_().Erase(uintptr_t(ptr));
}
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
    const size_t regionSize = VMFetchBlockSize_(ptr);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, regionSize));

    return regionSize;

#else
#   if defined(PLATFORM_WINDOWS)
    //  https://msdn.microsoft.com/en-us/library/windows/desktop/aa366902(v=vs.85).aspx
    ::MEMORY_BASIC_INFORMATION info;
    if (0 == ::VirtualQuery(ptr, &info, sizeof(info)))
        AssertNotReached();

    Assert(ptr == info.BaseAddress);

    return info.RegionSize;
#   else
#       error "not implemented !"
#   endif
#endif
}
//----------------------------------------------------------------------------
bool FVirtualMemory::Protect(void* ptr, size_t sizeInBytes, bool read, bool write) {
    Assert(ptr);
    Assert(sizeInBytes);

#if     defined(PLATFORM_WINDOWS)
    ::DWORD oldProtect, newProtect;

    if (read && write)
        newProtect = PAGE_READWRITE;
    else if (write)
        newProtect = PAGE_READWRITE;
    else if (read)
        newProtect = PAGE_READONLY;
    else
        newProtect = PAGE_NOACCESS;

    return (0 != ::VirtualProtect(ptr, sizeInBytes, newProtect, &oldProtect));

#else
#       error "Unsupported platform !"
#endif
}
//----------------------------------------------------------------------------
// Keep allocations aligned to OS granularity
// https://github.com/r-lyeh/ltalloc/blob/master/ltalloc.cc
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(6001) // Using uninitialized memory 'XXX'.
void* FVirtualMemory::Alloc(size_t alignment, size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAligned(ALLOCATION_GRANULARITY, sizeInBytes));
    Assert(Meta::IsPow2(alignment));


#if     defined(PLATFORM_WINDOWS)
    // Optimistically try mapping precisely the right amount before falling back to the slow method :
    void* p = ::VirtualAlloc(nullptr, sizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#ifdef USE_MEMORY_DOMAINS
    if (p)
        MEMORY_DOMAIN_TRACKING_DATA(Reserved).Allocate(1, sizeInBytes);
#endif

    if (not Meta::IsAligned(alignment, p)) {
        const size_t allocationGranularity = FPlatformMisc::SystemInfo.AllocationGranularity;

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

#elif   defined(PLATFORM_LINUX)
    void* p = (void*)(((uintptr_t)::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0) + 1)&~1);//with the conversion of MAP_FAILED to 0

    if (not Meta::IsAligned(alignment, p)) {
        p = VMALLOC(size + alignment - ALLOCATION_GRANULARITY());
        if (p/* != MAP_FAILED*/) {
            uintptr_t ap = ((uintptr_t)p + (alignment - 1)) & ~(alignment - 1);
            uintptr_t diff = ap - (uintptr_t)p;
            if (diff) VMFREE(p, diff);
            diff = alignment - ALLOCATION_GRANULARITY() - diff;
            assert((intptr_t)diff >= 0);
            if (diff) VMFREE((void*)(ap + size), diff);
            return (void*)ap;
        }
    }

#else
#   error "unsupported platform"
#endif

#if USE_VMALLOC_SIZE_PTRIE
    VMRegisterBlockSize_(p, sizeInBytes);
#endif

    return p;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
void FVirtualMemory::Free(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(sizeInBytes);

#if USE_VMALLOC_SIZE_PTRIE
    const size_t regionSize = VMReleaseBlockSize_(ptr);
    Assert(regionSize == sizeInBytes);
#endif

#ifdef USE_MEMORY_DOMAINS
    if (ptr)
        MEMORY_DOMAIN_TRACKING_DATA(Reserved).Deallocate(1, sizeInBytes);
#endif

#if     defined(PLATFORM_WINDOWS)
    UNUSED(sizeInBytes);
    if (0 == ::VirtualFree(ptr, 0, MEM_RELEASE))
        CORE_THROW_IT(FLastErrorException("VirtualFree"));

#elif   defined(PLATFORM_LINUX)

#else
#   error "unsupported platform"
#endif
}
//----------------------------------------------------------------------------
void* FVirtualMemory::InternalAlloc(size_t sizeInBytes) {
    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, sizeInBytes));

    void* const ptr = VMALLOC(sizeInBytes);
    Assert(ptr);

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Internal).Allocate(1, sizeInBytes);
#endif

    return ptr;
}
//----------------------------------------------------------------------------
void FVirtualMemory::InternalFree(void* ptr, size_t sizeInBytes) {
    Assert(ptr);
    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, sizeInBytes));

    VMFREE(ptr, sizeInBytes);

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Internal).Deallocate(1, sizeInBytes);
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
void* FVirtualMemoryCache::Allocate(size_t sizeInBytes, FFreePageBlock* first, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    const size_t alignment = FPlatformMisc::SystemInfo.AllocationGranularity;
    Assert(Meta::IsAligned(alignment, sizeInBytes));

    if (FreePageBlockCount && (sizeInBytes <= maxCacheSizeInBytes / 4)) {
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

#if 0 // this is better to release the chunks which are too large, since the client can't use the extra memory
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

#ifdef USE_MEMORY_DOMAINS
            // Only track overhead due to cached memory, actual blocks in use should be logged in their own domain
            trackingData.Deallocate(1, cachedBlockSize);
#endif

            if (cachedBlock + 1 != last)
                ::memmove(cachedBlock, cachedBlock + 1, sizeof(FFreePageBlock) * (last - cachedBlock - 1));

            Assert(Meta::IsAligned(alignment, result));

#if USE_VMCACHE_PAGE_PROTECT
            // Restore read+write access to pages, so it won't crash in the client
            FVirtualMemory::Protect(result, cachedBlockSize, true, true);
#endif

            return result;
        }

        if (void* result = FVirtualMemory::Alloc(alignment, sizeInBytes)) {
            Assert(Meta::IsAligned(alignment, result));
            return result;
        }

        // Are we holding on too much mem? Release it all.
        ReleaseAll(first TRACKINGDATA_ARG_FWD);
    }

    void* result = FVirtualMemory::Alloc(alignment, sizeInBytes);
    Assert(Meta::IsAligned(alignment, result));
    Assert(FVirtualMemory::SizeInBytes(result) == sizeInBytes);

    return result;
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::Free(void* ptr, size_t sizeInBytes, FFreePageBlock* first, size_t cacheBlocksCapacity, size_t maxCacheSizeInBytes TRACKINGDATA_ARG_IFP) {
    if (0 == sizeInBytes)
        sizeInBytes = FVirtualMemory::SizeInBytes(ptr);

    Assert(Meta::IsAligned(FPlatformMisc::SystemInfo.AllocationGranularity, sizeInBytes));

    if (sizeInBytes > maxCacheSizeInBytes / 4) {
        FVirtualMemory::Free(ptr, sizeInBytes);
        return;
    }

    while (FreePageBlockCount >= cacheBlocksCapacity || TotalCacheSizeInBytes + sizeInBytes > maxCacheSizeInBytes) {
        Assert(FreePageBlockCount);

        FVirtualMemory::Free(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        trackingData.Deallocate(1, first->SizeInBytes);
#endif

#ifdef _DEBUG
        first->Ptr = nullptr;
        first->SizeInBytes = 0;
#endif

        if (--FreePageBlockCount)
            ::memmove(first, first + 1, sizeof(FFreePageBlock) * FreePageBlockCount);
    }

    ONLY_IF_ASSERT(::memset(ptr, 0xDD, sizeInBytes)); // trash the memory block before caching

    first[FreePageBlockCount] = FFreePageBlock{ ptr, sizeInBytes };
    TotalCacheSizeInBytes += sizeInBytes;
    FreePageBlockCount++;

#ifdef USE_MEMORY_DOMAINS
    // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
    trackingData.Allocate(1, sizeInBytes);
#endif

#if USE_VMCACHE_PAGE_PROTECT
    // Forbid access to cached pages, so it will crash in the client it there's necrophilia attempt (read+write)
    FVirtualMemory::Protect(ptr, sizeInBytes, false, false);
#endif
}
//----------------------------------------------------------------------------
void FVirtualMemoryCache::ReleaseAll(FFreePageBlock* first TRACKINGDATA_ARG_IFP) {
    for (   FFreePageBlock* const last = (first + FreePageBlockCount);
            first != last;
            ++first ) {
        Assert(Meta::IsAligned(FPlatformMisc::SystemInfo.AllocationGranularity, first->Ptr));

        FVirtualMemory::Free(first->Ptr, first->SizeInBytes);

        Assert(TotalCacheSizeInBytes >= first->SizeInBytes);
        TotalCacheSizeInBytes -= first->SizeInBytes;

#ifdef USE_MEMORY_DOMAINS
        // Only track overhead due to cached memory, actual blocks in use should be logger in their owning domain
        trackingData.Deallocate(1, first->SizeInBytes);
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
