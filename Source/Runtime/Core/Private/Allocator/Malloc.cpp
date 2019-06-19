#include "stdafx.h"

#include "Allocator/Malloc.h"
#include "Allocator/Mallocator.h"
#include "Allocator/MallocBinned.h"
#include "Allocator/MallocStomp.h"

#include "Diagnostic/LeakDetector.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/Assert.h"

// Lowest level to hook or replace default allocator

#define PPE_MALLOC_ALLOCATOR_STD           0
#define PPE_MALLOC_ALLOCATOR_BINNED        1
#define PPE_MALLOC_ALLOCATOR_STOMP         2

#define PPE_MALLOC_FORCE_STD               0 //%_NOCOMMIT%
#define PPE_MALLOC_FORCE_STOMP             (USE_PPE_MEMORY_DEBUGGING) //%_NOCOMMIT%

#if PPE_MALLOC_FORCE_STD
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_STD
#elif PPE_MALLOC_FORCE_STOMP
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_STOMP
#else
#   define PPE_MALLOC_ALLOCATOR            PPE_MALLOC_ALLOCATOR_BINNED
#endif

#if USE_PPE_MEMORY_DEBUGGING && !PPE_MALLOC_FORCE_STD
#   define PPE_MALLOC_DEBUG_PROXY          1
#   define PPE_MALLOC_HISTOGRAM_PROXY      1 // Keep memory histogram available, shouldn't have any influence on debugging
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   0 // Disabled since it adds payload to each allocation
#   define PPE_MALLOC_POISON_PROXY         1 // Erase all data from blocks when allocating and releasing them, helps to find necrophilia
#elif not (USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING)
#   define PPE_MALLOC_DEBUG_PROXY          1
#   define PPE_MALLOC_HISTOGRAM_PROXY      1 // %_NOCOMMIT%
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   (USE_PPE_MALLOC_LEAKDETECTOR) // %_NOCOMMIT%
#   define PPE_MALLOC_POISON_PROXY         (USE_PPE_DEBUG) // %_NOCOMMIT%
#else
#   define PPE_MALLOC_DEBUG_PROXY          0
#   define PPE_MALLOC_HISTOGRAM_PROXY      0
#   define PPE_MALLOC_LEAKDETECTOR_PROXY   0
#   define PPE_MALLOC_POISON_PROXY         0
#endif

#define USE_PPE_MALLOC_PROXY               (PPE_MALLOC_HISTOGRAM_PROXY|PPE_MALLOC_LEAKDETECTOR_PROXY|PPE_MALLOC_POISON_PROXY)

#if PPE_MALLOC_DEBUG_PROXY
#   include "HAL/PlatformDebug.h"
#endif
#if PPE_MALLOC_HISTOGRAM_PROXY
#   include "HAL/PlatformAtomics.h"
#endif

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

    FORCE_INLINE static void    ReleaseCacheMemory();
    FORCE_INLINE static void    ReleasePendingBlocks();

    FORCE_INLINE static size_t  SnapSize(size_t size) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static size_t  RegionSize(void* ptr);
#endif
#if !USE_PPE_FINAL_RELEASE
    FORCE_INLINE static size_t  FetchMediumMips(void** vspace, size_t* numCommited, size_t* numReserved, size_t* mipSizeInBytes, TMemoryView<const u32>* mipMasks);
#endif
};
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_STD)
void* FMallocLowLevel::Malloc(size_t size) { return std::malloc(size); }
void  FMallocLowLevel::Free(void* ptr) { std::free(ptr); }
void* FMallocLowLevel::Calloc(size_t nmemb, size_t size) { return std::calloc(nmemb, size); }
void* FMallocLowLevel::Realloc(void *ptr, size_t size) { return std::realloc(ptr, size); }
void* FMallocLowLevel::AlignedMalloc(size_t size, size_t alignment) { return _aligned_malloc(size, alignment); }
void  FMallocLowLevel::AlignedFree(void *ptr) { _aligned_free(ptr); }
void* FMallocLowLevel::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = _aligned_malloc(size * nmemb, alignment);
    FPlatformMemory::Memzero(p, size * nmemb);
    return p;
}
void* FMallocLowLevel::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return _aligned_realloc(ptr, size, alignment);
}
void  FMallocLowLevel::ReleaseCacheMemory() {}
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT { return size; }
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
#   ifdef PLATFORM_WINDOWS
    return ::_msize(ptr);
#   else
    return 0;
#   endif
#endif
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::FetchMediumMips(void**, size_t*, size_t*, size_t*, TMemoryView<const u32>*) {
    return false; // unsupported
}
#endif
}
#endif //!PPE_MALLOC_ALLOCATOR_STD
//----------------------------------------------------------------------------
#if (PPE_MALLOC_ALLOCATOR == PPE_MALLOC_ALLOCATOR_BINNED)
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
size_t FMallocLowLevel::FetchMediumMips(void** vspace, size_t* numCommited, size_t* numReserved, size_t* mipSizeInBytes, TMemoryView<const u32>* mipMasks) {
    return FMallocBinned::FetchMediumMips(vspace, numReserved, numCommited, mipSizeInBytes, mipMasks);
}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_BINNED
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
void  FMallocLowLevel::ReleaseCacheMemory() {}
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) NOEXCEPT { return size; }
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocStomp::RegionSize(ptr);
}
#endif
#if !USE_PPE_FINAL_RELEASE
size_t FMallocLowLevel::FetchMediumMips(void**, size_t*, size_t*, size_t*, TMemoryView<const u32>*) {
    return false; // unsupported
}
#endif
#endif //!PPE_MALLOC_ALLOCATOR_STOMP
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if PPE_MALLOC_HISTOGRAM_PROXY
namespace {
static constexpr size_t GMallocNumClasses = 60;
static constexpr size_t GMallocSizeClasses[GMallocNumClasses] = {
    16,       0,        0,        0,        32,       0,
    48,       0,        64,       80,       96,       112,
    128,      160,      192,      224,      256,      320,
    384,      448,      512,      640,      768,      896,
    1024,     1280,     1536,     1792,     2048,     2560,
    3072,     3584,     4096,     5120,     6144,     7168,
    8192,     10240,    12288,    14336,    16384,    20480,
    24576,    28672,    32768,    40960,    49152,    57344,
    65536,    81920,    98304,    114688,   131072,   163840,
    196608,   229376,   262144,   327680,   393216,   425952,
};
static volatile i64 GMallocSizeAllocations[GMallocNumClasses] = { 0 };
static volatile i64 GMallocSizeTotalBytes[GMallocNumClasses] = { 0 };
struct FMallocHistogram {
    FORCE_INLINE static size_t MakeSizeClass(size_t size) {
        constexpr size_t POW_N = 2;
        constexpr size_t MinClassIndex = 19;
        const size_t index = FPlatformMaths::FloorLog2((size - 1) | 1);
        return ((index << POW_N) + ((size - 1) >> (index - POW_N)) - MinClassIndex);
    }

    static void Allocate(void* ptr, size_t sizeInBytes) {
        NOOP(ptr);
        const size_t sizeClass = Min(MakeSizeClass(sizeInBytes), GMallocNumClasses - 1);
        FPlatformAtomics::Increment(&GMallocSizeAllocations[sizeClass]);
        FPlatformAtomics::Add(&GMallocSizeTotalBytes[sizeClass], sizeInBytes);
    }
};
} //!namespace
#endif //!PPE_MALLOC_HISTOGRAM_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_PPE_MALLOC_PROXY
using FMallocProxy = FMallocLowLevel;
#else
class FMallocProxy {
public:
    FORCE_INLINE static void* Malloc(size_t size) { return AllocateBlock(FMallocLowLevel::Malloc(size), size); }
    FORCE_INLINE static void  Free(void* ptr) { FMallocLowLevel::Free(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size) { return AllocateBlock(FMallocLowLevel::Calloc(nmemb, size), size); }
    FORCE_INLINE static void* Realloc(void *ptr, size_t size) { return ReallocAllocateBlock(FMallocLowLevel::Realloc(ReallocReleaseBlock(ptr), size), ptr, size); }

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedMalloc(size, alignment), size, alignment); }
    FORCE_INLINE static void  AlignedFree(void *ptr) { FMallocLowLevel::AlignedFree(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedCalloc(nmemb, size, alignment), size, alignment); }
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment) { return ReallocAllocateBlock(FMallocLowLevel::AlignedRealloc(ReallocReleaseBlock(ptr, alignment), size, alignment), ptr , size, alignment); }

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
    FORCE_INLINE static size_t FetchMediumMips(void** vspace, size_t* numCommited, size_t* numReserved, size_t* mipSizeInBytes, TMemoryView<const u32>* mipMasks) {
        return FMallocLowLevel::FetchMediumMips(vspace, numCommited, numReserved, mipSizeInBytes, mipMasks);
    }
#endif

private:
    static void* AllocateBlock(void* ptr, size_t sizeInBytes, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        if (nullptr == ptr) return nullptr;
        Assert(Meta::IsAligned(alignment, ptr));
        Assert_NoAssume(FMallocLowLevel::RegionSize(ptr) == SnapSize(sizeInBytes));
#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_ALLOCATEEVENT(Malloc, ptr, sizeInBytes);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Allocate(ptr, sizeInBytes);
#   endif
#   if PPE_MALLOC_HISTOGRAM_PROXY
        FMallocHistogram::Allocate(ptr, sizeInBytes);
#   endif
#   if PPE_MALLOC_POISON_PROXY
        FPlatformMemory::Memset(ptr, 0xCC, sizeInBytes);
#   endif
        return ptr;
    }

    static void* ReleaseBlock(void* ptr, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        Assert(Meta::IsAligned(alignment, ptr));
        if (nullptr == ptr) return nullptr;
#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_DEALLOCATEEVENT(Malloc, ptr);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Release(ptr);
#   endif
#   if PPE_MALLOC_POISON_PROXY
        FPlatformMemory::Memset(ptr, 0xDD, FMallocLowLevel::RegionSize(ptr));
#   endif
        return ptr;
    }

    static void* ReallocAllocateBlock(void* newp, void* oldp, size_t sizeInBytes, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        Assert_NoAssume(newp || 0 == sizeInBytes);
        if (nullptr == newp) return nullptr;
        Assert(Meta::IsAligned(alignment, newp));
        Assert_NoAssume(FMallocLowLevel::RegionSize(newp) == SnapSize(sizeInBytes));
#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_REALLOCATEEVENT(Malloc, newp, sizeInBytes, oldp);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Allocate(newp, sizeInBytes);
#   endif
#   if PPE_MALLOC_HISTOGRAM_PROXY
        FMallocHistogram::Allocate(newp, sizeInBytes);
#   endif
#   if PPE_MALLOC_POISON_PROXY
        if (nullptr == oldp)
            FPlatformMemory::Memset(newp, 0xCC, sizeInBytes);
#   else
        UNUSED(oldp);
#   endif
        return newp;
    }

    static void* ReallocReleaseBlock(void* ptr, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        Assert(Meta::IsAligned(alignment, ptr));
        if (nullptr == ptr) return nullptr;
#   if PPE_MALLOC_DEBUG_PROXY
        PPE_DEBUG_DEALLOCATEEVENT(Malloc, ptr);
#   endif
#   if PPE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Release(ptr);
#   endif
        return ptr;
    }
};
#endif //!USE_PPE_MALLOC_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT PPE_DECLSPEC_ALLOCATOR()
void*   (malloc)(size_t size) {
    return FMallocProxy::Malloc(size);
}
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr) {
    return FMallocProxy::Free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size) {
    return FMallocProxy::Calloc(nmemb, size);
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
    return FMallocProxy::AlignedMalloc(size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr) {
    FMallocProxy::AlignedFree(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment) {
    Assert_NoAssume(ALLOCATION_BOUNDARY < alignment);
    return FMallocProxy::AlignedCalloc(nmemb, size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    Assert_NoAssume(ALLOCATION_BOUNDARY < alignment);
    return FMallocProxy::AlignedRealloc(ptr, size, alignment);
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
    return FMallocProxy::SnapSize(size);
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
    NOOP(ignoreleaks);
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
    NOOP(onlyNonDeleters);
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
bool FMallocDebug::FetchAllocationHistogram(
    TMemoryView<const size_t>* classes,
    TMemoryView<const i64>* allocations,
    TMemoryView<const i64>* totalBytes ) {
#if PPE_MALLOC_HISTOGRAM_PROXY
    *classes = MakeView(GMallocSizeClasses);
    *allocations = TMemoryView<const i64>((const i64*)&GMallocSizeAllocations, lengthof(GMallocSizeAllocations));
    *totalBytes = TMemoryView<const i64>((const i64*)&GMallocSizeTotalBytes, lengthof(GMallocSizeTotalBytes));
    return true;
#else
    NOOP(classes, allocations, totalBytes);
    return false;
#endif
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
bool FMallocDebug::FetchMediumMips(
    void** vspace,
    size_t* numCommited,
    size_t* numReserved,
    size_t* mipSizeInBytes,
    TMemoryView<const u32>* mipMasks ) {
    return FMallocProxy::FetchMediumMips(vspace, numReserved, numCommited, mipSizeInBytes, mipMasks);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
