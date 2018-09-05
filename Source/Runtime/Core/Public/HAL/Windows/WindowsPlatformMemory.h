#pragma once

#include "HAL/Generic/GenericPlatformMemory.h"

#ifdef PLATFORM_WINDOWS

#include <emmintrin.h>
#include <intrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformMemory : FGenericPlatformMemory {
public:
    STATIC_CONST_INTEGRAL(size_t, PageSize, PAGE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, CacheLineSize, CACHELINE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, AllocationGranularity, ALLOCATION_GRANULARITY);

    //------------------------------------------------------------------------
    // platform memory infos

    using FGenericPlatformMemory::FConstants;
    static const FConstants& Constants();

    using FGenericPlatformMemory::FStats;
    static FStats Stats();

    //------------------------------------------------------------------------
    // virtual memory *DONT USE THOSE DIRECTLY, SEE VirtualMemory.h * (no logs !)

    static void* PageAlloc(size_t sizeInBytes);
    static void PageFree(void* ptr, size_t sizeInBytes);

    static void* VirtualAlloc(size_t sizeInBytes);
    static void* VirtualAlloc(size_t alignment, size_t sizeInBytes);
    static void VirtualFree(void* ptr, size_t sizeInBytes);

    static size_t RegionSize(void* ptr);
    static bool PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write);

    //------------------------------------------------------------------------
    // memory block helpers

    using FGenericPlatformMemory::Memcmp;
    using FGenericPlatformMemory::Memset;
    using FGenericPlatformMemory::Memzero;
    using FGenericPlatformMemory::Memcpy;
    using FGenericPlatformMemory::Memmove;
    using FGenericPlatformMemory::Memswap;

#ifdef WITH_PPE_ASSERT
    using FGenericPlatformMemory::Memdeadbeef;
#endif

    static FORCE_INLINE void* Memstream(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        AssertRelease(Meta::IsAligned(16, dst)); // everything assumed to be aligned, no reminder
        AssertRelease(Meta::IsAligned(16, src));
        AssertRelease(Meta::IsAligned(16, sizeInBytes));
        Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));
        STATIC_ASSERT(sizeof(::__m128i) == 16);

        const size_t blks = (sizeInBytes / sizeof(::__m128i));

        // use SSE2 to minimize cache pollution
        const ::__m128i* pSrc = reinterpret_cast<const ::__m128i*>(src);
        ::__m128i* pDst = reinterpret_cast<::__m128i*>(dst);
        for (size_t b = blks; b > 0; b--, pSrc++, pDst++) {
            const ::__m128i loaded = ::_mm_stream_load_si128(pSrc);
            ::_mm_stream_si128(pDst, loaded);
        }

        // synchronize loads and stores before yielding
        ::_mm_mfence();

        return dst;
    }

    static FORCE_INLINE void* MemcpyLarge(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        AssertRelease(Meta::IsAligned(16, dst)); // only need ptr aligned, not size
        AssertRelease(Meta::IsAligned(16, src));
        Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));
        STATIC_ASSERT(sizeof(::__m128i) == 16);

        const size_t blks = (sizeInBytes / sizeof(::__m128i));

        // use SSE2 to minimize cache pollution
        const ::__m128i *pSrc = reinterpret_cast<const ::__m128i*>(src);
        ::__m128i *pDst = reinterpret_cast<::__m128i*>(dst);
        for (size_t b = blks; b > 0; b--, pSrc++, pDst++) {
            const ::__m128i loaded = ::_mm_stream_load_si128(pSrc);
            ::_mm_stream_si128(pDst, loaded);
        }

        // use regular copy for unaligned reminder
        FGenericPlatformMemory::MemcpyLarge(pDst, pSrc, sizeInBytes - blks * sizeof(::__m128i));

        // synchronize loads and stores before yielding
        ::_mm_mfence();

        return dst;
    }

    //------------------------------------------------------------------------
    // prefetch

    static void Prefetch(const void* p, size_t offset = 0) {
        return ::_mm_prefetch((char const*)(p)+offset, _MM_HINT_T0);
    }

    static void PrefetchBlock(const void* p, size_t sizeInBytes) {
        for (size_t lines = (sizeInBytes + CacheLineSize - 1) / CacheLineSize; lines; --lines) {
            ::_mm_prefetch((const char*)p, _MM_HINT_T0);
            p = (const char*)p + CacheLineSize;
        }
    }

    //------------------------------------------------------------------------
    // out of memory

    using FGenericPlatformMemory::BackupMemorySizeInBytes;

    static void OnOutOfMemory(size_t sizeInBytes, size_t alignment);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
