#include "stdafx.h"

#include "Malloc.h"
#include "MallocBinned.h"
#include "MallocStomp.h"

#ifdef WITH_CORE_ASSERT
#   include "Diagnostic/Callstack.h"
#   include "Memory/MemoryView.h"
#endif

#include "Meta/Assert.h"

// Lowest level to hook or replace default allocator

#define CORE_MALLOC_ALLOCATOR_STD           0
#define CORE_MALLOC_ALLOCATOR_BINNED        1
#define CORE_MALLOC_ALLOCATOR_STOMP         2

#define CORE_MALLOC_FORCE_STD               0 //%_NOCOMMIT%
#define CORE_MALLOC_FORCE_STOMP             USE_CORE_MEMORY_DEBUGGING //%_NOCOMMIT%

#if CORE_MALLOC_FORCE_STD
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_STD
#elif CORE_MALLOC_FORCE_STOMP
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_STOMP
#else
#   define CORE_MALLOC_ALLOCATOR            CORE_MALLOC_ALLOCATOR_BINNED
#endif

#if USE_CORE_MEMORY_DEBUGGING
#   define CORE_MALLOC_HISTOGRAM_PROXY      1 // Keep memory histogram available, shouldn't have any influence on debugging
#   define CORE_MALLOC_LOGGER_PROXY         0 // Disabled since it adds payload to each allocation
#elif not (defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define CORE_MALLOC_HISTOGRAM_PROXY      1 //%_NOCOMMIT% // Logs memory statictics in a histogram
#   define CORE_MALLOC_LOGGER_PROXY         0 //%_NOCOMMIT% // Turn on locally to get infos about leaked memory blocks
#else
#   define CORE_MALLOC_HISTOGRAM_PROXY      0
#   define CORE_MALLOC_LOGGER_PROXY         0
#endif

#ifdef WITH_CORE_ASSERT
#   define CORE_MALLOC_POISON_PROXY         (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_BINNED) // other allocators have their own poisoning system
#else
#   define CORE_MALLOC_POISON_PROXY         0
#endif

#define NEED_CORE_MALLOCPROXY               (CORE_MALLOC_HISTOGRAM_PROXY|CORE_MALLOC_POISON_PROXY|CORE_MALLOC_LOGGER_PROXY)

#if CORE_MALLOC_LOGGER_PROXY
#   include "Diagnostic/Callstack.h"
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FMalloc_ {
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
void* FMalloc_::Malloc(size_t size) { return std::malloc(size); }
void  FMalloc_::Free(void* ptr) { std::free(ptr); }
void* FMalloc_::Calloc(size_t nmemb, size_t size) { return std::calloc(nmemb, size); }
void* FMalloc_::Realloc(void *ptr, size_t size) { return std::realloc(ptr, size); }
void* FMalloc_::AlignedMalloc(size_t size, size_t alignment) { return _aligned_malloc(size, alignment); }
void  FMalloc_::AlignedFree(void *ptr) { _aligned_free(ptr); }
void* FMalloc_::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = _aligned_malloc(size * nmemb, alignment);
    ::memset(p, 0, size * nmemb);
    return p;
}
void* FMalloc_::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return _aligned_realloc(ptr, size, alignment);
}
void  FMalloc_::ReleasePendingBlocks() {}
size_t FMalloc_::SnapSize(size_t size) { return size; }
size_t FMalloc_::RegionSize(void* ptr) {
#   ifdef PLATFORM_WINDOWS
    return ::_msize(ptr);
#   else
    return 0;
#   endif
}
#endif //!CORE_MALLOC_ALLOCATOR_STD
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_BINNED)
void* FMalloc_::Malloc(size_t size) { return FMallocBinned::Malloc(size); }
void  FMalloc_::Free(void* ptr) { FMallocBinned::Free(ptr); }
void* FMalloc_::Calloc(size_t nmemb, size_t size) {
    void* const p = FMallocBinned::Malloc(size * nmemb);
    ::memset(p, 0, size * nmemb);
    return p;
}
void* FMalloc_::Realloc(void *ptr, size_t size) { return FMallocBinned::Realloc(ptr, size); }
void* FMalloc_::AlignedMalloc(size_t size, size_t alignment) { return FMallocBinned::AlignedMalloc(size, alignment); }
void  FMalloc_::AlignedFree(void *ptr) { FMallocBinned::AlignedFree(ptr); }
void* FMalloc_::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocBinned::AlignedMalloc(size * nmemb, alignment);
    ::memset(p, 0, size * nmemb);
    return p;
}
void* FMalloc_::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return FMallocBinned::AlignedRealloc(ptr, size, alignment);
}
void  FMalloc_::ReleasePendingBlocks() {
    FMallocBinned::ReleasePendingBlocks();
}
size_t FMalloc_::SnapSize(size_t size) {
    return FMallocBinned::SnapSize(size);
}
size_t FMalloc_::RegionSize(void* ptr) {
    return FMallocBinned::RegionSize(ptr);
}
#endif //!CORE_MALLOC_ALLOCATOR_BINNED
//----------------------------------------------------------------------------
#if (CORE_MALLOC_ALLOCATOR == CORE_MALLOC_ALLOCATOR_STOMP)
void* FMalloc_::Malloc(size_t size) { return FMallocStomp::Malloc(size); }
void  FMalloc_::Free(void* ptr) { FMallocStomp::Free(ptr); }
void* FMalloc_::Calloc(size_t nmemb, size_t size) {
    void* const p = FMallocStomp::Malloc(size * nmemb);
    ::memset(p, 0, size * nmemb);
    return p;
}
void* FMalloc_::Realloc(void *ptr, size_t size) { return FMallocStomp::Realloc(ptr, size); }
void* FMalloc_::AlignedMalloc(size_t size, size_t alignment) { return FMallocStomp::AlignedMalloc(size, alignment); }
void  FMalloc_::AlignedFree(void *ptr) { FMallocStomp::AlignedFree(ptr); }
void* FMalloc_::AlignedCalloc(size_t nmemb, size_t size, size_t alignment) {
    void* const p = FMallocStomp::AlignedMalloc(size * nmemb, alignment);
    ::memset(p, 0, size * nmemb);
    return p;
}
void* FMalloc_::AlignedRealloc(void *ptr, size_t size, size_t alignment) {
    return FMallocStomp::AlignedRealloc(ptr, size, alignment);
}
void  FMalloc_::ReleasePendingBlocks() {}
size_t FMalloc_::SnapSize(size_t size) { return size; }
size_t FMalloc_::RegionSize(void* ptr) {
    return FMallocStomp::RegionSize(ptr);
}
#endif //!CORE_MALLOC_ALLOCATOR_STOMP
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if NEED_CORE_MALLOCPROXY
namespace {
//----------------------------------------------------------------------------
struct FMallocFacetId_ {
    STATIC_CONST_INTEGRAL(size_t, BlockOffset,  0);
    STATIC_CONST_INTEGRAL(size_t, HeaderSize,   0);
    STATIC_CONST_INTEGRAL(size_t, FooterSize,   0);

    static void MakeBlock(void* ptr, size_t sizeInBytes) { UNUSED(ptr); UNUSED(sizeInBytes); }
    static void TestBlock(void* ptr) { UNUSED(ptr); }
};
//----------------------------------------------------------------------------
template <typename _Base, size_t _HeaderSize, size_t _FooterSize>
struct TMallocBaseFacet_ {
    STATIC_CONST_INTEGRAL(size_t, ThisHeaderSize, _HeaderSize);
    STATIC_CONST_INTEGRAL(size_t, ThisFooterSize, _FooterSize);

    STATIC_CONST_INTEGRAL(size_t, BlockOffset, _Base::BlockOffset + _Base::HeaderSize);
    STATIC_CONST_INTEGRAL(size_t, HeaderSize,  _Base::HeaderSize + ThisHeaderSize);
    STATIC_CONST_INTEGRAL(size_t, FooterSize,  _Base::FooterSize + ThisFooterSize);
};
//----------------------------------------------------------------------------
} //!namespace
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if NEED_CORE_MALLOCPROXY
namespace {
//----------------------------------------------------------------------------
#if CORE_MALLOC_LOGGER_PROXY
template <typename _Pred>
struct TMallocLoggerFacet_ : TMallocBaseFacet_<_Pred, 16 * sizeof(void*), 0> {
    typedef _Pred pred_type;

    struct FDebugData {
        STATIC_CONST_INTEGRAL(size_t, MaxDepth, 16);
        void* Frames[MaxDepth];
    };

    static void MakeBlock(void* ptr, size_t sizeInBytes) {
        FDebugData* const debugData = (FDebugData*)((u8*)ptr + BlockOffset);
        const size_t depth = FCallstack::Capture(debugData->Frames, nullptr, 4, FDebugData::MaxDepth);

        // null terminate the frames IFN to deduce its size later
        if (depth < FDebugData::MaxDepth)
            debugData->Frames[depth] = nullptr;

        pred_type::MakeBlock(ptr, sizeInBytes);
    }

    static void TestBlock(void* ptr) {
        pred_type::TestBlock(ptr);
    }
};
#endif
//----------------------------------------------------------------------------
#if CORE_MALLOC_LOGGER_PROXY
typedef TMallocLoggerFacet_<FMallocFacetId_> FMallocLoggerFacet_;
#else
typedef FMallocFacetId_ FMallocLoggerFacet_;
#endif //!_DEBUG
//----------------------------------------------------------------------------
} //!namespace
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if NEED_CORE_MALLOCPROXY
namespace {
//----------------------------------------------------------------------------
#if CORE_MALLOC_POISON_PROXY
template <typename _Pred = FMallocFacetId_>
struct TMallocPoisonFacet_ : TMallocBaseFacet_<_Pred, 64/* should preserve alignment */, 16> {
    typedef _Pred pred_type;

    static u32 MakeCanary_(const u32 seed, const void* p) {
        return u32(hash_size_t_constexpr(seed, size_t(p)));
    }

    STATIC_CONST_INTEGRAL(size_t, CanarySize, 16);

    struct FCanary {
        u32 SizeInBytes;
        u32 A, B, C;

        void MakeCanaries(size_t sizeInBytes) {
            SizeInBytes = u32(sizeInBytes);
            A = MakeCanary_(SizeInBytes, &A);
            B = MakeCanary_(SizeInBytes, &B);
            C = MakeCanary_(SizeInBytes, &C);
        }

        void CheckCanaries() const {
            Assert(A == MakeCanary_(SizeInBytes, &A));
            Assert(B == MakeCanary_(SizeInBytes, &B));
            Assert(C == MakeCanary_(SizeInBytes, &C));
        }
    };
    STATIC_ASSERT(sizeof(FCanary) == CanarySize);

    static void MakeBlock(void* ptr, size_t sizeInBytes) {
        u8* const pdata = (u8*)ptr + BlockOffset;

        ::memset(pdata, 0xEE, ThisHeaderSize - sizeof(FCanary));

        FCanary* const header = (FCanary*)(pdata + ThisHeaderSize - sizeof(FCanary));
        FCanary* const footer = (FCanary*)(pdata + ThisHeaderSize + sizeInBytes);

        header->MakeCanaries(sizeInBytes);
        footer->MakeCanaries(sizeInBytes);

        pred_type::MakeBlock(ptr, sizeInBytes);
    }

    static void TestBlock(void* ptr) {
        u8* const pdata = (u8*)ptr + BlockOffset;

        FCanary* const header = (FCanary*)(pdata + ThisHeaderSize - sizeof(FCanary));
        header->CheckCanaries();

        FCanary* const footer = (FCanary*)(pdata + ThisHeaderSize + header->SizeInBytes);
        footer->CheckCanaries();

        header->MakeCanaries(0xdeadbeef);
        footer->MakeCanaries(0xdeadbeef);

        ::memset(pdata, 0xDD, ThisHeaderSize - sizeof(FCanary));

        pred_type::TestBlock(ptr);
    }
};
#endif
//----------------------------------------------------------------------------
#if CORE_MALLOC_POISON_PROXY
typedef TMallocPoisonFacet_<FMallocLoggerFacet_> FMallocPoisonFacet_;
#else
typedef FMallocLoggerFacet_ FMallocPoisonFacet_;
#endif //!_DEBUG
//----------------------------------------------------------------------------
} //!namespace
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if NEED_CORE_MALLOCPROXY
namespace {
//----------------------------------------------------------------------------
#if CORE_MALLOC_HISTOGRAM_PROXY
template <typename _Pred = FMallocFacetId_>
struct TMallocHistogramFacet_ : TMallocBaseFacet_<_Pred, 0, 0> {
    typedef _Pred pred_type;

    static constexpr size_t GNumClasses = 60;
    static constexpr size_t GSizeClasses[GNumClasses] = {
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

    static size_t GSizeAllocations[GNumClasses];
    static size_t GSizeTotalBytes[GNumClasses];

    FORCE_INLINE static size_t MakeClass(size_t size) {
        constexpr size_t POW_N = 2;
        constexpr size_t MinClassIndex = 19;
        const size_t index = Meta::FloorLog2((size - 1) | 1);
        return ((index << POW_N) + ((size - 1) >> (index - POW_N)) - MinClassIndex);
    }

    static void MakeBlock(void* ptr, size_t sizeInBytes) {
        const size_t sizeClass = Min(MakeClass(sizeInBytes), GNumClasses - 1);
        ++GSizeAllocations[sizeClass];
        GSizeTotalBytes[sizeClass] += sizeInBytes;

        pred_type::MakeBlock(ptr, sizeInBytes);
    }

    static void TestBlock(void* ptr) {
        pred_type::TestBlock(ptr);
    }
};
template <typename _Pred>
size_t TMallocHistogramFacet_<_Pred>::GSizeAllocations[GNumClasses] = { 0 };
template <typename _Pred>
size_t TMallocHistogramFacet_<_Pred>::GSizeTotalBytes[GNumClasses] = { 0 };
#endif
//----------------------------------------------------------------------------
#if CORE_MALLOC_HISTOGRAM_PROXY
typedef TMallocHistogramFacet_<FMallocPoisonFacet_> FMallocHistogramFacet_;
#else
typedef FMallocPoisonFacet_ FMallocHistogramFacet_;
#endif //!_DEBUG
//----------------------------------------------------------------------------
} //!namespace
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !NEED_CORE_MALLOCPROXY
using FMallocProxy_ = FMalloc_;
#else
using FMallocFacet_ = FMallocHistogramFacet_;
class FMallocProxy_ {
    STATIC_CONST_INTEGRAL(size_t, OverheadSize, FMallocFacet_::HeaderSize + FMallocFacet_::FooterSize);

    static void* MakeBlock(void* ptr, size_t size, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        if (nullptr == ptr) return nullptr;
        FMallocFacet_::MakeBlock(ptr, size);
        void* const userland = ((u8*)ptr + FMallocFacet_::HeaderSize);
        Assert(Meta::IsAligned(alignment, userland));
        return userland;
    }

    static void* TestBlock(void* userland, size_t alignment = 16) {
        Assert(Meta::IsPow2(alignment));
        if (nullptr == userland) return nullptr;
        Assert(Meta::IsAligned(alignment, userland));
        void* const ptr = ((u8*)userland - FMallocFacet_::HeaderSize);
        FMallocFacet_::TestBlock(ptr);
        return ptr;
    }

public:
    FORCE_INLINE static void* Malloc(size_t size) { return MakeBlock(FMalloc_::Malloc(size + OverheadSize), size); }
    FORCE_INLINE static void  Free(void* ptr) { FMalloc_::Free(TestBlock(ptr)); }
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size) { return MakeBlock(FMalloc_::Calloc(nmemb, size + OverheadSize), size); }
    FORCE_INLINE static void* Realloc(void *ptr, size_t size) { return MakeBlock(FMalloc_::Realloc(TestBlock(ptr), size + OverheadSize), size); }

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment) { return MakeBlock(FMalloc_::AlignedMalloc(size + OverheadSize, alignment), size, alignment); }
    FORCE_INLINE static void  AlignedFree(void *ptr) { FMalloc_::AlignedFree(TestBlock(ptr)); }
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment) { return MakeBlock(FMalloc_::AlignedCalloc(nmemb, size + OverheadSize, alignment), size, alignment); }
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment) { return MakeBlock(FMalloc_::AlignedRealloc(TestBlock(ptr, alignment), size + OverheadSize, alignment), size, alignment); }

#if CORE_MALLOC_LOGGER_PROXY
    static const FMallocLoggerFacet_::FDebugData* DebugData(void* ptr) {
        void* const block = TestBlock(ptr);
        return (const FMallocLoggerFacet_::FDebugData*)((u8*)block + FMallocLoggerFacet_::BlockOffset);
    }
#endif
#if CORE_MALLOC_POISON_PROXY
    static const FMallocPoisonFacet_::FCanary* CanaryHeader(void* ptr) {
        void* const block = TestBlock(ptr);
        return (const FMallocPoisonFacet_::FCanary*)((u8*)block + FMallocPoisonFacet_::BlockOffset);
    }
#endif
};
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
NO_INLINE bool FetchMemoryBlockDebugInfos(void* ptr, class FCallstack* pCallstack, size_t* pSizeInBytes, bool raw/* = false */) {
#if CORE_MALLOC_LOGGER_PROXY && CORE_MALLOC_POISON_PROXY
    if (nullptr == ptr)
        return false;

    Assert(pCallstack || pSizeInBytes);

    const FMallocLoggerFacet_::FDebugData* debugData;

    const FMallocPoisonFacet_::FCanary* canary;

    if (raw) {
        debugData = (FMallocLoggerFacet_::FDebugData*)((u8*)ptr + FMallocLoggerFacet_::BlockOffset);
        canary = (FMallocPoisonFacet_::FCanary*)((u8*)ptr + FMallocPoisonFacet_::BlockOffset);
    }
    else {
        debugData = FMallocProxy_::DebugData(ptr);
        canary = FMallocProxy_::CanaryHeader(ptr);
    }

    canary->CheckCanaries();

    if (pCallstack)
        pCallstack->SetFrames(debugData->Frames);
    if (pSizeInBytes)
        *pSizeInBytes = canary->SizeInBytes;

    return true;

#else
    UNUSED(ptr);
    UNUSED(pCallstack);
    UNUSED(pSizeInBytes);
    return false;

#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
bool FetchMemoryAllocationHistogram(
    TMemoryView<const size_t>* classes,
    TMemoryView<const size_t>* allocations,
    TMemoryView<const size_t>* totalBytes ) {
#if CORE_MALLOC_HISTOGRAM_PROXY && CORE_MALLOC_POISON_PROXY
    *classes = MakeView(FMallocHistogramFacet_::GSizeClasses);
    *allocations = MakeView(FMallocHistogramFacet_::GSizeAllocations);
    *totalBytes = MakeView(FMallocHistogramFacet_::GSizeTotalBytes);
    return true;

#else
    return false;

#endif
}
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (malloc)(size_t size) {
    return FMallocProxy_::Malloc(size);
}
//----------------------------------------------------------------------------
NOALIAS
void    (free)(void *ptr) {
    return FMallocProxy_::Free(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (calloc)(size_t nmemb, size_t size) {
    return FMallocProxy_::Calloc(nmemb, size);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (realloc)(void *ptr, size_t size) {
    return FMallocProxy_::Realloc(ptr, size);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_malloc)(size_t size, size_t alignment) {
    return FMallocProxy_::AlignedMalloc(size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (aligned_free)(void *ptr) {
    FMallocProxy_::AlignedFree(ptr);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_calloc)(size_t nmemb, size_t size, size_t alignment) {
    return FMallocProxy_::AlignedCalloc(nmemb, size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS RESTRICT
void*   (aligned_realloc)(void *ptr, size_t size, size_t alignment) {
    return FMallocProxy_::AlignedRealloc(ptr, size, alignment);
}
//----------------------------------------------------------------------------
NOALIAS
void    (malloc_release_pending_blocks)() {
    FMalloc_::ReleasePendingBlocks();
}
//----------------------------------------------------------------------------
NOALIAS
size_t  (malloc_snap_size)(size_t size) {
    return FMalloc_::SnapSize(size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
