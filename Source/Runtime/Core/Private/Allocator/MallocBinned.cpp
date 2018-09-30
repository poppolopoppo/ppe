#include "stdafx.h"

#include "Allocator/MallocBinned.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"

#ifdef WITH_PPE_ASSERT
#   include "Diagnostic/Callstack.h"
#   include "Diagnostic/DecodedCallstack.h"
#endif
#ifdef USE_DEBUG_LOGGER
#   include "IO/TextWriter.h"
#   include "IO/FormatHelpers.h"
#   include "Thread/ThreadContext.h"
#endif

#if (defined(WITH_PPE_ASSERT) || USE_PPE_MEMORY_DEBUGGING)
#   define USE_MALLOCBINNED_PAGE_PROTECT    1// Crash when using a cached block
#else
#   define USE_MALLOCBINNED_PAGE_PROTECT    0// Crash when using a cached block
#endif

// for medium > large block size allocations + used as FBinnedChunk allocator (*WAY* faster)
#define USE_MALLOCBINNED_MIPMAPS 1 //%_NOCOMMIT%
#if USE_MALLOCBINNED_MIPMAPS
//  reserved virtual size for mip maps (not everything is consumed systematically)
#   define PPE_MIPMAPS_RESERVEDSIZE (CODE3264(512, 1024) * 1024 * 1024) // 512/1024mo
#   include "Allocator/MallocMipMap.h"
#endif

PRAGMA_INITSEG_COMPILER

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, MallocBinned)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(64 == CACHELINE_SIZE);
//----------------------------------------------------------------------------
struct FBinnedBlock_;
struct FBinnedChunk_;
struct FBinnedThreadCache_;
struct FBinnedGlobalCache_;
//----------------------------------------------------------------------------
static FORCE_INLINE size_t MakeBinnedClass_(size_t size) NOEXCEPT {
    constexpr size_t POW_N = 2;
    size = ROUND_TO_NEXT_16(size);
    const size_t index = FPlatformMaths::FloorLog2((size - 1) | 1);
    return ((index << POW_N) + ((size - 1) >> (index - POW_N)) - FMallocBinned::MinSizeClass);
}
//----------------------------------------------------------------------------
// FBinnedBlock_
//----------------------------------------------------------------------------
struct FBinnedBlock_ {
    FBinnedBlock_* Next;
};
//----------------------------------------------------------------------------
// FBinnedChunk_
//----------------------------------------------------------------------------
static void* AllocateNewBinnedChunk_();
static void DeallocateBinnedChunk_(void* p);
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedChunk_ {
    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);
    STATIC_CONST_INTEGRAL(size_t, MinSizeInBytes, ALLOCATION_BOUNDARY);
    STATIC_CONST_INTEGRAL(size_t, MaxSizeInBytes, 32768);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeInBytes, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeMask, ~(ChunkSizeInBytes - 1));
    STATIC_CONST_INTEGRAL(size_t, ChunkAvailableSizeInBytes, ChunkSizeInBytes - CACHELINE_SIZE);

    STATIC_ASSERT(Meta::IsAligned(Alignment, MinSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, MaxSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, ChunkSizeInBytes));
    STATIC_ASSERT(Meta::IsAligned(Alignment, ChunkAvailableSizeInBytes));

    FBinnedThreadCache_* ThreadCache;
    FAtomicSpinLock ThreadBarrier;

#ifdef WITH_PPE_ASSERT
    STATIC_CONST_INTEGRAL(u32, DefaultCanary, 0x4110c470ul);
    u32 _Canary;
    bool CheckCanary() const {
        Assert_NoAssume(Meta::IsAligned(FBinnedChunk_::ChunkSizeInBytes, this));
        return (DefaultCanary == _Canary);
    }
    static constexpr size_t NumBlocksTotalInChunk(size_t sizeClass) {
        return (FBinnedChunk_::ChunkAvailableSizeInBytes / FMallocBinned::SizeClasses[sizeClass]);
    }
#endif

    u32 SizeClass;
    u32 NumBlocksInUse;
    u32 NumBlocksTotal;
    u32 NumChunksInBatch;

    FBinnedBlock_* FreeBlocks;

    FBinnedChunk_* PrevChunk;
    FBinnedChunk_* NextChunk;
    FBinnedChunk_* NextBatch;

#if USE_MALLOCBINNED_PAGE_PROTECT
    void ProtectPage() { FVirtualMemory::Protect(this, ChunkSizeInBytes, false, false); }
    void UnprotectPage() { FVirtualMemory::Protect(this, ChunkSizeInBytes, true, true); }
#endif

    static FBinnedChunk_* Allocate() {
        void* const p = AllocateNewBinnedChunk_();
        Assert(p);
        Assert_NoAssume(Meta::IsAligned(ALLOCATION_GRANULARITY, p));
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(p, ChunkSizeInBytes));
        return (FBinnedChunk_*)p;
    }

    static void Release(FBinnedChunk_* p) {
        Assert(p);
        Assert_NoAssume(Meta::IsAligned(ALLOCATION_GRANULARITY, p));
#if USE_PPE_DEBUG
        Assert_NoAssume(p->CheckCanary());
        Assert_NoAssume(p->SizeClass < FMallocBinned::NumSizeClasses);
        Assert_NoAssume(0 == p->NumBlocksInUse);
        Assert_NoAssume(NumBlocksTotalInChunk(p->SizeClass) == p->NumBlocksTotal);
        Assert_NoAssume(p->FreeBlocks);
        size_t n = 0;
        for (FBinnedBlock_* b = p->FreeBlocks; b; b = b->Next) ++n;
        Assert_NoAssume(n == p->NumBlocksTotal);
#endif
        DeallocateBinnedChunk_(p);
    }
};
STATIC_ASSERT(sizeof(FAtomicSpinLock) == sizeof(u32));
STATIC_ASSERT(sizeof(FBinnedChunk_) == CACHELINE_SIZE);
using FBinnedBatch_ = Meta::TAddPointer<FBinnedChunk_>;
//----------------------------------------------------------------------------
// FBinnedBucket_
//----------------------------------------------------------------------------
struct FBinnedBucket_ {
    FBinnedBlock_* FreeBlocks;
    u32 NumBlocksInUse;
    u32 NumBlocksAvailable;
    FBinnedChunk_* UsedChunks;
};
STATIC_ASSERT(Meta::TIsPod_v<FBinnedBucket_>);
// using POD for fast access to buckets without singleton to help inline trivial allocations
static THREAD_LOCAL FBinnedBucket_ GBinnedThreadBuckets_[FMallocBinned::NumSizeClasses + 1/* extra for large blocks optimization */];
//----------------------------------------------------------------------------
// FBinnedMediumBlocks_
//----------------------------------------------------------------------------
#if USE_MALLOCBINNED_MIPMAPS
struct FBinnedMipMapTraits {
    STATIC_CONST_INTEGRAL(size_t, Granularity, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(size_t, ReservedSize, PPE_MIPMAPS_RESERVEDSIZE);
    static void* PageReserve(size_t sizeInBytes) { return FVirtualMemory::PageReserve(sizeInBytes); }
#   if USE_PPE_MEMORYDOMAINS
    using domain_tag = MEMORYDOMAIN_TAG(MediumMipMaps);
    static void PageCommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageCommit(ptr, sizeInBytes, domain_tag::TrackingData()); }
    static void PageDecommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageDecommit(ptr, sizeInBytes, domain_tag::TrackingData()); }
#   else
    static void PageCommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageCommit(ptr, sizeInBytes); }
    static void PageDecommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageDecommit(ptr, sizeInBytes); }
#   endif
    static void PageRelease(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageRelease(ptr, sizeInBytes); }
};
using FBinnedMediumBlocks_ = TMallocMipMap<FBinnedMipMapTraits>;
#endif //!USE_MALLOCBINNED_MIPMAPS
//----------------------------------------------------------------------------
// FBinnedGlobalCache_
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedGlobalCache_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, MaxFreeBatches, 4); // x 8 x 64kb = 2mb max reserve globally

#if USE_MALLOCBINNED_MIPMAPS
    CACHELINE_ALIGNED FBinnedMediumBlocks_ MipMaps;
#endif

    CACHELINE_ALIGNED FAtomicSpinLock Barrier;
    u32 NumFreeBatches;
    FBinnedBatch_ FreeBatches;
    FBinnedChunk_* DanglingChunks;


    FBinnedGlobalCache_()
    :   NumFreeBatches(0)
    ,   FreeBatches(nullptr)
    ,   DanglingChunks(nullptr)
    {}

    // this dtor is the sole reason why FBinnedGlobalCache_ is a non POD type
    ~FBinnedGlobalCache_();
};
static FBinnedGlobalCache_ GBinnedGlobalCache_; // should be in compiler segment, so destroyed last hopefully
//----------------------------------------------------------------------------
// FBinnedThreadCache_
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedThreadCache_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, MaxFreeChunks, 8); // x 64kb = 512kb max reserve per *thread* (NOT per *core*)
    STATIC_CONST_INTEGRAL(uintptr_t, InvalidBlockRef, uintptr_t(-1));

    u32 NumChunksFreed;
    FBinnedChunk_* FreeChunks; // singly linked list using only NextChunk

    // don't want false sharing, fields can be touched by other threads after this
    CACHELINE_ALIGNED size_t _Padding;

    FAtomicSpinLock DanglingBarrier;
    FBinnedBlock_* DanglingBlocks;

    FBinnedThreadCache_()
    :   NumChunksFreed(0)
    ,   FreeChunks(0)
    ,   DanglingBlocks(nullptr)
    {}

    // this dtor is the sole reason why FBinnedThreadCache_ is a non POD type
    ~FBinnedThreadCache_();

    static FBinnedThreadCache_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FBinnedThreadCache_, GInstance);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
// FBinnedLargeBlocks_
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedLargeBlocks_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, NumLargeBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, CacheSizeInBytes, 16 * 1024 * 1024); // <=> 16 mo global cache for large blocks

    std::mutex Barrier;
    VIRTUALMEMORYCACHE(LargeBlocks, NumLargeBlocks, CacheSizeInBytes) VM;

    FBinnedLargeBlocks_() {}
    ~FBinnedLargeBlocks_();

    static FBinnedLargeBlocks_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FBinnedLargeBlocks_, GInstance);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT
//----------------------------------------------------------------------------
static THREAD_LOCAL bool GBinnedIsInScopeTLS_;
struct FBinnedReentrancy_ {
    FBinnedReentrancy_() {
        Assert(not GBinnedIsInScopeTLS_);
        GBinnedIsInScopeTLS_ = true;
    }
    ~FBinnedReentrancy_() {
        Assert(GBinnedIsInScopeTLS_);
        GBinnedIsInScopeTLS_ = false;
    }
};
//----------------------------------------------------------------------------
struct FBinnedNoReentrancy_ {
    FBinnedNoReentrancy_() {
        Assert(GBinnedIsInScopeTLS_);
        GBinnedIsInScopeTLS_ = false;
    }
    ~FBinnedNoReentrancy_() {
        Assert(not GBinnedIsInScopeTLS_);
        GBinnedIsInScopeTLS_ = true;
    }
};
//----------------------------------------------------------------------------
static bool ContainsChunk_(const FBinnedBatch_ head, const FBinnedChunk_* ch) {
    for (FBinnedChunk_* it = head; it; it = it->NextChunk) {
        Assert_NoAssume(it->CheckCanary());
        Assert_NoAssume(it->ThreadCache == ch->ThreadCache);

        if (it == ch)
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
static void PoisonChunk_(FBinnedChunk_* ch) {
    ch->NumBlocksInUse = u32(-1);
    ch->FreeBlocks = (FBinnedBlock_*)intptr_t(-1);
}
//----------------------------------------------------------------------------
static std::atomic<i64> GBinnedAllocCount_;
static std::atomic<i64> GBinnedAllocSizeInBytes_;
static void OnBinnedAlloc_(size_t sz) {
    GBinnedAllocCount_++;
    GBinnedAllocSizeInBytes_ += checked_cast<i64>(FMallocBinned::SnapSize(sz));
}
static void OnBinnedFree_(size_t sz) {
    Verify(0 <= --GBinnedAllocCount_);
    Verify(0 <= (GBinnedAllocSizeInBytes_ -= checked_cast<i64>(sz)));
}
//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT
//----------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------
static void* AllocateNewBinnedChunk_() {
    void* p;

#if USE_MALLOCBINNED_MIPMAPS
    // better reuse the same memory than mip maps, plus it's *WAY* faster
    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;
    p = gc.MipMaps.AllocateHintTLS(FBinnedChunk_::ChunkSizeInBytes);
    if (p)
        return p;
#endif

#if USE_PPE_MEMORYDOMAINS
    p = FVirtualMemory::InternalAlloc(FBinnedChunk_::ChunkSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(SmallTables));
#else
    p = FVirtualMemory::InternalAlloc(FBinnedChunk_::ChunkSizeInBytes);
#endif

    return p;
}
//----------------------------------------------------------------------------
static void DeallocateBinnedChunk_(void* p) {
    Assert(p);

#if USE_MALLOCBINNED_MIPMAPS
    // need to check if the container is aliasing
    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;
    if (gc.MipMaps.AliasesToMipMaps(p)) {
        gc.MipMaps.Free(p);
        return;
    }
#endif

#if USE_PPE_MEMORYDOMAINS
    FVirtualMemory::InternalFree(p, FBinnedChunk_::ChunkSizeInBytes, MEMORYDOMAIN_TRACKING_DATA(SmallTables));
#else
    FVirtualMemory::InternalFree(p, FBinnedChunk_::ChunkSizeInBytes);
#endif
}
//----------------------------------------------------------------------------
static FORCE_INLINE FBinnedChunk_* ChunkFromBlock_(FBinnedBlock_* blk) {
    STATIC_ASSERT(Meta::IsPow2(FBinnedChunk_::ChunkSizeInBytes));
    Assert_NoAssume(uintptr_t(blk) & (FBinnedChunk_::ChunkSizeInBytes - 1));

    FBinnedChunk_* const ch = (FBinnedChunk_*)(uintptr_t(blk) & FBinnedChunk_::ChunkSizeMask);
    Assert_NoAssume(ch->CheckCanary());

    return ch;
}
//----------------------------------------------------------------------------
static void PokeChunkToFront_(FBinnedBucket_& bk, FBinnedChunk_* ch) {
    Assert(ch);
    Assert(ch != bk.UsedChunks);
    Assert_NoAssume(ch->CheckCanary());
    Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);
    Assert_NoAssume(ch->PrevChunk->CheckCanary());
    Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

    bk.UsedChunks->FreeBlocks = bk.FreeBlocks;
    bk.UsedChunks->NumBlocksInUse = bk.NumBlocksInUse;

    ch->PrevChunk->NextChunk = ch->NextChunk;
    if (ch->NextChunk) {
        Assert_NoAssume(ch->NextChunk->CheckCanary());
        Assert_NoAssume(ch->ThreadCache == ch->NextChunk->ThreadCache);

        ch->NextChunk->PrevChunk = ch->PrevChunk;
    }

    ch->PrevChunk = nullptr;
    ch->NextChunk = bk.UsedChunks;

    bk.UsedChunks->PrevChunk = ch;
    bk.UsedChunks = ch;
    bk.FreeBlocks = ch->FreeBlocks;
    bk.NumBlocksInUse = ch->NumBlocksInUse;

    ONLY_IF_ASSERT(PoisonChunk_(ch)); // crash if we don't update back its state
}

//----------------------------------------------------------------------------
static size_t NonSmallBlockRegionSize_(void* ptr) {
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_GRANULARITY, ptr));

#if USE_MALLOCBINNED_MIPMAPS
    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;
    return (gc.MipMaps.AliasesToMipMaps(ptr)
        ? gc.MipMaps.AllocationSize(ptr)
        : FVirtualMemory::SizeInBytes(ptr));
#else
    return FVirtualMemory::SizeInBytes(ptr);
#endif
}
//----------------------------------------------------------------------------
static size_t NonSmallBlockSnapSize_(size_t sizeInBytes) {
    Assert_NoAssume(sizeInBytes > FBinnedChunk_::MaxSizeInBytes);

#if USE_MALLOCBINNED_MIPMAPS
    sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);
    return (sizeInBytes <= FBinnedMediumBlocks_::TopMipSize
        ? FBinnedMediumBlocks_::SnapSize(sizeInBytes)
        : sizeInBytes );
#else
    return ROUND_TO_NEXT_64K(sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
static void ReleaseFreeBatch_(FBinnedBatch_ batch) {
    Assert(batch);

    ONLY_IF_ASSERT(size_t n = batch->NumChunksInBatch);
    while (batch) {
        Assert_NoAssume(batch->CheckCanary());
        Assert_NoAssume(nullptr == batch->PrevChunk);
        Assert_NoAssume(nullptr == batch->ThreadCache);
        Assert_NoAssume(0 == batch->NumBlocksInUse);
        Assert_NoAssume(batch->FreeBlocks);

        FBinnedBatch_ const next = batch->NextChunk;

        FBinnedChunk_::Release(batch);

        batch = next;
        ONLY_IF_ASSERT(--n);
    }
    Assert_NoAssume(0 == n);
}
//----------------------------------------------------------------------------
static void ReleaseFreeBatches_(FBinnedGlobalCache_& gc) {
    if (gc.FreeBatches) {
        const FAtomicSpinLock::FScope scopeLock(gc.Barrier);

        FBinnedBatch_ batch = gc.FreeBatches;
        while (batch) {
            ONLY_IF_ASSERT(Assert(gc.NumFreeBatches--));

            FBinnedBatch_ const next = batch->NextBatch;
            ReleaseFreeBatch_(batch);
            batch = next;
        }

        gc.FreeBatches = nullptr;
#ifdef WITH_PPE_ASSERT
        Assert(0 == gc.NumFreeBatches);
#else
        gc.NumFreeBatches = 0;
#endif
    }
}
//----------------------------------------------------------------------------
static void ReleaseFreeChunks_(FBinnedThreadCache_& tc) {
    if (tc.FreeChunks) {
        FBinnedBatch_ const batch = tc.FreeChunks;
        batch->NumChunksInBatch = tc.NumChunksFreed;

        tc.FreeChunks = nullptr;
        tc.NumChunksFreed = 0;

        ReleaseFreeBatch_(batch);
    }
}
//----------------------------------------------------------------------------
static FORCE_INLINE void ReleaseLargeBlocks_(FBinnedLargeBlocks_& lb) {
    lb.VM.ReleaseAll();
}
//----------------------------------------------------------------------------
static void BinnedFree_(FBinnedBucket_& bk, FBinnedChunk_* ch, FBinnedBlock_* blk);
static NO_INLINE bool ReleaseDanglingBlocks_(FBinnedThreadCache_& tc) {
    const FAtomicSpinLock::FTryScope scopeLock(tc.DanglingBarrier);

    if (scopeLock.Locked) { // lazy locking to minimize thread contention
        if (Likely(FBinnedBlock_* blk = tc.DanglingBlocks)) {
            do {
                Assert_NoAssume(uintptr_t(blk) != FBinnedThreadCache_::InvalidBlockRef);
                FBinnedChunk_* const ch = ChunkFromBlock_(blk);
                Assert_NoAssume(&tc == ch->ThreadCache);
                Assert(ch->SizeClass < FMallocBinned::NumSizeClasses);

                FBinnedBucket_& bk = GBinnedThreadBuckets_[ch->SizeClass];
                Assert(bk.UsedChunks);

                FBinnedBlock_* const nxt = blk->Next;

                if (Likely(bk.UsedChunks == ch && bk.NumBlocksInUse > 1)) {
                    Assert_NoAssume(ch->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

                    blk->Next = bk.FreeBlocks;
                    bk.FreeBlocks = blk;
                    bk.NumBlocksInUse--;
                    bk.NumBlocksAvailable++;
                }
                else {
                    BinnedFree_(bk, ch, blk);
                }

                blk = nxt;
            } while (blk);

            tc.DanglingBlocks = nullptr;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
static void DetachDanglingChunk_(FBinnedGlobalCache_& gc, FBinnedChunk_* ch) {
    Assert(ch);

    if (ch->NextChunk)
        ch->NextChunk->PrevChunk = ch->PrevChunk;

    if (ch->PrevChunk) {
        Assert_NoAssume(ch != gc.DanglingChunks);
        ch->PrevChunk->NextChunk = ch->NextChunk;
    }
    else {
        Assert_NoAssume(ch == gc.DanglingChunks);
        gc.DanglingChunks = ch->NextChunk;
    }
}
//----------------------------------------------------------------------------
// BinnedMalloc_()
//----------------------------------------------------------------------------
static FBinnedBatch_ FetchFreeBatch_() {
    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;

    if (gc.FreeBatches) {
        const FAtomicSpinLock::FScope scopeLock(gc.Barrier);

        if (FBinnedBatch_ const batch = gc.FreeBatches) {
            Assert(gc.NumFreeBatches);
            Assert_NoAssume(batch->CheckCanary());
            Assert_NoAssume(batch->NumChunksInBatch);
            Assert_NoAssume(batch->ThreadCache == nullptr);
            Assert_NoAssume(batch->NumBlocksInUse == 0);
            Assert_NoAssume(batch->FreeBlocks);

            gc.FreeBatches = batch->NextBatch;
            gc.NumFreeBatches--;

            batch->NextBatch = nullptr;

            return batch;
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
static void InitializeChunk_(FBinnedChunk_* ch, const size_t sizeClass) {
    Assert(ch);
    Assert_NoAssume(0 == ch->NumBlocksInUse);

    const size_t blockSize = FMallocBinned::SizeClasses[sizeClass];
    Assert_NoAssume(Meta::IsAligned(16, blockSize));

    ch->SizeClass = u32(sizeClass);
    ch->NumBlocksTotal = u32(FBinnedChunk_::ChunkAvailableSizeInBytes / blockSize);
    ch->FreeBlocks = (FBinnedBlock_*)((u8*)ch + FBinnedChunk_::ChunkSizeInBytes - ch->NumBlocksTotal * blockSize);
    Assert_NoAssume(Meta::IsAligned(16, ch->FreeBlocks));

    FBinnedBlock_* blk = ch->FreeBlocks;
    forrange(i, 1, ch->NumBlocksTotal) {
        auto* const next = (FBinnedBlock_*)((u8*)blk + blockSize);
        blk->Next = next;
        blk = next;
    }

    Assert_NoAssume(((u8*)blk + blockSize) == ((u8*)ch + FBinnedChunk_::ChunkSizeInBytes));
    blk->Next = nullptr;
}
//----------------------------------------------------------------------------
static NO_INLINE void* BinnedMalloc_(FBinnedBucket_& bk, size_t size, size_t sizeClass) {
    Assert_NoAssume(size);

    // for small blocks < 32kb :
    if (Likely(sizeClass < FMallocBinned::NumSizeClasses)) {
        Assert_NoAssume(size <= FBinnedChunk_::MaxSizeInBytes);
        Assert_NoAssume(size <= FMallocBinned::SizeClasses[sizeClass]);
        Assert_NoAssume(bk.FreeBlocks == nullptr);

        // check if there's still free blocks available in the bucket
        if (bk.NumBlocksAvailable) {
            Assert(bk.UsedChunks);
            Assert_NoAssume(bk.UsedChunks->PrevChunk == nullptr);

            // find the chunk with place available (guaranteed)
            FBinnedChunk_* ch = bk.UsedChunks->NextChunk;
            Assert_NoAssume(bk.NumBlocksInUse == ch->NumBlocksTotal);

            for (;;) {
                Assert(ch);
                Assert_NoAssume(ch->CheckCanary());
                Assert_NoAssume(ch != bk.UsedChunks);

                if (ch->NumBlocksInUse < ch->NumBlocksTotal) {
                    Assert(ch->FreeBlocks);

                    // steal one free block for current allocation
                    void* const p = ch->FreeBlocks;
                    ch->FreeBlocks = ch->FreeBlocks->Next;
                    ch->NumBlocksInUse++;
                    bk.NumBlocksAvailable--;

                    // poke to front of bk.UsedChunks if it has more free blocks than current head
                    if (ch->NumBlocksInUse < bk.NumBlocksInUse)
                        PokeChunkToFront_(bk, ch);

                    return p; // early out with the free block
                }

                ch = ch->NextChunk;
            }
        }
        // we're out of memory in the current bucket, fetch some
        else {
            FBinnedThreadCache_& tc = FBinnedThreadCache_::Get();
            Assert_NoAssume(FBinnedThreadCache_::InvalidBlockRef != uintptr_t(tc.DanglingBlocks));

            // since we're OOM in the current bucket try to release pending blocks
            if (Unlikely(tc.DanglingBlocks && ReleaseDanglingBlocks_(tc) && bk.NumBlocksAvailable)) {
                // needed to avoid assertion due to recursive call to FMallocBinned::Malloc() below
                ONLY_IF_ASSERT(FBinnedNoReentrancy_ noReentrancy);
                ONLY_IF_ASSERT(OnBinnedFree_(FMallocBinned::SizeClasses[sizeClass]));

                // recurse to Malloc() if we freed some blocks in the current bucket
                return FMallocBinned::Malloc(size);
            }

            FBinnedChunk_* ch;

            // try to fetch from thread local cache
            if (Likely(tc.FreeChunks)) {
                Assert(tc.NumChunksFreed);
                Assert_NoAssume(tc.NumChunksFreed < FBinnedThreadCache_::MaxFreeChunks);
                Assert_NoAssume(nullptr == tc.FreeChunks->PrevChunk);

                ch = tc.FreeChunks;
                tc.FreeChunks = tc.FreeChunks->NextChunk;
                tc.NumChunksFreed--;
            }
            // else look in global cache or allocate a new page
            else {
                Assert_NoAssume(tc.FreeChunks == nullptr);
                Assert_NoAssume(tc.NumChunksFreed == 0);

                // try to fetch a batch of free chunks from global cache
                ch = FetchFreeBatch_();
                if (ch) {
                    tc.FreeChunks = ch->NextChunk;
                    tc.NumChunksFreed = (ch->NumChunksInBatch - 1);
                    ch->NumChunksInBatch = 0;
                }
                // allocate a new page from virtual memory;
                else {
                    ch = FBinnedChunk_::Allocate();
                    INPLACE_NEW(&ch->ThreadBarrier, FAtomicSpinLock);
                    ch->ThreadCache = nullptr;
                    ch->SizeClass = u32(-1); // to trigger initialization of FreeBlocks linked list
                    ch->NumBlocksInUse = 0;
                    ch->NumChunksInBatch = 0;
                    ch->NextBatch = nullptr;
#ifdef WITH_PPE_ASSERT
                    ch->_Canary = FBinnedChunk_::DefaultCanary;
#endif
                }
            }

            Assert(ch);
            Assert_NoAssume(ch->CheckCanary());
            Assert_NoAssume(nullptr == ch->ThreadCache);
            Assert_NoAssume(nullptr == ch->NextBatch);
            Assert_NoAssume(0 == ch->NumBlocksInUse);
            Assert_NoAssume(0 == ch->NumChunksInBatch);

            ch->ThreadCache = &tc;
            if (ch->SizeClass != sizeClass)
                InitializeChunk_(ch, sizeClass);

            Assert_NoAssume(ch->FreeBlocks);
            Assert_NoAssume(ch->NumBlocksTotal);

            // insert at front of used chunks list
            ch->PrevChunk = nullptr;
            ch->NextChunk = bk.UsedChunks;

            if (bk.UsedChunks) {
                Assert_NoAssume(bk.UsedChunks->CheckCanary());
                Assert_NoAssume(nullptr == bk.UsedChunks->PrevChunk);
                Assert_NoAssume(bk.NumBlocksInUse == bk.UsedChunks->NumBlocksTotal);
                Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1)); // check poisoning

                // sync previous chunk with bucket data
                bk.UsedChunks->NumBlocksInUse = bk.NumBlocksInUse;
                bk.UsedChunks->FreeBlocks = nullptr;
                bk.UsedChunks->PrevChunk = ch;
            }

            // allocate the first block for current request and register new chunk
            void* const p = ch->FreeBlocks;
            ch->FreeBlocks = ch->FreeBlocks->Next;
            bk.FreeBlocks = ch->FreeBlocks;
            bk.NumBlocksInUse = ++ch->NumBlocksInUse;
            bk.NumBlocksAvailable += (ch->NumBlocksTotal - 1);
            bk.UsedChunks = ch;

            ONLY_IF_ASSERT(PoisonChunk_(ch));
            Assert_NoAssume(uintptr_t(p) & (FBinnedChunk_::ChunkSizeInBytes - 1));
            return p;
        }
    }
    // for large blocks, align on 64kb :
    else {
        Assert_NoAssume(size > FBinnedChunk_::MaxSizeInBytes);

        STATIC_ASSERT(ALLOCATION_GRANULARITY == 64 * 1024);
        size = ROUND_TO_NEXT_64K(size);

        void* p = nullptr;
#if USE_MALLOCBINNED_MIPMAPS
        if (size <= FBinnedMediumBlocks_::TopMipSize)
            p = GBinnedGlobalCache_.MipMaps.AllocateHintTLS(size);

        if (nullptr == p) // also handle mip map OOM
#endif
            p = FBinnedLargeBlocks_::Get().VM.Allocate(size);
#if USE_MALLOCBINNED_MIPMAPS
        else {
            Assert_NoAssume(GBinnedGlobalCache_.MipMaps.AliasesToMipMaps(p));
            Assert_NoAssume(GBinnedGlobalCache_.MipMaps.AllocationSize(p) >= size);
        }
#endif


        Assert_NoAssume(Meta::IsAligned(FBinnedChunk_::ChunkSizeInBytes, p));
        return p;
    }
}
//----------------------------------------------------------------------------
// BinnedFree_()
//----------------------------------------------------------------------------
static NO_INLINE void RejectFreeBatch_(FBinnedThreadCache_& tc) {
    Assert(tc.FreeChunks);
    Assert_NoAssume(tc.NumChunksFreed == FBinnedThreadCache_::MaxFreeChunks);

    FBinnedBatch_ batch = tc.FreeChunks;
    Assert_NoAssume(batch->CheckCanary());
    Assert_NoAssume(0 == batch->NumChunksInBatch);
    Assert_NoAssume(batch->ThreadCache == nullptr);
    Assert_NoAssume(batch->NextBatch == nullptr);
    Assert_NoAssume(batch->NumBlocksInUse == 0);
    Assert_NoAssume(batch->FreeBlocks);
    batch->NumChunksInBatch = tc.NumChunksFreed;

    tc.FreeChunks = nullptr;
    tc.NumChunksFreed = 0;

    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;

    if (gc.NumFreeBatches < FBinnedGlobalCache_::MaxFreeBatches) {
        const FAtomicSpinLock::FScope scopeLock(gc.Barrier);

        if (gc.NumFreeBatches < FBinnedGlobalCache_::MaxFreeBatches) {
            batch->NextBatch = gc.FreeBatches;
            gc.FreeBatches = batch;
            gc.NumFreeBatches++;

            return; // batch successfully registered in global cache
        }
    }

    // global cache is full, release the batch
    ReleaseFreeBatch_(batch);
}
//----------------------------------------------------------------------------
static void DanglingFree_(FBinnedThreadCache_& tc, FBinnedChunk_* ch, FBinnedBlock_* blk) {
    Assert(ch);
    Assert(blk);
    Assert_NoAssume(ch->NumBlocksInUse);
    {
        // first lock the chunk (only useful for dying thread caches)
        FAtomicSpinLock::FUniqueLock lockChunk(ch->ThreadBarrier);

        // test if the chunk is dangling
        if (Likely(ch->ThreadCache)) {
            FBinnedThreadCache_& oc = (*ch->ThreadCache);

            // then lock the thread cache before adding the pending block
            const FAtomicSpinLock::FScope lockThread(oc.DanglingBarrier);
            Assert_NoAssume(ch->CheckCanary());
            Assert_NoAssume(&oc == ch->ThreadCache);
            Assert_NoAssume(FBinnedThreadCache_::InvalidBlockRef != uintptr_t(oc.DanglingBlocks));

            blk->Next = oc.DanglingBlocks;
            oc.DanglingBlocks = blk;

            return; // early out since chunk can't be stolen
        }
        // else the chunk itself is dangling
        else {
            FBinnedGlobalCache_& gc = GBinnedGlobalCache_;

            FAtomicSpinLock::FUniqueLock lockGlobal(gc.Barrier);
            Assert_NoAssume(ContainsChunk_(gc.DanglingChunks, ch));

            // remove the dangling chunk from the global cache
            if (Likely(FBinnedThreadCache_::InvalidBlockRef != uintptr_t(tc.DanglingBlocks))) {
                DetachDanglingChunk_(gc, ch);
            }
            // the current thread cache is already destroyed,
            // delete the block from the chunk while keeping it in the global cache
            else {
                blk->Next = ch->FreeBlocks;
                ch->FreeBlocks = blk;

                // release the chunk if it's finally empty
                if (0 == --ch->NumBlocksInUse) {
                    DetachDanglingChunk_(gc, ch);

                    // unlock gc before deleting, less contention
                    lockGlobal.Unlock();

                    // unlock ch before deleting, no necrophilia
                    lockChunk.Unlock();

                    FBinnedChunk_::Release(ch);
                }

                return; // early out since we can/want not steal the chunk
            }
        }
        Assert_NoAssume(ch->CheckCanary());

        // steal the chunk in current thread local cache
        FBinnedBucket_& bk = GBinnedThreadBuckets_[ch->SizeClass];

        ch->ThreadCache = &tc;

        // insert at front of used chunks list
        ch->PrevChunk = nullptr;
        ch->NextChunk = bk.UsedChunks;

        if (bk.UsedChunks) {
            Assert_NoAssume(bk.UsedChunks->CheckCanary());
            Assert_NoAssume(nullptr == bk.UsedChunks->PrevChunk);

            // sync previous chunk with bucket data
            bk.UsedChunks->NumBlocksInUse = bk.NumBlocksInUse;
            bk.UsedChunks->FreeBlocks = bk.FreeBlocks;
            bk.UsedChunks->PrevChunk = ch;
        }

        // register the new free blocks as head of the used chunks
        bk.FreeBlocks = ch->FreeBlocks;
        bk.NumBlocksInUse = ch->NumBlocksInUse;
        bk.NumBlocksAvailable += (ch->NumBlocksTotal - ch->NumBlocksInUse);
        bk.UsedChunks = ch;

        ONLY_IF_ASSERT(PoisonChunk_(ch));
    }

    // needed to avoid an error due to call to FMallocBinned::Free() below
    ONLY_IF_ASSERT(FBinnedNoReentrancy_ noReentrancy);
    ONLY_IF_ASSERT(OnBinnedAlloc_(FMallocBinned::SizeClasses[ch->SizeClass]));

    // finally recurse for FMallocBinned::Free() *OUTSIDE* the chunk lock scope
    FMallocBinned::Free(blk);
}
//----------------------------------------------------------------------------
static NO_INLINE void BinnedFree_(FBinnedBucket_& bk, FBinnedChunk_* ch, FBinnedBlock_* blk) {
    Assert(ch);
    Assert(blk);
    Assert_NoAssume(nullptr == ch->NextBatch);
    Assert_NoAssume(0 == ch->NumChunksInBatch);

    FBinnedThreadCache_& tc = FBinnedThreadCache_::Get();

    // check if chunk is owned by current thread
    if (Likely(ch->ThreadCache == &tc)) {
        Assert_NoAssume(ch->CheckCanary());
        Assert_NoAssume(FBinnedThreadCache_::InvalidBlockRef != uintptr_t(tc.DanglingBlocks));

        // if this the head chunk then it's because we need to release the chunk
        if (Unlikely(bk.UsedChunks == ch)) {
            Assert(bk.NumBlocksInUse == 1);
            Assert_NoAssume(nullptr == ch->PrevChunk);
            Assert_NoAssume(bk.NumBlocksAvailable >= (ch->NumBlocksTotal - 1));
            Assert_NoAssume(ch->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

            bk.NumBlocksAvailable -= (ch->NumBlocksTotal - 1);
            bk.UsedChunks = ch->NextChunk;

            blk->Next = bk.FreeBlocks;
            ch->FreeBlocks = blk;
            ch->NumBlocksInUse = 0;
            ch->ThreadCache = nullptr;
            ch->NextChunk = tc.FreeChunks;

            if (bk.UsedChunks) {
                Assert_NoAssume(bk.UsedChunks->CheckCanary());
                Assert_NoAssume(ch == bk.UsedChunks->PrevChunk);
                Assert_NoAssume(bk.UsedChunks->NumBlocksInUse);

                bk.UsedChunks->PrevChunk = nullptr;
                bk.FreeBlocks = bk.UsedChunks->FreeBlocks;
                bk.NumBlocksInUse = bk.UsedChunks->NumBlocksInUse;

                ONLY_IF_ASSERT(PoisonChunk_(bk.UsedChunks));
            }
            else {
                Assert_NoAssume(0 == bk.NumBlocksAvailable);

                bk.FreeBlocks = nullptr;
                bk.NumBlocksInUse = 0;
            }

            tc.FreeChunks = ch;
            tc.NumChunksFreed++;
        }
        // else we release the block and check if the chunk also needs to be released
        else {
            Assert(ch->PrevChunk); // since ch != bk.UsedChunks
            Assert(ch->NumBlocksInUse);
            Assert_NoAssume(ch->NumBlocksInUse <= ch->NumBlocksTotal);
            Assert_NoAssume(ContainsChunk_(bk.UsedChunks, ch));

            bk.NumBlocksAvailable++;

            blk->Next = ch->FreeBlocks;
            ch->FreeBlocks = blk;
            ch->NumBlocksInUse--;

            // put the chunk in thread's free chunks cache if it's empty
            if (Unlikely(0 == ch->NumBlocksInUse)) {
                Assert(bk.NumBlocksAvailable >= ch->NumBlocksTotal);

                bk.NumBlocksAvailable -= ch->NumBlocksTotal;

                ch->PrevChunk->NextChunk = ch->NextChunk;
                if (ch->NextChunk)
                    ch->NextChunk->PrevChunk = ch->PrevChunk;

                ch->ThreadCache = nullptr;
                ch->PrevChunk = nullptr;
                ch->NextChunk = tc.FreeChunks;

                tc.FreeChunks = ch;
                tc.NumChunksFreed++;
            }
            // poke to front if chunk has more free blocks than current head
            else if (ch->NumBlocksInUse < bk.NumBlocksInUse) {
                PokeChunkToFront_(bk, ch);
            }
        }

        // if the number of free chunks is too big we give this free batch of chunks to the global cache
        if (Unlikely(FBinnedThreadCache_::MaxFreeChunks == tc.NumChunksFreed))
            RejectFreeBatch_(tc);
    }
    else {
        DanglingFree_(tc, ch, blk);
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void LargeFree_(void* ptr) {
    Assert(ptr);

#if USE_MALLOCBINNED_MIPMAPS
    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;
    if (gc.MipMaps.AliasesToMipMaps(ptr)) {
        gc.MipMaps.Free(ptr);
    }
    else
#endif
    {
        FBinnedLargeBlocks_::Get().VM.Free(ptr);
    }
}
//----------------------------------------------------------------------------
// Destructors
//----------------------------------------------------------------------------
FBinnedThreadCache_::~FBinnedThreadCache_() {
    ReleaseDanglingBlocks_(*this);

    // invalid ptr to be sure than it won't be accessed
    FAtomicSpinLock::FScope scopeLock(DanglingBarrier);
    Assert_NoAssume(nullptr == DanglingBlocks);
    DanglingBlocks = (FBinnedBlock_*)InvalidBlockRef;

    ReleaseFreeChunks_(*this);

    FBinnedGlobalCache_& gc = GBinnedGlobalCache_;

    // need to refurbish chunks with blocks still allocated
    forrange(bk, &GBinnedThreadBuckets_[0], &GBinnedThreadBuckets_[FMallocBinned::NumSizeClasses]) {
        if (FBinnedChunk_* ch = bk->UsedChunks) {
            const FAtomicSpinLock::FScope lockGlobal(gc.Barrier);

            Assert_NoAssume(ch->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

            // don't forget to sync the head with the bucket
            bk->UsedChunks->NumBlocksInUse = bk->NumBlocksInUse;
            bk->UsedChunks->FreeBlocks = bk->FreeBlocks;

            ONLY_IF_ASSERT(bk->UsedChunks = (FBinnedChunk_*)InvalidBlockRef);
            ONLY_IF_ASSERT(bk->FreeBlocks = (FBinnedBlock_*)InvalidBlockRef);

            do {
                FBinnedChunk_* const next = ch->NextChunk;
                Assert_NoAssume(ch->CheckCanary());
                Assert_NoAssume(not next || next->PrevChunk == ch);
                Assert_NoAssume(this == ch->ThreadCache);
                Assert_NoAssume(ch->NumBlocksInUse);
                Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);
                Assert_NoAssume(nullptr == ch->NextBatch);
                Assert_NoAssume(0 == ch->NumChunksInBatch);
                {
                    const FAtomicSpinLock::FScope lockChunk(ch->ThreadBarrier);

                    ch->ThreadCache = nullptr;
                    ch->PrevChunk = nullptr;
                    ch->NextChunk = gc.DanglingChunks;

                    if (gc.DanglingChunks)
                        gc.DanglingChunks->PrevChunk = ch;

                    gc.DanglingChunks = ch;
                }
                ch = next;
            } while (ch);
        }
    }

    Assert_NoAssume((FBinnedBlock_*)InvalidBlockRef == DanglingBlocks);
}
//----------------------------------------------------------------------------
FBinnedGlobalCache_::~FBinnedGlobalCache_() {
    ReleaseFreeBatches_(*this);

    // inspect leaking chunks, think about pgm destruction
#if USE_PPE_PLATFORM_DEBUG
    size_t numDanglingBlocks = 0;
    size_t numDanglingChunks = 0;
    size_t totalSizeDangling = 0;
    {
        const FAtomicSpinLock::FScope scopeLock(Barrier);
        for (FBinnedChunk_* ch = DanglingChunks; ch; ch = ch->NextChunk) {
            Assert_NoAssume(ch->CheckCanary());
            Assert_NoAssume(ch->SizeClass < FMallocBinned::NumSizeClasses);
            Assert_NoAssume(FBinnedChunk_::NumBlocksTotalInChunk(ch->SizeClass) == ch->NumBlocksTotal);
            Assert_NoAssume(nullptr == ch->ThreadCache);
            Assert_NoAssume(ch->NumBlocksInUse);
            Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);
            Assert_NoAssume(nullptr == ch->NextBatch);
            Assert_NoAssume(0 == ch->NumChunksInBatch);
            Assert_NoAssume(not ch->NextChunk || ch->NextChunk->PrevChunk == ch);

            numDanglingBlocks += ch->NumBlocksInUse;
            numDanglingChunks++;
            totalSizeDangling += (FMallocBinned::SizeClasses[ch->SizeClass] * ch->NumBlocksInUse);

#ifdef   WITH_PPE_ASSERT
            size_t n = 0;
            const size_t sz = FMallocBinned::SizeClasses[ch->SizeClass];
            for (auto* b = ch->FreeBlocks; b; ) {
                auto* q = b;
                b = b->Next;
                FPlatformMemory::Memset(q, 0xFF, sz);
                ++n;
            }
            Assert((ch->NumBlocksTotal - ch->NumBlocksInUse) == n);
            PoisonChunk_(ch);
#endif
        }
    }

    if (numDanglingChunks) {
        wchar_t buffer[1024];
        Format(buffer, L"ERROR: still have {0} dangling chunk(s) with {1} dangling blocks (total = {2})\n",
            numDanglingChunks, numDanglingBlocks, Fmt::SizeInBytes(totalSizeDangling) );

        FPlatformDebug::OutputDebug(buffer);
    }

    // tries to know if it's a bug in the allocator
    ONLY_IF_ASSERT(const i64 binnedAllocCount = GBinnedAllocCount_);
    ONLY_IF_ASSERT(const i64 binnedAllocSizeInBytes = GBinnedAllocSizeInBytes_);
    Assert_NoAssume(checked_cast<i64>(numDanglingBlocks) == binnedAllocCount);
    Assert_NoAssume(checked_cast<i64>(totalSizeDangling) == binnedAllocSizeInBytes);
#endif
}
//----------------------------------------------------------------------------
FBinnedLargeBlocks_::~FBinnedLargeBlocks_() {
    ReleaseLargeBlocks_(*this);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMallocBinned::Malloc(size_t size) {
    Assert_NoAssume(size);
    ONLY_IF_ASSERT(OnBinnedAlloc_(size));

    const size_t sizeClass = MakeBinnedClass_(size);

    // SizeClass <NumSizeClasses> won't ever be filled, allows to make only one test
    FBinnedBucket_& bk = GBinnedThreadBuckets_[Min(sizeClass, FMallocBinned::NumSizeClasses)];

    if (Likely(FBinnedBlock_* const blk = bk.FreeBlocks)) {
        Assert(bk.NumBlocksAvailable);
        Assert_NoAssume(size <= FMallocBinned::SizeClasses[sizeClass]);
        Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

        bk.FreeBlocks = blk->Next;
        bk.NumBlocksInUse++;
        bk.NumBlocksAvailable--;
        return blk;
    }
    else {
        ONLY_IF_ASSERT(const FBinnedReentrancy_ checkReentrancy);

        return BinnedMalloc_(bk, size, sizeClass);
    }
}
//----------------------------------------------------------------------------
void FMallocBinned::Free(void* ptr) {
    Assert(ptr);
    ONLY_IF_ASSERT(OnBinnedFree_(RegionSize(ptr)));

    if (Likely(uintptr_t(ptr) & (FBinnedChunk_::ChunkSizeInBytes - 1))) {
        FBinnedBlock_* const blk = (FBinnedBlock_*)ptr;
        FBinnedChunk_* const ch = ChunkFromBlock_(blk);
        Assert(ch->SizeClass < FMallocBinned::NumSizeClasses);

        FBinnedBucket_& bk = GBinnedThreadBuckets_[ch->SizeClass];

        if (Likely(bk.UsedChunks == ch && bk.NumBlocksInUse > 1)) {
            Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

            blk->Next = bk.FreeBlocks;
            bk.FreeBlocks = blk;
            bk.NumBlocksInUse--;
            bk.NumBlocksAvailable++;
        }
        else {
            ONLY_IF_ASSERT(const FBinnedReentrancy_ checkReentrancy);

            BinnedFree_(bk, ch, blk);
        }
    }
    else {
        return LargeFree_(ptr);
    }
}
//----------------------------------------------------------------------------
void* FMallocBinned::Realloc(void* ptr, size_t size) {
    Assert_NoAssume(ptr || size);

    if (Likely(ptr)) {
        void* newp;

        if (Likely(size)) {
            const size_t old = FMallocBinned::RegionSize(ptr);
            if (MakeBinnedClass_(old) == MakeBinnedClass_(size))
                return ptr;

            // get a new block
            newp = FMallocBinned::Malloc(size);

            // need to align on 16 for Memstream()
            const size_t cpy = ROUND_TO_NEXT_16(Min(old, size));
            Assert_NoAssume(cpy <= FMallocBinned::SizeClasses[
                MakeBinnedClass_(Min(old, size))]);

            // copy previous data to new block without polluting caches
            FPlatformMemory::Memstream(newp, ptr, cpy);
        }
        else {
            newp = nullptr;
        }

        // release old block
        FMallocBinned::Free(ptr);

        return newp;
    }
    else {
        Assert_NoAssume(size);

        return FMallocBinned::Malloc(size);
    }
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedMalloc(size_t size, size_t alignment) {
    void* const p = FMallocBinned::Malloc(Meta::RoundToNext(size, alignment));
    Assert_NoAssume(Meta::IsAligned(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
void FMallocBinned::AlignedFree(void* ptr) {
    return FMallocBinned::Free(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedRealloc(void* ptr, size_t size, size_t alignment) {
    void* const p = FMallocBinned::Realloc(ptr, Meta::RoundToNext(size, alignment));
    Assert_NoAssume(0 == size || Meta::IsAligned(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleaseCacheMemory() {
    LOG(MallocBinned, Debug, L"release cache memory in {0}", std::this_thread::get_id());

    auto& tc = FBinnedThreadCache_::Get();
    ReleaseDanglingBlocks_(tc);
    ReleaseFreeChunks_(tc);

    ReleaseFreeBatches_(GBinnedGlobalCache_);
    ReleaseLargeBlocks_(FBinnedLargeBlocks_::Get());

#if USE_MALLOCBINNED_MIPMAPS
    GBinnedGlobalCache_.MipMaps.GarbageCollect();
#endif
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleasePendingBlocks() {
    //LOG(MallocBinned, Debug, L"release pending blocks in {0}", std::this_thread::get_id()); // too verbose

    ReleaseDanglingBlocks_(FBinnedThreadCache_::Get());
}
//----------------------------------------------------------------------------
size_t FMallocBinned::SnapSize(size_t size) {
    Assert(size);

    const size_t sizeClass = MakeBinnedClass_(size);
    return (Likely(sizeClass < FMallocBinned::NumSizeClasses)
        ? FMallocBinned::SizeClasses[sizeClass]
        : NonSmallBlockSnapSize_(size) );
}
//----------------------------------------------------------------------------
size_t FMallocBinned::RegionSize(void* ptr) {
    Assert(ptr);

    return (Likely(not Meta::IsAligned(FBinnedChunk_::ChunkSizeInBytes, ptr))
        ? FMallocBinned::SizeClasses[ChunkFromBlock_((FBinnedBlock_*)ptr)->SizeClass]
        : NonSmallBlockRegionSize_(ptr) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
