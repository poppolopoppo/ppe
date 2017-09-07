#include "stdafx.h"

#include "Malloc.h"
#include "MallocBinned.h"

#ifdef WITH_CORE_ASSERT
#   include "Diagnostic/Callstack.h"
#endif

#include "Meta/Assert.h"

// Lowest level to hook or replace default allocator

#define CORE_MALLOC_FORCE_STD           0 //%_NOCOMMIT%
#define CORE_MALLOC_TRACK_ALLOCATIONS   0 //%_NOCOMMIT%


#define CORE_MALLOC_ALLOCATOR_STD       0
#define CORE_MALLOC_ALLOCATOR_BINNED    1

#if     CORE_MALLOC_FORCE_STD
#   define CORE_MALLOC_ALLOCATOR CORE_MALLOC_ALLOCATOR_STD
#else
#   define CORE_MALLOC_ALLOCATOR CORE_MALLOC_ALLOCATOR_BINNED
#endif

#if (CORE_MALLOC_ALLOCATOR != CORE_MALLOC_ALLOCATOR_STD)
#   ifdef WITH_CORE_ASSERT
#       define CORE_MALLOC_POISON_PROXY 1
#   else
#       define CORE_MALLOC_POISON_PROXY 0
#   endif
#else
#   define CORE_MALLOC_POISON_PROXY 0
#endif

#ifdef WITH_CORE_ASSERT
#   define CORE_MALLOC_LOGGER_PROXY CORE_MALLOC_TRACK_ALLOCATIONS
#else
#   define CORE_MALLOC_LOGGER_PROXY 0
#endif

#define NEED_CORE_MALLOCPROXY (CORE_MALLOC_POISON_PROXY||CORE_MALLOC_LOGGER_PROXY)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FMalloc_ {
    FORCE_INLINE static void* Malloc(size_t size);
    FORCE_INLINE static void  Free(void* ptr);
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size);
    FORCE_INLINE static void* Realloc(void *ptr, size_t size);

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment);
    FORCE_INLINE static void  AlignedFree(void *ptr);
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment);
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment);

    FORCE_INLINE static void  ReleasePendingBlocks();
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
#endif //!CORE_MALLOC_ALLOCATOR_BINNED
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
    STATIC_CONST_INTEGRAL(size_t, BlockOffset, _Base::BlockOffset + _Base::HeaderSize);
    STATIC_CONST_INTEGRAL(size_t, HeaderSize,  _Base::HeaderSize + _HeaderSize);
    STATIC_CONST_INTEGRAL(size_t, FooterSize,  _Base::FooterSize + _FooterSize);
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
struct TMallocPoisonFacet_ : TMallocBaseFacet_<_Pred, 16, 16> {
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
        FCanary* const header = (FCanary*)((u8*)ptr + BlockOffset);
        FCanary* const footer = (FCanary*)((u8*)(header + 1) + sizeInBytes);

        header->MakeCanaries(sizeInBytes);
        footer->MakeCanaries(sizeInBytes);

        pred_type::MakeBlock(ptr, sizeInBytes);
    }

    static void TestBlock(void* ptr) {
        FCanary* const header = (FCanary*)((u8*)ptr + BlockOffset);
        header->CheckCanaries();

        FCanary* const footer = (FCanary*)((u8*)(header + 1) + header->SizeInBytes);
        footer->CheckCanaries();

        header->MakeCanaries(0xdeadbeef);
        footer->MakeCanaries(0xdeadbeef);

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
#if !NEED_CORE_MALLOCPROXY
using FMallocProxy_ = FMalloc_;
#else
using FMallocFacet_ = FMallocPoisonFacet_;
class FMallocProxy_ : private FMallocFacet_ {
    STATIC_CONST_INTEGRAL(size_t, OverheadSize, HeaderSize + FooterSize);

    static void* MakeBlock_(void* ptr, size_t size) {
        if (nullptr == ptr) return nullptr;
        FMallocFacet_::MakeBlock(ptr, size);
        return ((u8*)ptr + FMallocFacet_::HeaderSize);
    }

    static void* TestBlock_(void* ptr) {
        if (nullptr == ptr) return nullptr;
        void* const block = ((u8*)ptr - FMallocFacet_::HeaderSize);
        FMallocFacet_::TestBlock(block);
        return block;
    }

public:
    FORCE_INLINE static void* Malloc(size_t size) { return MakeBlock_(FMalloc_::Malloc(size + OverheadSize), size); }
    FORCE_INLINE static void  Free(void* ptr) { FMalloc_::Free(TestBlock_(ptr)); }
    FORCE_INLINE static void* Calloc(size_t nmemb, size_t size) { return MakeBlock_(FMalloc_::Calloc(nmemb, size + OverheadSize), size); }
    FORCE_INLINE static void* Realloc(void *ptr, size_t size) { return MakeBlock_(FMalloc_::Realloc(TestBlock_(ptr), size + OverheadSize), size); }

    FORCE_INLINE static void* AlignedMalloc(size_t size, size_t alignment) { return MakeBlock_(FMalloc_::AlignedMalloc(size + OverheadSize, alignment), size); }
    FORCE_INLINE static void  AlignedFree(void *ptr) { FMalloc_::AlignedFree(TestBlock_(ptr)); }
    FORCE_INLINE static void* AlignedCalloc(size_t nmemb, size_t size, size_t alignment) { return MakeBlock_(FMalloc_::AlignedCalloc(nmemb, size + OverheadSize, alignment), size); }
    FORCE_INLINE static void* AlignedRealloc(void *ptr, size_t size, size_t alignment) { return MakeBlock_(FMalloc_::AlignedRealloc(TestBlock_(ptr), size + OverheadSize, alignment), size); }

#if CORE_MALLOC_LOGGER_PROXY
    static const FMallocLoggerFacet_::FDebugData* DebugData(void* ptr) {
        void* const block = TestBlock_(ptr);
        return (const FMallocLoggerFacet_::FDebugData*)((u8*)block + FMallocLoggerFacet_::BlockOffset);
    }
#endif
#if CORE_MALLOC_POISON_PROXY
    static const FMallocPoisonFacet_::FCanary* CanaryHeader(void* ptr) {
        void* const block = TestBlock_(ptr);
        return (const FMallocPoisonFacet_::FCanary*)((u8*)block + FMallocPoisonFacet_::BlockOffset);
    }
#endif
};
#endif //!NEED_CORE_MALLOCPROXY
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
NO_INLINE bool FetchMemoryBlockDebugInfos(void* ptr, class FCallstack* pCallstack, size_t* pSizeInBytes, bool raw/* = false */) {
#if CORE_MALLOC_LOGGER_PROXY
    STATIC_ASSERT(CORE_MALLOC_POISON_PROXY);

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
#endif
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
