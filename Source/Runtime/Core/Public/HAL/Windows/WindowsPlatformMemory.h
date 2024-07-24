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
struct FWindowsPlatformMemory : FGenericPlatformMemory {
public:
    STATIC_CONST_INTEGRAL(size_t, PageSize, PAGE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, CacheLineSize, CACHELINE_SIZE);
    static const size_t AllocationGranularity;

    //------------------------------------------------------------------------
    // platform memory infos

    using FGenericPlatformMemory::FConstants;
    static PPE_CORE_API const FConstants& Constants();

    using FGenericPlatformMemory::FStats;
    static PPE_CORE_API FStats Stats();

    using FGenericPlatformMemory::FStackUsage;
    static PPE_CORE_API FStackUsage StackUsage();

    static void* AddressOfReturnAddress() {
        // https://docs.microsoft.com/en-us/cpp/intrinsics/addressofreturnaddress?view=vs-2017
        return _AddressOfReturnAddress();
    }

    //------------------------------------------------------------------------
    // virtual memory *DONT USE THOSE DIRECTLY, SEE VirtualMemory.h * (no logs !)

    static PPE_CORE_API void* PageAlloc(size_t sizeInBytes);
    static PPE_CORE_API void PageFree(void* ptr, size_t sizeInBytes);

    static PPE_CORE_API void* VirtualAlloc(size_t sizeInBytes, bool commit);
    static PPE_CORE_API void* VirtualAlloc(size_t alignment, size_t sizeInBytes, bool commit);
    static PPE_CORE_API void VirtualCommit(void* ptr, size_t sizeInBytes);
    static PPE_CORE_API void VirtualFree(void* ptr, size_t sizeInBytes, bool release);

    static PPE_CORE_API size_t RegionSize(void* ptr);
    static PPE_CORE_API bool PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write);

    //------------------------------------------------------------------------
    // system allocator

    NODISCARD PPE_CORE_API static void* SystemMalloc(size_t s);
    NODISCARD PPE_CORE_API static void* SystemRealloc(void* p, size_t s);
    PPE_CORE_API static void SystemFree(void* p);

    NODISCARD PPE_CORE_API static void* SystemAlignedMalloc(size_t s, size_t boundary);
    NODISCARD PPE_CORE_API static void* SystemAlignedRealloc(void* p, size_t s, size_t boundary);
    PPE_CORE_API static void SystemAlignedFree(void* p, size_t boundary);

#if !USE_PPE_FINAL_RELEASE
    PPE_CORE_API static size_t SystemAlignedRegionSize(void* p, size_t boundary);
#endif

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
        AssertRelease(Meta::IsAlignedPow2(16, dst)); // everything assumed to be aligned, no reminder
        AssertRelease(Meta::IsAlignedPow2(16, src));
        AssertRelease(Meta::IsAlignedPow2(16, sizeInBytes));
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
		AssertRelease(Meta::IsAlignedPow2(32, dst)); // everything assumed to be aligned, no reminder
		AssertRelease(Meta::IsAlignedPow2(32, src));
		AssertRelease(Meta::IsAlignedPow2(32, sizeInBytes));
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
        Assert_NoAssume(not FGenericPlatformMemory::Memoverlap(dst, sizeInBytes, src, sizeInBytes));

#if USE_PPE_WIN32MEMORY_SIMD
        STATIC_ASSERT(sizeof(::__m128i) == 16);

        // use SSE2 to minimize cache pollution
        const size_t blks = (sizeInBytes / sizeof(::__m128i));
        const ::__m128i* __restrict pSrc = reinterpret_cast<const ::__m128i*>(src);
        ::__m128i* __restrict pDst = reinterpret_cast<::__m128i*>(dst);
        for (size_t b = blks; b > 0; b--, pSrc++, pDst++) {
            const ::__m128i loaded = ::_mm_loadu_si128(pSrc);
            ::_mm_storeu_si128(pDst, loaded);
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
        Unused(p);
        Unused(offset);
#else
        ::_m_prefetchw((char const*)(p)+offset);
#endif
    }

    //------------------------------------------------------------------------
    // out of memory

    using FGenericPlatformMemory::BackupMemorySizeInBytes;

    static PPE_CORE_API void OnOutOfMemory(size_t sizeInBytes, size_t alignment);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
