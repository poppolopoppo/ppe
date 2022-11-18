#include "stdafx.h"

#include "Allocator/Malloc.h"
#include "Allocator/Mallocator.h"
#include "Allocator/MallocBinned.h"
#include "Allocator/MallocBinned2.h"
#include "Allocator/MallocMipMap.h"
#include "Allocator/MallocStomp.h"

#include "Diagnostic/LeakDetector.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/Assert.h"

// Lowest level to hook or replace default allocator

#define PPE_MALLOC_ALLOCATOR_STD           0
#define PPE_MALLOC_ALLOCATOR_STOMP         1
#define PPE_MALLOC_ALLOCATOR_BINNED1       2
#define PPE_MALLOC_ALLOCATOR_BINNED2       3

#define PPE_MALLOC_FORCE_BINNED2           1 //%_NOCOMMIT%
#if PPE_MALLOC_FORCE_BINNED2
#   define PPE_MALLOC_ALLOCATOR_BINNED     PPE_MALLOC_ALLOCATOR_BINNED2
#else
#   define PPE_MALLOC_ALLOCATOR_BINNED     PPE_MALLOC_ALLOCATOR_BINNED1
#endif

#define PPE_MALLOC_FORCE_STD               (USE_PPE_SANITIZER || CODE3264(1, 0)) //%_NOCOMMIT% FMallocBinned2 doesn't handle well address space limited to 32 bits
#define PPE_MALLOC_FORCE_STOMP             (USE_PPE_MEMORY_DEBUGGING && !USE_PPE_SANITIZER && 1) //%_NOCOMMIT%

#if PPE_MALLOC_FORCE_STD
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_STD
#elif PPE_MALLOC_FORCE_STOMP
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_STOMP
#else
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_BINNED
#endif

#if USE_PPE_MEMORY_DEBUGGING && !PPE_MALLOC_FORCE_STD && !USE_PPE_SANITIZER
#   define PPE_MALLOC_DEBUG_PROXY          1
#   define PPE_MALLOC_HISTOGRAM_PROXY      1 // Keep memory histogram available, shouldn't have any influence on debugging
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   (USE_PPE_MALLOC_LEAKDETECTOR) // %_NOCOMMIT%
#   define PPE_MALLOC_POISON_PROXY         1 // Erase all data from blocks when allocating and releasing them, helps to find necrophilia
#   define PPE_MALLOC_UNACCOUNTED_PROXY    (USE_PPE_MEMORYDOMAINS)
#elif not (USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING || USE_PPE_SANITIZER)
#   define PPE_MALLOC_DEBUG_PROXY          1
#   define PPE_MALLOC_HISTOGRAM_PROXY      1 // %_NOCOMMIT%
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   (USE_PPE_MALLOC_LEAKDETECTOR) // %_NOCOMMIT%
#   define PPE_MALLOC_POISON_PROXY         (USE_PPE_DEBUG && !USE_PPE_FASTDEBUG) // %_NOCOMMIT%
#   define PPE_MALLOC_UNACCOUNTED_PROXY    (USE_PPE_MEMORYDOMAINS)
#else
#   define PPE_MALLOC_DEBUG_PROXY          0
#   define PPE_MALLOC_HISTOGRAM_PROXY      0
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   0
#   define PPE_MALLOC_POISON_PROXY         0
#   define PPE_MALLOC_UNACCOUNTED_PROXY    0
#endif

#define USE_PPE_MALLOC_PROXY               (PPE_MALLOC_HISTOGRAM_PROXY|PPE_MALLOC_LEAKDETECTOR_PROXY|PPE_MALLOC_POISON_PROXY)

#if PPE_MALLOC_DEBUG_PROXY
#   include "HAL/PlatformDebug.h"
#endif
#if PPE_MALLOC_HISTOGRAM_PROXY
#   include "Memory/MemoryTracking.h"
#endif
#if PPE_MALLOC_UNACCOUNTED_PROXY
#   include "Memory/MemoryDomain.h"
#   include "Memory/MemoryTracking.h"
#endif

PRAGMA_DISABLE_RUNTIMECHECKS

namespace PPE {
STATIC_ASSERT(PPE_MALLOC_LEAKDETECTOR_PROXY || not USE_PPE_MALLOC_LEAKDETECTOR);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FMallocLowLevel {
    FORCE_INLINE static void*   Malloc(size_t size);
    FORCE_INLINE static void    Free(void* ptr);
    FORCE_INLINE static void*   Calloc(size_t nmemb, size_t size);
    FORCE_INLINE static void*   Realloc(void *ptr, size_t size);

    FORCE_INLINE static void*   AlignedMalloc(size_t size, size_t alignment);
    FORCE_INLINE static void    AlignedFree(void *ptr);
    FORCE_INLINE static void*   AlignedCalloc(size_t nmemb, size_t size, size_t alignment);
    FORCE_INLINE static void*   AlignedRealloc(void *ptr, size_t size, size_t alignment);

    FORCE_INLINE static void*   MallocForNew(size_t size);
    FORCE_INLINE static void*   ReallocForNew(void* ptr, size_t size, size_t old);
    FORCE_INLINE static void    FreeForDelete(void* ptr, size_t size);

    FORCE_INLINE static void    ReleaseCacheMemory();
    FORCE_INLINE static void    ReleasePendingBlocks();

    FORCE_INLINE static size_t  SnapSize(size_t size) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static size_t  RegionSize(void* ptr);
#endif
#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static void    DumpMemoryInfo(FWTextWriter& oss);
#endif
};
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_STD)
void* FMallocLowLevel::Malloc(size_t size) { return AlignedMalloc(size, ALLOCATION_BOUNDARY); }
void  FMallocLowLevel::Free(void* ptr) { AlignedFree(ptr); }
void* FMallocLowLevel::Calloc(size_t nmemb, size_t size) { return AlignedCalloc(nmemb, size, ALLOCATION_BOUNDARY); }
void* FMallocLowLevel::Realloc(void *ptr, size_t size) { return AlignedRealloc(ptr, size, ALLOCATION_BOUNDARY); }
void* FMallocLowLevel::AlignedMalloc(size_t size, size_t alignment) {
    alignment = Max(alignment, ALLOCATION_BOUNDARY);
    Assert(alignment == ALLOCATION_BOUNDARY);
    return FPlatformMemory::SystemAlignedMalloc(size, alignment);
}
void  FMallocLowLevel::AlignedFree(void *ptr) {
    FPlatformMemory::SystemAlignedFree(ptr, ALLOCATION_BOUNDARY);
}
void* FMallocLowLevel::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    alignment = Max(alignment, ALLOCATION_BOUNDARY);
    Assert(alignment == ALLOCATION_BOUNDARY);
    void* const p = FPlatformMemory::SystemAlignedMalloc(size * nmemb, alignment);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    alignment = Max(alignment, ALLOCATION_BOUNDARY);
    Assert(alignment == ALLOCATION_BOUNDARY);
    return FPlatformMemory::SystemAlignedRealloc(ptr, size, alignment);
}
void* FMallocLowLevel::MallocForNew(size_t size) { return Malloc(size); }
void* FMallocLowLevel::ReallocForNew(void* ptr, size_t size, size_t) { return Realloc(ptr, size); }
void  FMallocLowLevel::FreeForDelete(void* ptr, size_t) { Free(ptr); }
void  FMallocLowLevel::ReleaseCacheMemory() {}
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT { return size; }
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FPlatformMemory::SystemAlignedRegionSize(ptr, ALLOCATION_BOUNDARY);
}
void FMallocLowLevel::DumpMemoryInfo(FWTextWriter&) {
    // #TODO
}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_STD
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_BINNED1)
void* FMallocLowLevel::Malloc(size_t size) { return FMallocBinned::Malloc(size); }
void  FMallocLowLevel::Free(void* ptr) { FMallocBinned::Free(ptr); }
void* FMallocLowLevel::Calloc(size_t nmemb, size_t size) {
    void* const p = FMallocBinned::Malloc(size * nmemb);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::Realloc(void *ptr, size_t size) { return FMallocBinned::Realloc(ptr, size); }
void* FMallocLowLevel::AlignedMalloc(size_t size, size_t alignment) { return FMallocBinned::AlignedMalloc(size, alignment); }
void  FMallocLowLevel::AlignedFree(void *ptr) { FMallocBinned::AlignedFree(ptr); }
void* FMallocLowLevel::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocBinned::AlignedMalloc(size * nmemb, alignment);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return FMallocBinned::AlignedRealloc(ptr, size, alignment);
}
void* FMallocLowLevel::MallocForNew(size_t size) { return Malloc(size); }
void* FMallocLowLevel::ReallocForNew(void* ptr, size_t size, size_t) { return Realloc(ptr, size); }
void  FMallocLowLevel::FreeForDelete(void* ptr, size_t ) { Free(ptr); }
void  FMallocLowLevel::ReleaseCacheMemory() {
    FMallocBinned::ReleaseCacheMemory();
}
void  FMallocLowLevel::ReleasePendingBlocks() {
    FMallocBinned::ReleasePendingBlocks();
}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT {
    return FMallocBinned::SnapSize(size);
}
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocBinned::RegionSize(ptr);
}
#endif
#if !USE_PPE_FINAL_RELEASE
void FMallocLowLevel::DumpMemoryInfo(FWTextWriter& oss) {
    FMallocBinned::DumpMemoryInfo(oss);
}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_BINNED1
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_BINNED2)
void* FMallocLowLevel::Malloc(size_t size) { return FMallocBinned2::Malloc(size); }
void  FMallocLowLevel::Free(void* ptr) { FMallocBinned2::Free(ptr); }
void* FMallocLowLevel::Calloc(size_t nmemb, size_t size) {
    void* const p = FMallocBinned2::Malloc(size * nmemb);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::Realloc(void *ptr, size_t size) { return FMallocBinned2::Realloc(ptr, size); }
void* FMallocLowLevel::AlignedMalloc(size_t size, size_t alignment) { return FMallocBinned2::AlignedMalloc(size, alignment); }
void  FMallocLowLevel::AlignedFree(void *ptr) { FMallocBinned2::AlignedFree(ptr); }
void* FMallocLowLevel::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocBinned2::AlignedMalloc(size * nmemb, alignment);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return FMallocBinned2::AlignedRealloc(ptr, size, alignment);
}
void* FMallocLowLevel::MallocForNew(size_t size) { return FMallocBinned2::MallocForNew(size); }
void* FMallocLowLevel::ReallocForNew(void* ptr, size_t size, size_t old) { return FMallocBinned2::ReallocForNew(ptr, size, old); }
void  FMallocLowLevel::FreeForDelete(void* ptr, size_t size) { FMallocBinned2::FreeForDelete(ptr, size); }
void  FMallocLowLevel::ReleaseCacheMemory() {
    FMallocBinned2::ReleaseCacheMemory();
}
void  FMallocLowLevel::ReleasePendingBlocks() {
    FMallocBinned2::ReleasePendingBlocks();
}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT {
    return FMallocBinned2::SnapSize(size);
}
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocBinned2::RegionSize(ptr);
}
#endif
#if !USE_PPE_FINAL_RELEASE
void FMallocLowLevel::DumpMemoryInfo(FWTextWriter& oss) {
    FMallocBinned2::DumpMemoryInfo(oss);
}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_BINNED1
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_STOMP)
void* FMallocLowLevel::Malloc(size_t size) { return FMallocStomp::Malloc(size); }
void  FMallocLowLevel::Free(void* ptr) { FMallocStomp::Free(ptr); }
void* FMallocLowLevel::Calloc(size_t nmemb, size_t size) {
    void* const p = FMallocStomp::Malloc(size * nmemb);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::Realloc(void *ptr, size_t size) { return FMallocStomp::Realloc(ptr, size); }
void* FMallocLowLevel::AlignedMalloc(size_t size, size_t alignment) { return FMallocStomp::AlignedMalloc(size, alignment); }
void  FMallocLowLevel::AlignedFree(void *ptr) { FMallocStomp::AlignedFree(ptr); }
void* FMallocLowLevel::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocStomp::AlignedMalloc(size * nmemb, alignment);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return FMallocStomp::AlignedRealloc(ptr, size, alignment);
}
void* FMallocLowLevel::MallocForNew(size_t size) { return Malloc(size); }
void* FMallocLowLevel::ReallocForNew(void* ptr, size_t size, size_t) { return Realloc(ptr, size); }
void  FMallocLowLevel::FreeForDelete(void* ptr, size_t) { Free(ptr); }
void  FMallocLowLevel::ReleaseCacheMemory() {}
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT { return size; }
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocStomp::RegionSize(ptr);
}
#endif
#if !USE_PPE_FINAL_RELEASE
void FMallocLowLevel::DumpMemoryInfo(FWTextWriter&) {/* #TODO */}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_STOMP
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
// Allocation size histogram
//----------------------------------------------------------------------------
#if PPE_MALLOC_HISTOGRAM_PROXY
namespace {
struct FMallocHistogram {
    static constexpr const u32 BlockSizes[123] = {
       0x00000010u,0x00000020u,0x00000030u,0x00000040u,0x00000050u,0x00000060u,0x00000070u,0x00000080u,
       0x00000090u,0x000000b0u,0x000000d0u,0x000000f0u,0x00000110u,0x00000140u,0x00000170u,0x000001a0u,
       0x000001e0u,0x00000220u,0x00000270u,0x000002c0u,0x00000320u,0x000003a0u,0x00000420u,0x000004c0u,
       0x00000590u,0x00000650u,0x00000730u,0x00000840u,0x00000950u,0x00000aa0u,0x00000c70u,0x00000e60u,
       0x00001090u,0x00001330u,0x00001640u,0x00001920u,0x00001cc0u,0x00002120u,0x00002550u,0x00002aa0u,
       0x000031c0u,0x00003aa0u,0x00004240u,0x00004aa0u,0x00005540u,0x00005ff0u,0x00006bf0u,0x00008000u,
       0x00010000u,0x00018000u,0x00020000u,0x00028000u,0x00030000u,0x00038000u,0x00040000u,0x00048000u,
       0x00050000u,0x00058000u,0x00060000u,0x00068000u,0x00070000u,0x00078000u,0x00080000u,0x00088000u,
       0x00090000u,0x00098000u,0x000a0000u,0x000a8000u,0x000b0000u,0x000b8000u,0x000c0000u,0x000c8000u,
       0x000d0000u,0x000d8000u,0x000e0000u,0x000e8000u,0x000f0000u,0x000f8000u,0x00100000u,0x00200000u,
       0x00300000u,0x00400000u,0x00500000u,0x00600000u,0x00700000u,0x00800000u,0x00900000u,0x00a00000u,
       0x00b00000u,0x00c00000u,0x00d00000u,0x00e00000u,0x00f00000u,0x01000000u,0x01100000u,0x01200000u,
       0x01300000u,0x01400000u,0x01500000u,0x01600000u,0x01700000u,0x01800000u,0x01900000u,0x01a00000u,
       0x01b00000u,0x01c00000u,0x01d00000u,0x01e00000u,0x01f00000u,0x02000000u,0x02800000u,0x03000000u,
       0x03800000u,0x04000000u,0x04800000u,0x05000000u,0x05800000u,0x06000000u,0x06800000u,0x07000000u,
       0x07800000u,0x08000000u,0xFFFFFFFFu
    };
    STATIC_CONST_INTEGRAL(u32, NumBlockSizes, lengthof(BlockSizes));

    FMemoryTracking Bins[NumBlockSizes];

    static FMallocHistogram& Get() NOEXCEPT {
        static FMallocHistogram GInstance;
        return GInstance;
    }

    FORCE_INLINE static u32 MakeSizeClass(size_t size) NOEXCEPT {
        const auto it = std::lower_bound(std::begin(BlockSizes), std::end(BlockSizes), checked_cast<u32>(size));
        const u32 sizeClass = checked_cast<u32>(std::distance(std::begin(BlockSizes), it));
        Assert_NoAssume(sizeClass < NumBlockSizes);
        Assert_NoAssume(BlockSizes[sizeClass] >= size);
        return sizeClass;
    }

    void Allocate(size_t userSize, size_t systemSize) NOEXCEPT {
        const u32 sizeClass = MakeSizeClass(systemSize);
        Assert_NoAssume(BlockSizes[sizeClass] >= systemSize);

        FMemoryTracking& bin = Bins[sizeClass];
        bin.Allocate(userSize, systemSize);
    }

    void Deallocate(size_t userSize, size_t systemSize) NOEXCEPT {
        const u32 sizeClass = MakeSizeClass(systemSize);
        Assert_NoAssume(BlockSizes[sizeClass] >= systemSize);

        FMemoryTracking& bin = Bins[sizeClass];
        bin.Deallocate(userSize, systemSize);
    }
};
} //!namespace
#endif //!PPE_MALLOC_HISTOGRAM_PROXY
//----------------------------------------------------------------------------
// Unaccounted allocations logger
//----------------------------------------------------------------------------
#if PPE_MALLOC_UNACCOUNTED_PROXY
struct FMallocUnaccounted {

    FORCE_INLINE static void Allocate(size_t userSize, size_t systemSize) NOEXCEPT {
        if (Unlikely(FMemoryTracking::ThreadTrackingData() == nullptr)) {
            if (userSize == 0)
                userSize = systemSize; // can't retrieve user size in Deallocate()
            MEMORYDOMAIN_TRACKING_DATA(UnaccountedMalloc).Allocate(userSize, systemSize);
        }
    }

    FORCE_INLINE static void Deallocate(size_t userSize, size_t systemSize) NOEXCEPT {
        if (Unlikely(FMemoryTracking::ThreadTrackingData() == nullptr)) {
            if (userSize == 0)
                userSize = systemSize; // can't retrieve user size in Deallocate()
            MEMORYDOMAIN_TRACKING_DATA(UnaccountedMalloc).Deallocate(userSize, systemSize);
        }
    }
};
#endif //!PPE_MALLOC_UNACCOUNTED_PROXY
//----------------------------------------------------------------------------
// Malloc proxy layer with added debug and validation
//----------------------------------------------------------------------------
#if !USE_PPE_MALLOC_PROXY
using FMallocProxy = FMallocLowLevel;
#else
class FMallocProxy {
public:
    enum EFlags : u32 {
        MLP_None        = 0,
        MLP_UserSize    = 1<<0,
        MLP_Realloc     = 1<<1,
    };
    ENUM_FLAGS_FRIEND(EFlags);

    FORCE_INLINE static void* Malloc(size_t size) { return AllocateBlock(FMallocLowLevel::Malloc(size), size); }
    FORCE_INLINE static void  Free(void* ptr) { FMallocLowLevel::Free(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size) { return AllocateBlock(FMallocLowLevel::Calloc(nmemb, size), size); }
    FORCE_INLINE static void* Realloc(void *ptr, size_t size) { return ReallocAllocateBlock(FMallocLowLevel::Realloc(ReallocReleaseBlock(ptr), size), ptr, size); }

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedMalloc(size, alignment), size, alignment); }
    FORCE_INLINE static void  AlignedFree(void *ptr) { FMallocLowLevel::AlignedFree(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedCalloc(nmemb, size, alignment), size, alignment); }
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment) { return ReallocAllocateBlock(FMallocLowLevel::AlignedRealloc(ReallocReleaseBlock(ptr, 0, alignment), size, alignment), ptr , size, alignment); }

    FORCE_INLINE static void* MallocForNew(size_t size) { return AllocateBlock<MLP_UserSize>(FMallocLowLevel::MallocForNew(size), size); }
    FORCE_INLINE static void* ReallocForNew(void* ptr, size_t size, size_t old) { return ReallocAllocateBlock<MLP_UserSize>(FMallocLowLevel::ReallocForNew(ReallocReleaseBlock<MLP_UserSize>(ptr, old), size, old), ptr, size); }
    FORCE_INLINE static void  FreeForDelete(void* ptr, size_t size) { FMallocLowLevel::FreeForDelete(ReleaseBlock<MLP_UserSize>(ptr, size), size); }

    FORCE_INLINE static size_t SnapSize(size_t size) NOEXCEPT { // must always return a block size larger or equal than user input
        const size_t snapped = FMallocLowLevel::SnapSize(size);
        Assert_NoAssume(snapped >= size);
        return snapped;
    }

#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static size_t RegionSize(void* ptr) {
        Assert(ptr);
        return FMallocLowLevel::RegionSize(ptr);
    }
#endif
#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static void DumpMemoryInfo(FWTextWriter& oss) {
        FMallocLowLevel::DumpMemoryInfo(oss);
    }
#endif

private:
    template <EFlags _Flags = MLP_None>
    static void* AllocateBlock(void* ptr, size_t userSize, size_t alignment = ALLOCATION_BOUNDARY) {
        Assert(Meta::IsPow2(alignment));
        Assert_NoAssume(ptr || 0 == userSize);
        if (nullptr == ptr) return nullptr;

        Assert(Meta::IsAlignedPow2(alignment, ptr));
        const size_t systemSize = SnapSize(userSize);
        Unused(systemSize);
        Assert_NoAssume(FMallocLowLevel::RegionSize(ptr) == systemSize);

        IF_CONSTEXPR(not (_Flags & MLP_UserSize)) {
            userSize = systemSize;
        }

#   if PPE_MALLOC_DEBUG_PROXY
        IF_CONSTEXPR(not (_Flags & MLP_Realloc)) {
            PPE_DEBUG_ALLOCATEEVENT(Malloc, ptr, userSize);
        }
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        IF_CONSTEXPR(not (_Flags & MLP_Realloc)) {
            FLeakDetector::Get().Allocate(ptr, userSize);
        }
#   endif
#   if PPE_MALLOC_HISTOGRAM_PROXY
        FMallocHistogram::Get().Allocate(userSize, systemSize);
#   endif
#   if PPE_MALLOC_UNACCOUNTED_PROXY
        FMallocUnaccounted::Allocate(userSize, systemSize);
#   endif
#   if PPE_MALLOC_POISON_PROXY
        IF_CONSTEXPR(not (_Flags & MLP_Realloc)) {
            FPlatformMemory::Memuninitialized(ptr, systemSize);
        }
#   endif
        return ptr;
    }

    template <EFlags _Flags = MLP_None>
    static void* ReleaseBlock(void* ptr, size_t userSize = 0, size_t alignment = ALLOCATION_BOUNDARY) {
        Assert(Meta::IsPow2(alignment));
        Assert(Meta::IsAlignedPow2(alignment, ptr));
        if (nullptr == ptr) return nullptr;

        size_t systemSize;
        IF_CONSTEXPR(_Flags & MLP_UserSize) {
            systemSize = SnapSize(userSize);
            Unused(systemSize);
            Assert_NoAssume(FMallocLowLevel::RegionSize(ptr) == systemSize);
        }
        else {
            Assert_NoAssume(0 == userSize);
            userSize = systemSize = FMallocLowLevel::RegionSize(ptr);
        }

#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_DEALLOCATEEVENT(Malloc, ptr);
#   endif
#   if PPE_MALLOC_POISON_PROXY
        IF_CONSTEXPR(not (_Flags & MLP_Realloc)) {
            FPlatformMemory::Memdeadbeef(ptr, systemSize);
        }
#   endif
#   if PPE_MALLOC_HISTOGRAM_PROXY
        FMallocHistogram::Get().Deallocate(userSize, systemSize);
#   endif
#   if PPE_MALLOC_UNACCOUNTED_PROXY
        FMallocUnaccounted::Deallocate(userSize, systemSize);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        IF_CONSTEXPR(not (_Flags & MLP_Realloc)) {
            FLeakDetector::Get().Release(ptr);
        }
#   endif
        return ptr;
    }

    template <EFlags _Flags = MLP_None>
    static void* ReallocAllocateBlock(void* newp, void* oldp, size_t sizeInBytes, size_t alignment = ALLOCATION_BOUNDARY) {
        Unused(oldp);

#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_REALLOCATEEVENT(Malloc, newp, sizeInBytes, oldp);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Realloc(newp, sizeInBytes, oldp);
#   endif

        return AllocateBlock<_Flags | MLP_Realloc>(newp, sizeInBytes, alignment);
    }

    template <EFlags _Flags = MLP_None>
    static void* ReallocReleaseBlock(void* ptr, size_t size = 0, size_t alignment = ALLOCATION_BOUNDARY) {
        return ReleaseBlock<_Flags | MLP_Realloc>(ptr, size, alignment);
    }
};
#endif //!USE_PPE_MALLOC_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT PPE_DECLSPEC_ALLOCATOR()
void*   (malloc)(size_t size) {
    return (Likely(size) ? FMallocProxy::Malloc(size) : nullptr);
}
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr) {
    if (Likely(ptr))
        FMallocProxy::Free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size) {
    AssertRelease(size);
    return (Likely(nmemb) ? FMallocProxy::Calloc(nmemb, size) : nullptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (realloc)(void *ptr, size_t size) {
    return FMallocProxy::Realloc(ptr, size);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_malloc)(size_t size, size_t alignment) {
    Assert_NoAssume(ALLOCATION_BOUNDARY < alignment);
    return (Likely(size) ? FMallocProxy::AlignedMalloc(size, alignment) : nullptr);
}
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr) {
    if (Likely(ptr))
        FMallocProxy::AlignedFree(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment) {
    AssertRelease(size);
    Assert_NoAssume(ALLOCATION_BOUNDARY < alignment);
    return (Likely(nmemb) ? FMallocProxy::AlignedCalloc(nmemb, size, alignment) : nullptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    Assert_NoAssume(ALLOCATION_BOUNDARY < alignment);
    return FMallocProxy::AlignedRealloc(ptr, size, alignment);
}
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT PPE_DECLSPEC_ALLOCATOR()
void*   (malloc_for_new)(size_t size) {
    return FMallocProxy::MallocForNew(size);
}
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS RESTRICT
void*   (realloc_for_new)(void* ptr, size_t size, size_t old) {
    return FMallocProxy::ReallocForNew(ptr, size, old);
}
//----------------------------------------------------------------------------
PPE_CORE_API NOALIAS
void    (free_for_delete)(void* ptr, size_t size) {
    return FMallocProxy::FreeForDelete(ptr, size);
}
//----------------------------------------------------------------------------
NOALIAS
void    (malloc_release_cache_memory)() {
    FMallocLowLevel::ReleaseCacheMemory();
}
//----------------------------------------------------------------------------
NOALIAS
void    (malloc_release_pending_blocks)() {
    FMallocLowLevel::ReleasePendingBlocks();
}
//----------------------------------------------------------------------------
NOALIAS
size_t  (malloc_snap_size)(size_t size) NOEXCEPT {
    const size_t snapped = FMallocProxy::SnapSize(size);
    Assert_NoAssume((0 == snapped) || (size != 0));
    return snapped;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocDebug::StartLeakDetector() {
#if PPE_MALLOC_LEAKDETECTOR_PROXY
    FLeakDetector::Get().Start();
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocDebug::ShutdownLeakDetector() {
#if PPE_MALLOC_LEAKDETECTOR_PROXY
    FLeakDetector::Get().Shutdown();
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
bool FMallocDebug::SetLeakDetectorWhiteListed(bool ignoreleaks) {
#if PPE_MALLOC_LEAKDETECTOR_PROXY
    bool& whitelistedTLS = FLeakDetector::WhiteListedTLS();
    const bool wasIgnoringLeaks = whitelistedTLS;
    whitelistedTLS = ignoreleaks;
    return wasIgnoringLeaks;
#else
    Unused(ignoreleaks);
    return false;
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocDebug::DumpMemoryLeaks(bool onlyNonDeleters/* = false */) {
#if PPE_MALLOC_LEAKDETECTOR_PROXY
    auto& leakDetector = FLeakDetector::Get();
    leakDetector.ReportLeaks(onlyNonDeleters
        ? FLeakDetector::ReportOnlyNonDeleters
        : FLeakDetector::ReportOnlyLeaks );
#else
    Unused(onlyNonDeleters);
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
bool FMallocDebug::FetchAllocationHistogram(
    TMemoryView<const u32>* sizeClasses,
    TMemoryView<const FMemoryTracking>* bins ) {
#if PPE_MALLOC_HISTOGRAM_PROXY
    if (sizeClasses)
        *sizeClasses = MakeView(FMallocHistogram::BlockSizes);
    if (bins)
        *bins = MakeView(FMallocHistogram::Get().Bins);
    return true;
#else
    Unused(sizeClasses);
    Unused(bins);
    return false;
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocDebug::DumpMemoryInfo(FWTextWriter& oss) {
    FMallocProxy::DumpMemoryInfo(oss);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
size_t FMallocDebug::RegionSize(void* ptr) {
    return FMallocProxy::RegionSize(ptr);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_RESTORE_RUNTIMECHECKS
