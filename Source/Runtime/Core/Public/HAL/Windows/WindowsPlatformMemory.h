#pragma once

#include "HAL/Generic/GenericPlatformMemory.h"

#ifdef PLATFORM_WINDOWS

#include <emmintrin.h>
#include <intrin.h>
#include <xmmintrin.h>

#define USE_PPE_WIN32MEMORY_SIMD (!USE_PPE_MEMORY_DEBUGGING)

#ifdef __AVX2__
#   include <immintrin.h>
#endif

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

    using FGenericPlatformMemory::FStackUsage;
    static FStackUsage StackUsage();

    static void* AddressOfReturnAddress();

    //------------------------------------------------------------------------
    // virtual memory *DONT USE THOSE DIRECTLY, SEE VirtualMemory.h * (no logs !)

    static void* PageAlloc(size_t sizeInBytes);
    static void PageFree(void* ptr, size_t sizeInBytes);

    static void* VirtualAlloc(size_t sizeInBytes, bool commit);
    static void* VirtualAlloc(size_t alignment, size_t sizeInBytes, bool commit);
    static void VirtualCommit(void* ptr, size_t sizeInBytes);
    static void VirtualFree(void* ptr, size_t sizeInBytes, bool release);

    static size_t RegionSize(void* ptr);
    static bool PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write);

    //------------------------------------------------------------------------
    // memory block helpers

    using FGenericPlatformMemory::Memaliases;
    using FGenericPlatformMemory::Memoverlap;
    using FGenericPlatformMemory::Memcmp;
    using FGenericPlatformMemory::Memset;
    using FGenericPlatformMemory::Memzero;
    using FGenericPlatformMemory::Memcpy;
    using FGenericPlatformMemory::Memmove;
    using FGenericPlatformMemory::Memswap;

#if USE_PPE_ASSERT
    using FGenericPlatformMemory::Memdeadbeef;
#endif

    static FORCE_INLINE void* Memstream(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        AssertRelease(Meta::IsAligned(16, dst)); // everything assumed to be aligned, no reminder
        AssertRelease(Meta::IsAligned(16, src));
        AssertRelease(Meta::IsAligned(16, sizeInBytes));
        Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));

#if USE_PPE_WIN32MEMORY_SIMD
        STATIC_ASSERT(sizeof(::__m128i) == 16);
        // use SSE2 to minimize cache pollution
        const size_t nblks128 = (sizeInBytes / sizeof(::__m128i));
		const ::__m128i* __restrict src128 = reinterpret_cast<const ::__m128i*>(src);
		::__m128i* __restrict dst128 = reinterpret_cast<::__m128i*>(dst);
        for (size_t b = nblks128; b > 0; b--, src128++, dst128++) {
            const ::__m128i loaded = ::_mm_stream_load_si128(src128);
            ::_mm_stream_si128(dst128, loaded);
        }

        ::_mm_sfence(); // synchronize stores before yielding
#else
        ::memcpy(dst, src, sizeInBytes);
#endif
        return dst;
    }

	static FORCE_INLINE void* MemstreamLarge(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
#if USE_PPE_WIN32MEMORY_SIMD && __AVX2__
		AssertRelease(Meta::IsAligned(32, dst)); // everything assumed to be aligned, no reminder
		AssertRelease(Meta::IsAligned(32, src));
		AssertRelease(Meta::IsAligned(32, sizeInBytes));
		Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));
		STATIC_ASSERT(sizeof(::__m256i) == 32);

		// use SSE2 to minimize cache pollution
		const ::__m256i* __restrict pSrc = reinterpret_cast<const ::__m256i*>(src);
		::__m256i* __restrict pDst = reinterpret_cast<::__m256i*>(dst);
		for (size_t b = (sizeInBytes / sizeof(::__m256i)); b > 0; b--, pSrc++, pDst++) {
			const ::__m256i loaded = ::_mm256_stream_load_si256(pSrc);
			::_mm256_stream_si256(pDst, loaded);
		}

        ::_mm_sfence(); // synchronize stores before yielding

		return dst;
#else
        return Memstream(dst, src, sizeInBytes);
#endif
	}

    static FORCE_INLINE void* MemcpyLarge(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        AssertRelease(Meta::IsAligned(16, dst)); // only need ptr aligned, not size
        AssertRelease(Meta::IsAligned(16, src));
        Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));

#if USE_PPE_WIN32MEMORY_SIMD
        STATIC_ASSERT(sizeof(::__m128i) == 16);

        // use SSE2 to minimize cache pollution
        const size_t blks = (sizeInBytes / sizeof(::__m128i));
        const ::__m128i* __restrict pSrc = reinterpret_cast<const ::__m128i*>(src);
        ::__m128i* __restrict pDst = reinterpret_cast<::__m128i*>(dst);
        for (size_t b = blks; b > 0; b--, pSrc++, pDst++) {
            const ::__m128i loaded = ::_mm_stream_load_si128(pSrc);
            ::_mm_stream_si128(pDst, loaded);
        }

        // use regular copy for unaligned reminder
        FGenericPlatformMemory::MemcpyLarge(pDst, pSrc, sizeInBytes - blks * sizeof(::__m128i));

        ::_mm_sfence(); // synchronize stores before yielding
#else
        ::memcpy(dst, src, sizeInBytes);
#endif

        return dst;
    }

    //------------------------------------------------------------------------
    // prefetch

    static void ReadPrefetch(const void* p, size_t offset = 0) {
        ::_mm_prefetch((char const*)(p)+offset, _MM_HINT_T0);
    }

    static void ReadPrefetchBlock(const void* p, size_t sizeInBytes) {
        for (size_t lines = (sizeInBytes + CacheLineSize - 1) / CacheLineSize; lines; --lines) {
            ::_mm_prefetch((const char*)p, _MM_HINT_T0);
            p = (const char*)p + CacheLineSize;
        }
    }

    static void WritePrefetch(const void* p, size_t offset = 0) {
#ifdef __clang__
        UNUSED(p);
        UNUSED(offset);
#else
        ::_m_prefetchw((char const*)(p)+offset);
#endif
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
