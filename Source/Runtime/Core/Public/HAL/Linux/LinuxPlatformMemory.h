#pragma once

#include "HAL/Generic/GenericPlatformMemory.h"

#ifdef PLATFORM_LINUX

#include <emmintrin.h>
#include <xmmintrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformMemory : FGenericPlatformMemory {
public:
    STATIC_CONST_INTEGRAL(size_t, PageSize, PAGE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, CacheLineSize, CACHELINE_SIZE);
    static const size_t AllocationGranularity;

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

    using FGenericPlatformMemory::Memcmp;
    using FGenericPlatformMemory::Memset;
    using FGenericPlatformMemory::Memzero;
    using FGenericPlatformMemory::Memcpy;
    using FGenericPlatformMemory::Memmove;
    using FGenericPlatformMemory::Memswap;

#if USE_PPE_ASSERT
    using FGenericPlatformMemory::Memdeadbeef;
#endif

    using FGenericPlatformMemory::Memstream;
    using FGenericPlatformMemory::MemcpyLarge;

    //------------------------------------------------------------------------
    // prefetch

    static void ReadPrefetch(const void* p, size_t offset = 0) {
        _mm_prefetch((char const*)(p)+offset, _MM_HINT_T0);
    }

    static void ReadPrefetchBlock(const void* p, size_t sizeInBytes) {
        for (size_t lines = (sizeInBytes + CacheLineSize - 1) / CacheLineSize; lines; --lines) {
            _mm_prefetch((const char*)p, _MM_HINT_T0);
            p = (const char*)p + CacheLineSize;
        }
    }

    static void WritePrefetch(const void* , size_t ) {}

    //------------------------------------------------------------------------
    // out of memory

    using FGenericPlatformMemory::BackupMemorySizeInBytes;

    static void OnOutOfMemory(size_t sizeInBytes, size_t alignment);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
