#include "stdafx.h"

#include "Malloc.h"
#include "MallocBinned.h"
#include "MallocStomp.h"

#include "Diagnostic/LeakDetector.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/Assert.h"

// Lowest level to hook or replace default allocator

#define CORE_MALLOC_ALLOCATOR_STD           0
#define CORE_MALLOC_ALLOCATOR_BINNED        1
#define CORE_MALLOC_ALLOCATOR_STOMP         2

#define CORE_MALLOC_FORCE_STD               0 //%_NOCOMMIT%
#define CORE_MALLOC_FORCE_STOMP             (USE_CORE_MEMORY_DEBUGGING) //%_NOCOMMIT%

#if CORE_MALLOC_FORCE_STD
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_STD
#elif CORE_MALLOC_FORCE_STOMP
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_STOMP
#else
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_BINNED
#endif

#if USE_CORE_MEMORY_DEBUGGING && !CORE_MALLOC_FORCE_STD
#   define CORE_MALLOC_HISTOGRAM_PROXY      1 // Keep memory histogram available, shouldn't have any influence on debugging
#   define CORE_MALLOC_LEAKDETECTOR_PROXY   0 // Disabled since it adds payload to each allocation
#elif not (defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define CORE_MALLOC_HISTOGRAM_PROXY      1 //%_NOCOMMIT% // Logs memory allocation size statistics in an histogram
#   define CORE_MALLOC_LEAKDETECTOR_PROXY   (USE_CORE_MALLOC_LEAKDETECTOR) //%_NOCOMMIT% // Will find leaking code paths
#else
#   define CORE_MALLOC_HISTOGRAM_PROXY      0
#   define CORE_MALLOC_LEAKDETECTOR_PROXY   0
#endif

#define USE_CORE_MALLOC_PROXY               (CORE_MALLOC_HISTOGRAM_PROXY|CORE_MALLOC_LEAKDETECTOR_PROXY)

namespace Core {
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

    FORCE_INLINE static void    ReleasePendingBlocks();

    FORCE_INLINE static size_t  SnapSize(size_t size);

#ifndef FINAL_RELEASE
    FORCE_INLINE static size_t  RegionSize(void* ptr);
#endif
};
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_STD)
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
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) { return size; }
#ifndef FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
#   ifdef PLATFORM_WINDOWS
    return ::_msize(ptr);
#   else
    return 0;
#   endif
#endif
#endif //!CORE_MALLOC_ALLOCATOR_STD
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_BINNED)
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
void  FMallocLowLevel::ReleasePendingBlocks() {
    FMallocBinned::ReleasePendingBlocks();
}
size_t FMallocLowLevel::SnapSize(size_t size) {
    return FMallocBinned::SnapSize(size);
}
#ifndef FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocBinned::RegionSize(ptr);
}
#endif
#endif //!CORE_MALLOC_ALLOCATOR_BINNED
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_STOMP)
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
void  FMallocLowLevel::ReleasePendingBlocks() {}
size_t FMallocLowLevel::SnapSize(size_t size) { return size; }
#ifndef FINAL_RELEASE
size_t FMallocLowLevel::RegionSize(void* ptr) {
    return FMallocStomp::RegionSize(ptr);
}
#endif
#endif //!CORE_MALLOC_ALLOCATOR_STOMP
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if CORE_MALLOC_LEAKDETECTOR_PROXY
namespace {
struct FMallocLeakDetectorFacet {
    static void A(void* ptr, size_t sizeInBytes) {

    }
    static void TestBlock(void* ptr) {
        FLeakDetector::Get().Release(ptr);
    }
};
} //!namespace
#endif //!CORE_MALLOC_LEAKDETECTOR_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if CORE_MALLOC_HISTOGRAM_PROXY
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
static size_t GMallocSizeAllocations[GMallocNumClasses] = { 0 };
static size_t GMallocSizeTotalBytes[GMallocNumClasses] = { 0 };
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
        ++GMallocSizeAllocations[sizeClass];
        GMallocSizeTotalBytes[sizeClass] += sizeInBytes;
    }
};
} //!namespace
#endif //!CORE_MALLOC_HISTOGRAM_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_CORE_MALLOC_PROXY
using FMallocProxy = FMallocLowLevel;
#else
class FMallocProxy {
public:
    FORCE_INLINE static void* Malloc(size_t size) { return AllocateBlock(FMallocLowLevel::Malloc(size), size); }
    FORCE_INLINE static void  Free(void* ptr) { FMallocLowLevel::Free(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size) { return AllocateBlock(FMallocLowLevel::Calloc(nmemb, size), size); }
    FORCE_INLINE static void* Realloc(void *ptr, size_t size) { return AllocateBlock(FMallocLowLevel::Realloc(ReleaseBlock(ptr), size), size); }

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedMalloc(size, alignment), size, alignment); }
    FORCE_INLINE static void  AlignedFree(void *ptr) { FMallocLowLevel::AlignedFree(ReleaseBlock(ptr)); }
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedCalloc(nmemb, size, alignment), size, alignment); }
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment) { return AllocateBlock(FMallocLowLevel::AlignedRealloc(ReleaseBlock(ptr, alignment), size, alignment), size, alignment); }

private:
    static void* AllocateBlock(void* ptr, size_t sizeInBytes, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        if (nullptr == ptr) return nullptr;
        Assert(Meta::IsAligned(alignment, ptr));
#   if CORE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Allocate(ptr, sizeInBytes);
#   endif
#   if CORE_MALLOC_HISTOGRAM_PROXY
        FMallocHistogram::Allocate(ptr, sizeInBytes);
#   endif
        return ptr;
    }

    static void* ReleaseBlock(void* ptr, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        Assert(Meta::IsAligned(alignment, ptr));
        if (nullptr == ptr) return nullptr;
#   if CORE_MALLOC_LEAKDETECTOR_PROXY
        FLeakDetector::Get().Release(ptr);
#   endif
        return ptr;
    }
};
#endif //!USE_CORE_MALLOC_PROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT
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
    return FMallocProxy::AlignedCalloc(nmemb, size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    return FMallocProxy::AlignedRealloc(ptr, size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (malloc_release_pending_blocks)() {
    FMallocLowLevel::ReleasePendingBlocks();
}
//----------------------------------------------------------------------------
NOALIAS
size_t  (malloc_snap_size)(size_t size) {
    return FMallocLowLevel::SnapSize(size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void StartLeakDetector() {
#if CORE_MALLOC_LEAKDETECTOR_PROXY
    FLeakDetector::Get().Start();
#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void ShutdownLeakDetector() {
#if CORE_MALLOC_LEAKDETECTOR_PROXY
    FLeakDetector::Get().Shutdown();
#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
bool SetLeakDetectorWhiteListed(bool ignoreleaks) {
#if CORE_MALLOC_LEAKDETECTOR_PROXY
    bool& whitelistedTLS = FLeakDetector::WhiteListedTLS();
    const bool wasIgnoringLeaks = whitelistedTLS;
    whitelistedTLS = ignoreleaks;
    return wasIgnoringLeaks;
#else
    NOOP(ignoreleaks);
    return false;
#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void DumpMemoryLeaks(bool onlyNonDeleters/* = false */) {
#if CORE_MALLOC_LEAKDETECTOR_PROXY
    auto& leakDetector = FLeakDetector::Get();
    leakDetector.ReportLeaks(onlyNonDeleters);
#else
    NOOP(onlyNonDeleters);
#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
bool FetchMemoryAllocationHistogram(
    TMemoryView<const size_t>* classes,
    TMemoryView<const size_t>* allocations,
    TMemoryView<const size_t>* totalBytes ) {
#if CORE_MALLOC_HISTOGRAM_PROXY
    *classes = MakeView(GMallocSizeClasses);
    *allocations = MakeView(GMallocSizeAllocations);
    *totalBytes = MakeView(GMallocSizeTotalBytes);
    return true;
#else
    NOOP(classes, allocations, totalBytes);
    return false;
#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
