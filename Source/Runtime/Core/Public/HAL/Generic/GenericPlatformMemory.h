#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericPlatformMemoryConstants {
    u64 AddressLimit;
    u64 AllocationGranularity;
    u64 CacheLineSize;
    u64 PageSize;
    u64 TotalPhysical;
    u64 TotalVirtual;
};
//----------------------------------------------------------------------------
struct FGenericPlatformMemoryStats {
    u64 AvailablePhysical;
    u64 AvailableVirtual;
    u64 UsedPhysical;
    u64 UsedVirtual;
    u64 PeakUsedVirtual;
    u64 PeakUsedPhysical;
};
//----------------------------------------------------------------------------
struct PPE_API FGenericPlatformMemory {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(size_t, CacheLineSize, 64);

    //------------------------------------------------------------------------
    // platform memory infos

    using FConstants = FGenericPlatformMemoryConstants;
    static const FConstants& Constants() = delete;

    using FStats = FGenericPlatformMemoryStats;
    static FStats Stats() = delete;

    //------------------------------------------------------------------------
    // virtual memory *DONT USE THOSE DIRECTLY* (no logs !)

    static void* PageAlloc(size_t sizeInBytes) = delete; // PageGranularity
    static void PageFree(void* ptr, size_t sizeInBytes) = delete;

    static void* VirtualAlloc(size_t sizeInBytes) = delete; // AllocationGranularity
    static void* VirtualAlloc(size_t alignment, size_t sizeInBytes) = delete;
    static void VirtualFree(void* ptr, size_t sizeInBytes) = delete;

    static size_t RegionSize(void* ptr) = delete;
    static bool PageProtect(void* ptr, size_t sizeInBytes, bool read, bool write) = delete;

    //------------------------------------------------------------------------
    // memory block helpers

    static bool Memoverlap(const void* lhs, size_t lhsSize, const void* rhs, size_t rhsSize) {
        return (Max(uintptr_t(lhs) + lhsSize, uintptr_t(rhs) + rhsSize) -
                Min(uintptr_t(lhs), uintptr_t(rhs)) < (lhsSize + rhsSize) );
    }

    static FORCE_INLINE int Memcmp(const void* lhs, const void* rhs, size_t sizeInBytes) {
        return ::memcmp(lhs, rhs, sizeInBytes);
    }

    static FORCE_INLINE void* Memset(void* dst, u8 ch, size_t sizeInBytes) {
        return ::memset(dst, ch, sizeInBytes);
    }

    static FORCE_INLINE void* Memzero(void* dst, size_t sizeInBytes) {
        return ::memset(dst, 0, sizeInBytes);
    }

    static FORCE_INLINE void* Memcpy(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        Assert_NoAssume(not Memoverlap(dst, sizeInBytes, src, sizeInBytes));
        return ::memcpy(dst, src, sizeInBytes); // don't handle overlapping regions, use Memmove() for this
    }

    static FORCE_INLINE void* Memmove(void* dst, const void* src, size_t sizeInBytes) {
        return ::memmove(dst, src, sizeInBytes); // do support overlapped blocks
    }

    static FORCE_INLINE void* MemcpyLarge(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        Assert_NoAssume(not Memoverlap(dst, sizeInBytes, src, sizeInBytes));
        return ::memcpy(dst, src, sizeInBytes); // optimized for large blocks on some platforms
    }

    static FORCE_INLINE void* Memstream(void* __restrict dst, const void* __restrict src, size_t sizeInBytes) {
        Assert_NoAssume(not Memoverlap(dst, sizeInBytes, src, sizeInBytes));
        return ::memcpy(dst, src, sizeInBytes); // on some platform it's possible to avoid trashing L2 cache with streaming intrinsics
    }

    static FORCE_INLINE void Memswap(void* __restrict lhs, void* __restrict rhs, size_t sizeInBytes) {
        Assert_NoAssume(not Memoverlap(lhs, sizeInBytes, rhs, sizeInBytes));

        constexpr size_t w = sizeof(size_t);
        u8* p0 = (u8*)lhs;
        u8* p1 = (u8*)rhs;
        u8* const pend = (p0 + sizeInBytes);

        for (; p0 + w <= pend; p0 += w, p1 += w) {
            const size_t tmp = *(size_t*)p0;
            *(size_t*)p0 = *(size_t*)p1;
            *(size_t*)p1 = tmp;
        }

        for (; p0 != pend; ++p0, ++p1) {
            const u8 tmp = *p0;
            *p0 = *p1;
            *p1 = tmp;
        }
    }

    //------------------------------------------------------------------------
    // prefetch

    static void Prefetch(const void* p, size_t offset = 0) = delete; // stride is platform dependent
    static void PrefetchBlock(const void* p, size_t sizeInBytes) = delete;

    //------------------------------------------------------------------------
    // out of memory

#if USE_PPE_FINAL_RELEASE
    STATIC_CONST_INTEGRAL(size_t, BackupMemorySizeInBytes, 30 * 1024 * 1024); // disabled in final builds
#else
    STATIC_CONST_INTEGRAL(size_t, BackupMemorySizeInBytes, 30 * 1024 * 1024); // 30 mb to handle crashes
#endif

    static void OnOutOfMemory(size_t sizeInBytes, size_t alignment) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
