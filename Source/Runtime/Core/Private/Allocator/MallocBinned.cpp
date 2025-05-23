﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/MallocBinned.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Memory/VirtualMemoryCache.h"
#include "Thread/AtomicSpinLock.h"

#if USE_PPE_ASSERT
#   include "Diagnostic/DecodedCallstack.h"
#endif
#if USE_PPE_LOGGER
#   include "Thread/ThreadContext.h"
#endif
#if USE_PPE_PLATFORM_DEBUG
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
#endif

#include "Allocator/InitSegAllocator.h"

#if (USE_PPE_ASSERT || USE_PPE_MEMORY_DEBUGGING)
#   define USE_MALLOCBINNED_PAGE_PROTECT    1// Crash when using a cached block
#else
#   define USE_MALLOCBINNED_PAGE_PROTECT    0// Crash when using a cached block
#endif

// for medium > large block size allocations + used as FBinnedChunk allocator (*WAY* faster)
#define USE_MALLOCBINNED_BITMAPS 1 // less internal fragmentation %_NOCOMMIT%
#define USE_MALLOCBINNED_MIPMAPS 0 // less external fragmentation %_NOCOMMIT%

#if USE_MALLOCBINNED_BITMAPS && USE_MALLOCBINNED_MIPMAPS
#   error "can't use both bitmaps and mipmaps"
#endif

#if USE_MALLOCBINNED_BITMAPS
#    include "Allocator/MallocBitmap.h"
#endif
#if USE_MALLOCBINNED_MIPMAPS
#    include "Allocator/MallocMipMap.h"
#endif

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
    STATIC_ASSERT(MaxSizeInBytes == FMallocBinned::MaxSmallBlockSize);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeInBytes, ALLOCATION_GRANULARITY);
    STATIC_CONST_INTEGRAL(size_t, ChunkSizeMask, ~(ChunkSizeInBytes - 1));
    STATIC_CONST_INTEGRAL(size_t, ChunkAvailableSizeInBytes, ChunkSizeInBytes - CACHELINE_SIZE);

    STATIC_ASSERT(Meta::IsAlignedPow2(Alignment, MinSizeInBytes));
    STATIC_ASSERT(Meta::IsAlignedPow2(Alignment, MaxSizeInBytes));
    STATIC_ASSERT(Meta::IsAlignedPow2(Alignment, ChunkSizeInBytes));
    STATIC_ASSERT(Meta::IsAlignedPow2(Alignment, ChunkAvailableSizeInBytes));

    FBinnedThreadCache_* ThreadCache;
    FAtomicSpinLock ThreadBarrier;

#if USE_PPE_ASSERT
    STATIC_CONST_INTEGRAL(u32, DefaultCanary, 0x4110c470ul);
    u32 _Canary;
    bool CheckCanary() const {
        Assert_NoAssume(Meta::IsAlignedPow2(FBinnedChunk_::ChunkSizeInBytes, this));
        return (DefaultCanary == _Canary);
    }
    static constexpr size_t NumBlocksTotalInChunk(size_t sizeClass) {
        return (FBinnedChunk_::ChunkAvailableSizeInBytes / FMallocBinned::SizeClasses[sizeClass]);
    }
#endif

    u32 SizeClass;
    u16 HighestIndex;
    u16 NumBlocksInUse;
    u32 NumBlocksTotal;
    u32 NumChunksInBatch;

    FBinnedBlock_* FreeBlocks;

    FBinnedChunk_* PrevChunk;
    FBinnedChunk_* NextChunk;
    FBinnedChunk_* NextBatch;

#if USE_MALLOCBINNED_PAGE_PROTECT
    void ProtectPage() { FVirtualMemory::Protect(this, ChunkSizeInBytes, true, true); }
    void UnprotectPage() { FVirtualMemory::Protect(this, ChunkSizeInBytes, false, false); }
#endif

    static FBinnedChunk_* Allocate() {
        void* const p = AllocateNewBinnedChunk_();
        Assert(p);
        Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_GRANULARITY, p));
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, ChunkSizeInBytes));
        return (FBinnedChunk_*)p;
    }

    static void Release(FBinnedChunk_* p) {
        Assert(p);
        Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_GRANULARITY, p));
#if USE_PPE_DEBUG
        Assert_NoAssume(p->CheckCanary());
        Assert_NoAssume(p->SizeClass < FMallocBinned::NumSizeClasses);
        Assert_NoAssume(0 == p->NumBlocksInUse);
        Assert_NoAssume(NumBlocksTotalInChunk(p->SizeClass) == p->NumBlocksTotal);
        Assert_NoAssume(p->FreeBlocks);
        u32 n = p->HighestIndex;
        for (FBinnedBlock_* b = p->FreeBlocks; b; b = b->Next) ++n;
        Assert_NoAssume(n == p->NumBlocksTotal);
#endif
        DeallocateBinnedChunk_(p);
    }

    FORCE_INLINE static FBinnedBlock_* BlockFromIndex(FBinnedChunk_* ch, size_t i, size_t blockSize) {
        Assert(i);
        Assert_NoAssume(FMallocBinned::SizeClasses[ch->SizeClass] == blockSize);

        return (FBinnedBlock_*)((u8*)ch + FBinnedChunk_::ChunkSizeInBytes - i * blockSize);
    }

};
STATIC_ASSERT(sizeof(FBinnedChunk_) == CACHELINE_SIZE);
using FBinnedBatch_ = Meta::TAddPointer<FBinnedChunk_>;
//----------------------------------------------------------------------------
// FBinnedBucket_
//----------------------------------------------------------------------------
struct FBinnedBucket_ {
    FBinnedBlock_* FreeBlocks;
    u16 HighestIndex;
    u16 NumBlocksInUse;
    u32 NumBlocksAvailable;
    FBinnedChunk_* UsedChunks;
};
STATIC_ASSERT(Meta::is_pod_v<FBinnedBucket_>);
// using POD for fast access to buckets without singleton to help inline trivial allocations
static THREAD_LOCAL FBinnedBucket_ GBinnedThreadBuckets_[FMallocBinned::NumSizeClasses + 1/* extra for large blocks optimization */];
//----------------------------------------------------------------------------
// FBinnedGlobalCache_
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedGlobalCache_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, MaxFreeBatches, 4); // x 8 x 64kb = 2mb max reserve globally

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

    static FBinnedGlobalCache_& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBinnedGlobalCache_>, GInstance, 5000);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
// FBinnedThreadCache_
//----------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedThreadCache_ : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(size_t, MaxFreeChunks, 2); // x 64kb = 128kb max reserve per *thread* (NOT per *core*)
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
    VIRTUALMEMORYCACHE(VeryLargeBlocks, NumLargeBlocks, CacheSizeInBytes) VM;

    FBinnedLargeBlocks_() {}
    ~FBinnedLargeBlocks_();

    size_t RegionSize(void* ptr) const NOEXCEPT {
        return VM.RegionSize(ptr);
    }

    void* LargeAlloc(const size_t sizeInBytes) {
        Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_GRANULARITY, sizeInBytes));

        void* const newp = VM.Allocate(sizeInBytes);
        AssertRelease(newp);

#if USE_PPE_MEMORYDOMAINS
        Assert_NoAssume(VM.RegionSize(newp) == sizeInBytes);
        MEMORYDOMAIN_TRACKING_DATA(VeryLargeBlocks).AllocateUser(sizeInBytes);
#endif
        return newp;
    }

    void LargeFree(void* p, size_t sizeInBytes = 0) {
        Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_GRANULARITY, sizeInBytes));

#if USE_PPE_MEMORYDOMAINS
        if (0 == sizeInBytes)
            sizeInBytes = VM.RegionSize(p);

        MEMORYDOMAIN_TRACKING_DATA(VeryLargeBlocks).DeallocateUser(sizeInBytes);
#endif

        VM.Free(p, sizeInBytes);
    }

    static FBinnedLargeBlocks_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FBinnedLargeBlocks_, GInstance);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
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
    ch->HighestIndex = u16(-1);
    ch->NumBlocksInUse = u16(-1);
    ch->FreeBlocks = (FBinnedBlock_*)intptr_t(-1);
}
//----------------------------------------------------------------------------
struct FBinnedStats_ {
    std::atomic<i64> NumAllocs;
    std::atomic<i64> SizeInBytes;

    static FBinnedStats_& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBinnedStats_>, GStats, 2000);
        return GStats;
    }

    void OnAlloc(size_t sz) {
        NumAllocs++;
        SizeInBytes += checked_cast<i64>(FMallocBinned::SnapSize(sz));
    }

    void OnFree(size_t sz) {
        Verify(0 <= --NumAllocs);
        Verify(0 <= (SizeInBytes -= checked_cast<i64>(sz)));
    }

};
//----------------------------------------------------------------------------
#endif //!USE_PPE_ASSERT
//----------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------
static void* AllocateNewBinnedChunk_() {
    void* p;

#if USE_MALLOCBINNED_BITMAPS
    // better reuse the same memory than bitmaps, plus it's *WAY* faster
    p = FMallocBitmap::MediumAlloc(FBinnedChunk_::ChunkSizeInBytes, ALLOCATION_BOUNDARY);
    if (p)
        return p;
#endif
#if USE_MALLOCBINNED_MIPMAPS
    // better reuse the same memory than mipmaps, plus it's *WAY* faster
    p = FMallocMipMap::MediumAlloc(FBinnedChunk_::ChunkSizeInBytes, ALLOCATION_BOUNDARY);
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

#if USE_MALLOCBINNED_BITMAPS
    // need to check if the container is aliasing
    if (FMallocBitmap::AliasesToMediumHeap(p)) {
        FMallocBitmap::MediumFree(p);
        return;
    }
#endif
#if USE_MALLOCBINNED_MIPMAPS
    // need to check if the container is aliasing
    if (FMallocMipMap::AliasesToMediumMips(p)) {
        FMallocMipMap::MediumFree(p);
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
static FORCE_INLINE FBinnedChunk_* ChunkFromBlock_(FBinnedBlock_* const blk) {
    STATIC_ASSERT(Meta::IsPow2(FBinnedChunk_::ChunkSizeInBytes));
    Assert_NoAssume(uintptr_t(blk) & (FBinnedChunk_::ChunkSizeInBytes - 1));

    FBinnedChunk_* const ch = (FBinnedChunk_*)(uintptr_t(blk) & FBinnedChunk_::ChunkSizeMask);
    Assert_NoAssume(ch->CheckCanary());

    return ch;
}
//----------------------------------------------------------------------------
static void PokeChunkToFront_(FBinnedBucket_& bk, FBinnedChunk_* const ch) {
    Assert(ch);
    Assert(ch != bk.UsedChunks);
    Assert_NoAssume(ch->CheckCanary());
    Assert_NoAssume(ch->HighestIndex < ch->NumBlocksTotal);
    Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);
    Assert_NoAssume(ch->PrevChunk->CheckCanary());
    Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

    bk.UsedChunks->FreeBlocks = bk.FreeBlocks;
    bk.UsedChunks->HighestIndex = bk.HighestIndex;
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
    bk.HighestIndex = ch->HighestIndex;
    bk.NumBlocksInUse = ch->NumBlocksInUse;

    ONLY_IF_ASSERT(PoisonChunk_(ch)); // crash if we don't update back its state
}

//----------------------------------------------------------------------------
static size_t NonSmallBlockRegionSize_(void* const ptr) {
    Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_GRANULARITY, ptr));

#if USE_MALLOCBINNED_BITMAPS
    if (FMallocBitmap::AliasesToMediumHeap(ptr))
        return FMallocBitmap::MediumRegionSize(ptr);
    if (FMallocBitmap::AliasesToLargeHeap(ptr))
        return FMallocBitmap::LargeRegionSize(ptr);
#endif
#if USE_MALLOCBINNED_MIPMAPS
    if (FMallocMipMap::AliasesToMediumMips(ptr))
        return FMallocMipMap::MediumRegionSize(ptr);
    if (FMallocMipMap::AliasesToLargeMips(ptr))
        return FMallocMipMap::LargeRegionSize(ptr);
#endif

    return FBinnedLargeBlocks_::Get().RegionSize(ptr);
}
//----------------------------------------------------------------------------
static size_t NonSmallBlockSnapSize_(size_t sizeInBytes) {
    Assert_NoAssume(sizeInBytes > FBinnedChunk_::MaxSizeInBytes);

#if USE_MALLOCBINNED_BITMAPS
    sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);
    if (sizeInBytes <= FMallocBitmap::MediumMaxAllocSize)
        return FMallocBitmap::MediumSnapSize(sizeInBytes);
    else
        return FMallocBitmap::LargeSnapSize(sizeInBytes);

#elif USE_MALLOCBINNED_MIPMAPS
    sizeInBytes = ROUND_TO_NEXT_64K(sizeInBytes);
    if (sizeInBytes <= FMallocMipMap::MediumMaxAllocSize)
        return FMallocMipMap::MediumSnapSize(sizeInBytes);
    else
        return FMallocMipMap::LargeSnapSize(sizeInBytes);

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
        Assert_NoAssume(batch->HighestIndex < batch->NumBlocksTotal);
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
#if USE_PPE_ASSERT
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
    const Meta::FLockGuard scopeLock(lb.Barrier);
    lb.VM.ReleaseAll();
}
//----------------------------------------------------------------------------
static void BinnedFree_(FBinnedBucket_& bk, FBinnedChunk_* ch, FBinnedBlock_* blk);
static NO_INLINE bool ReleaseDanglingBlocks_(FBinnedThreadCache_& tc) {
    const FAtomicSpinLock::FTryScope scopeLock(tc.DanglingBarrier);

    if (scopeLock.Locked) { // lazy locking to minimize thread contention
        FBinnedBlock_* blk = tc.DanglingBlocks;
        if (Likely(blk)) {
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
    FBinnedGlobalCache_& gc = FBinnedGlobalCache_::Get();

    if (gc.FreeBatches) {
        const FAtomicSpinLock::FScope scopeLock(gc.Barrier);

        if (FBinnedBatch_ const batch = gc.FreeBatches) {
            Assert(gc.NumFreeBatches);
            Assert_NoAssume(batch->CheckCanary());
            Assert_NoAssume(batch->NumChunksInBatch);
            Assert_NoAssume(nullptr == batch->ThreadCache);
            Assert_NoAssume(0 == batch->NumBlocksInUse);
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
static void InitializeChunk_(FBinnedChunk_* const ch, const size_t sizeClass) {
    Assert(ch);
    Assert_NoAssume(0 == ch->NumBlocksInUse);

    const size_t blockSize = FMallocBinned::SizeClasses[sizeClass];
    Assert_NoAssume(Meta::IsAlignedPow2(16, blockSize));

    ch->SizeClass = u32(sizeClass);;
    ch->NumBlocksTotal = u32(FBinnedChunk_::ChunkAvailableSizeInBytes / blockSize);
    ch->HighestIndex = checked_cast<u16>(ch->NumBlocksTotal - 1);
    ch->FreeBlocks = FBinnedChunk_::BlockFromIndex(ch, ch->NumBlocksTotal, blockSize);
    Assert_NoAssume(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, ch->FreeBlocks));

    // FreeBlocks linked list is lazily initialized thanks to HighestIndex
    // It avoids traversing all the chunk to set all Next fields, and thus avoid trashing
    // the cache. It's important since InitializeChunk_() can be call often due to chunk
    // stealing and recycling mechanics.
    ch->FreeBlocks->Next = nullptr;
}
//----------------------------------------------------------------------------
static void* AllocateBlockFromChunk_(FBinnedChunk_* const ch, size_t sizeClass) {
    Assert(ch);
    Assert_NoAssume(sizeClass == ch->SizeClass);
    Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);

    void* p;
    if (Likely(ch->FreeBlocks)) {
        p = ch->FreeBlocks;
        ch->FreeBlocks = ch->FreeBlocks->Next;
    }
    else {
        Assert(ch->HighestIndex);
        const size_t blockSize = FMallocBinned::SizeClasses[sizeClass];
        p = FBinnedChunk_::BlockFromIndex(ch, ch->HighestIndex--, blockSize);
    }

    ch->NumBlocksInUse++;
    return p;
}
//----------------------------------------------------------------------------
static NO_INLINE void* BinnedMalloc_(FBinnedBucket_& bk, size_t size, size_t sizeClass) {
    Assert(size);

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
                    Assert(ch->FreeBlocks || ch->HighestIndex);

                    // steal one free block for current allocation
                    void* const p = AllocateBlockFromChunk_(ch, sizeClass);
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
                ONLY_IF_ASSERT(FBinnedStats_::Get().OnFree(FMallocBinned::SizeClasses[sizeClass]));

                // recurse to Malloc() if we freed some blocks in the current bucket
                return FMallocBinned::Malloc(size);
            }

            FBinnedChunk_* ch;

            // try to fetch from thread local cache
            if (Likely(tc.FreeChunks)) {
                Assert(tc.NumChunksFreed);
                Assert_NoAssume(tc.FreeChunks->CheckCanary());
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
                    ch->HighestIndex = 0;
                    ch->NumBlocksInUse = 0;
                    ch->NumChunksInBatch = 0;
                    ch->NextBatch = nullptr;
#if USE_PPE_ASSERT
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
                bk.UsedChunks->HighestIndex = bk.HighestIndex;
                bk.UsedChunks->NumBlocksInUse = bk.NumBlocksInUse;
                bk.UsedChunks->FreeBlocks = nullptr;
                bk.UsedChunks->PrevChunk = ch;
            }

            // allocate the first block for current request
            void* const p = AllocateBlockFromChunk_(ch, sizeClass);

            // then register and mirror new chunk in the bucket
            bk.FreeBlocks = ch->FreeBlocks;
            bk.HighestIndex = ch->HighestIndex;
            bk.NumBlocksInUse = ch->NumBlocksInUse;
            bk.NumBlocksAvailable += (ch->NumBlocksTotal - ch->NumBlocksInUse);
            bk.UsedChunks = ch;

            ONLY_IF_ASSERT(PoisonChunk_(ch));
            Assert_NoAssume(uintptr_t(p) & (FBinnedChunk_::ChunkSizeInBytes - 1));
            return p;
        }
    }
    // for large blocks, align on 64kb :
    else {
        Assert_NoAssume(size > FBinnedChunk_::MaxSizeInBytes);

        void* p = nullptr;
#if USE_MALLOCBINNED_BITMAPS
        p = FMallocBitmap::HeapAlloc(size, ALLOCATION_BOUNDARY).Data;

        if (nullptr == p) // also handle bitmap OOM
#endif
#if USE_MALLOCBINNED_MIPMAPS
        p = FMallocMipMap::MipAlloc(size, ALLOCATION_BOUNDARY);

        if (nullptr == p) // also handle mipmap OOM
#endif
        {
            STATIC_ASSERT(ALLOCATION_GRANULARITY == 64 * 1024);
            p = FBinnedLargeBlocks_::Get().LargeAlloc(ROUND_TO_NEXT_64K(size));
        }

        Assert_NoAssume(Meta::IsAlignedPow2(FBinnedChunk_::ChunkSizeInBytes, p));
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

    FBinnedGlobalCache_& gc = FBinnedGlobalCache_::Get();

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

    // polling to avoid dead lock (it's common to have many thread dying at the same time)
    for (;;) {
        // first lock the chunk (only useful for dying thread caches)
        FAtomicSpinLock::FUniqueLock lockChunk(ch->ThreadBarrier);

        // test if the chunk is dangling
        if (Likely(ch->ThreadCache)) {
            FBinnedThreadCache_& oc = (*ch->ThreadCache);

            // then lock the thread cache before adding the pending block
            const FAtomicSpinLock::FTryScope lockThread(oc.DanglingBarrier);
            if (not lockThread.Locked)
                continue; // try again, but release the chunk barrier to avoid dead lock

            Assert_NoAssume(ch->CheckCanary());
            Assert_NoAssume(&oc == ch->ThreadCache);
            Assert_NoAssume(FBinnedThreadCache_::InvalidBlockRef != uintptr_t(oc.DanglingBlocks));

            blk->Next = oc.DanglingBlocks;
            oc.DanglingBlocks = blk;

            return; // early out since chunk can't be stolen
        }
        // else the chunk itself is dangling
        else {
            FBinnedGlobalCache_& gc = FBinnedGlobalCache_::Get();

            FAtomicSpinLock::FTryScope lockGlobal(gc.Barrier);
            if (not lockGlobal.Locked)
                continue; // try again, but release the chunk barrier to avoid dead lock

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
            bk.UsedChunks->HighestIndex = bk.HighestIndex;
            bk.UsedChunks->NumBlocksInUse = bk.NumBlocksInUse;
            bk.UsedChunks->FreeBlocks = bk.FreeBlocks;
            bk.UsedChunks->PrevChunk = ch;
        }

        // register the new free blocks as head of the used chunks
        bk.FreeBlocks = ch->FreeBlocks;
        bk.HighestIndex = ch->HighestIndex;
        bk.NumBlocksInUse = ch->NumBlocksInUse;
        bk.NumBlocksAvailable += (ch->NumBlocksTotal - ch->NumBlocksInUse);
        bk.UsedChunks = ch;

        ONLY_IF_ASSERT(PoisonChunk_(ch));

        break;
    }

    // needed to avoid an error due to call to FMallocBinned::Free() below
    ONLY_IF_ASSERT(FBinnedNoReentrancy_ noReentrancy);
    ONLY_IF_ASSERT(FBinnedStats_::Get().OnAlloc(FMallocBinned::SizeClasses[ch->SizeClass]));

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

        // if it's the head chunk then we need to release the chunk
        if (Unlikely(bk.UsedChunks == ch)) {
            Assert(bk.NumBlocksInUse == 1);
            Assert_NoAssume(nullptr == ch->PrevChunk);
            Assert_NoAssume(bk.NumBlocksAvailable >= (ch->NumBlocksTotal - 1));
            Assert_NoAssume(ch->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

            bk.NumBlocksAvailable -= (ch->NumBlocksTotal - 1);
            bk.UsedChunks = ch->NextChunk;

            blk->Next = bk.FreeBlocks;
            ch->FreeBlocks = blk;
            ch->HighestIndex = bk.HighestIndex;
            ch->NumBlocksInUse = 0;
            ch->ThreadCache = nullptr;
            ch->NextChunk = tc.FreeChunks;

            if (bk.UsedChunks) {
                Assert_NoAssume(bk.UsedChunks->CheckCanary());
                Assert_NoAssume(ch == bk.UsedChunks->PrevChunk);
                Assert_NoAssume(bk.UsedChunks->NumBlocksInUse);

                bk.UsedChunks->PrevChunk = nullptr;
                bk.FreeBlocks = bk.UsedChunks->FreeBlocks;
                bk.HighestIndex = bk.UsedChunks->HighestIndex;
                bk.NumBlocksInUse = bk.UsedChunks->NumBlocksInUse;

                ONLY_IF_ASSERT(PoisonChunk_(bk.UsedChunks));
            }
            else {
                Assert_NoAssume(0 == bk.NumBlocksAvailable);

                bk.FreeBlocks = nullptr;
                bk.HighestIndex = 0;
                bk.NumBlocksInUse = 0;
            }

            tc.FreeChunks = ch;
            tc.NumChunksFreed++;
        }
        // else we release the block and check if the chunk also needs to be released
        else {
            Assert(ch->PrevChunk); // since ch != bk.UsedChunks
            Assert(ch->NumBlocksInUse);
            Assert_NoAssume(ch->HighestIndex < ch->NumBlocksTotal);
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
static NO_INLINE void LargeFree_(void* const ptr) {
    AssertRelease(ptr);

#if USE_MALLOCBINNED_BITMAPS
    if (FMallocBitmap::AliasesToHeaps(ptr))
        FMallocBitmap::HeapFree(FAllocatorBlock(ptr, 0));
    else
#elif USE_MALLOCBINNED_MIPMAPS
    if (FMallocMipMap::AliasesToMips(ptr))
        FMallocMipMap::MipFree(ptr);
    else
#endif
    {
        FBinnedLargeBlocks_::Get().LargeFree(ptr);
    }
}
//----------------------------------------------------------------------------
NODISCARD static NO_INLINE void* LargeRealloc_(void* const oldp, size_t newSize, size_t oldSize) {
    Assert(oldp);
    Assert(newSize);
    Assert(oldSize);
    Assert(newSize > FBinnedChunk_::MaxSizeInBytes);
    Assert(oldSize > FBinnedChunk_::MaxSizeInBytes);

    // only called when both new and old size are large blocks

    void* newp = nullptr;
#if USE_MALLOCBINNED_BITMAPS && 0
    if (oldSize <= FMallocBitmap::MaxAllocSize &&
        FMallocBitmap::AliasesToHeaps(oldp))
        newp = FMallocBitmap::HeapResize(FAllocatorBlock(oldp, oldSize), newSize).Data; // try to resize the mipmap allocation
    else
        Assert_NoAssume(FMallocBitmap::AliasesToHeaps(oldp) == false);

    if (Unlikely(nullptr == newp)) // if resize failed fallback on alloc+free (specialized for large allocs)
#elif USE_MALLOCBINNED_MIPMAPS
    if (oldSize <= FMallocMipMap::MipMaxAllocSize &&
        FMallocMipMap::AliasesToMips(oldp) )
        newp = FMallocMipMap::MipResize(oldp, newSize, oldSize); // try to resize the mipmap allocation
    else
        Assert_NoAssume(FMallocMipMap::AliasesToMips(oldp) == false);

    if (Unlikely(nullptr == newp)) // if resize failed fallback on alloc+free (specialized for large allocs)
#endif
    {
#if USE_MALLOCBINNED_BITMAPS
        newp = FMallocBitmap::HeapAlloc(newSize, ALLOCATION_BOUNDARY).Data;

        if (nullptr == newp) // also handle mip map OOM
#elif USE_MALLOCBINNED_MIPMAPS
        newp = FMallocMipMap::MipAlloc(newSize, ALLOCATION_BOUNDARY);

        if (nullptr == newp) // also handle mip map OOM
#endif
        {
            STATIC_ASSERT(ALLOCATION_GRANULARITY == 64 * 1024);
            newp = FBinnedLargeBlocks_::Get().LargeAlloc(ROUND_TO_NEXT_64K(newSize));
        }

        // need to align on 16 for Memstream()
        const size_t cpySize = ROUND_TO_NEXT_16(Min(oldSize, newSize));
#if USE_MALLOCBINNED_BITMAPS
        Assert_NoAssume(cpySize <= FMallocBitmap::RegionSize(newp));
#elif USE_MALLOCBINNED_MIPMAPS
        Assert_NoAssume(cpySize <= FMallocMipMap::RegionSize(newp));
#endif

        // copy previous data to new block without polluting caches
        FPlatformMemory::MemstreamLarge(newp, oldp, cpySize);

#if USE_MALLOCBINNED_BITMAPS
        if (FMallocBitmap::AliasesToHeaps(oldp))
            FMallocBitmap::HeapFree(FAllocatorBlock(oldp, 0));
        else
#elif USE_MALLOCBINNED_MIPMAPS
        if (FMallocMipMap::AliasesToMips(oldp))
            FMallocMipMap::MipFree(oldp);
        else
#endif
        {
            FBinnedLargeBlocks_::Get().LargeFree(oldp);
        }
    }

    return newp;
}
//----------------------------------------------------------------------------
// Destructors
//----------------------------------------------------------------------------
FBinnedThreadCache_::~FBinnedThreadCache_() {
    // let other thread release blocks belonging to this thread, if any
    for (i32 backoff = 0;;) {
        ReleaseDanglingBlocks_(*this);
        FPlatformProcess::SleepForSpinning(backoff);
        if (nullptr == DanglingBlocks)
            break;
    }

    FPlatformAtomics::MemoryBarrier();

    // invalid ptr to be sure than it won't be accessed
    FAtomicSpinLock::FScope scopeLock(DanglingBarrier);
    Assert_NoAssume(nullptr == DanglingBlocks);
    DanglingBlocks = (FBinnedBlock_*)InvalidBlockRef;

    ReleaseFreeChunks_(*this);

    FBinnedGlobalCache_& gc = FBinnedGlobalCache_::Get();

    // need to refurbish chunks with blocks still allocated
    forrange(bk, &GBinnedThreadBuckets_[0], &GBinnedThreadBuckets_[FMallocBinned::NumSizeClasses]) {
        if (FBinnedChunk_* ch = bk->UsedChunks) {
            const FAtomicSpinLock::FScope lockGlobal(gc.Barrier);

            Assert_NoAssume(ch->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

            // don't forget to sync the head with the bucket
            bk->UsedChunks->HighestIndex = bk->HighestIndex;
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
                Assert_NoAssume(ch->HighestIndex < ch->NumBlocksTotal);
                Assert_NoAssume(ch->NumBlocksInUse <= ch->NumBlocksTotal);
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
            Assert_NoAssume(ch->HighestIndex < ch->NumBlocksTotal);
            Assert_NoAssume(ch->NumBlocksInUse < ch->NumBlocksTotal);
            Assert_NoAssume(nullptr == ch->NextBatch);
            Assert_NoAssume(0 == ch->NumChunksInBatch);
            Assert_NoAssume(not ch->NextChunk || ch->NextChunk->PrevChunk == ch);

            numDanglingBlocks += ch->NumBlocksInUse;
            numDanglingChunks++;
            totalSizeDangling += (size_t(FMallocBinned::SizeClasses[ch->SizeClass]) * ch->NumBlocksInUse);

#if USE_PPE_ASSERT
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
    ONLY_IF_ASSERT(const i64 binnedAllocCount = FBinnedStats_::Get().NumAllocs);
    ONLY_IF_ASSERT(const i64 binnedAllocSizeInBytes = FBinnedStats_::Get().SizeInBytes);
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
    AssertRelease(size);
    ONLY_IF_ASSERT(FBinnedStats_::Get().OnAlloc(size));

    const size_t sizeClass = MakeBinnedClass_(size);

    // SizeClass <NumSizeClasses> won't ever be filled, allows to make only one test
    FBinnedBucket_& bk = GBinnedThreadBuckets_[Min(sizeClass, FMallocBinned::NumSizeClasses)];

    FBinnedBlock_* blk = bk.FreeBlocks;
    if (Likely(!!blk | !!bk.HighestIndex)) {
        Assert(bk.NumBlocksAvailable);
        Assert_NoAssume(size <= FMallocBinned::SizeClasses[sizeClass]);
        Assert_NoAssume(bk.NumBlocksInUse < bk.UsedChunks->NumBlocksTotal);
        Assert_NoAssume(bk.UsedChunks->FreeBlocks == (FBinnedBlock_*)intptr_t(-1));

        if (blk)
            bk.FreeBlocks = blk->Next;
        else // lazily initialize the chunk free list
            blk = FBinnedChunk_::BlockFromIndex(bk.UsedChunks, bk.HighestIndex--, FMallocBinned::SizeClasses[sizeClass]);

        Assert_NoAssume(ChunkFromBlock_(blk) == bk.UsedChunks);

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
void FMallocBinned::Free(void* const ptr) {
    AssertRelease(ptr);
    ONLY_IF_ASSERT(FBinnedStats_::Get().OnFree(RegionSize(ptr)));

    if (Likely(uintptr_t(ptr) & (FBinnedChunk_::ChunkSizeInBytes - 1))) {
        FBinnedBlock_* const blk = (FBinnedBlock_*)ptr;
        FBinnedChunk_* const ch = ChunkFromBlock_(blk);
        Assert(ch->SizeClass < FMallocBinned::NumSizeClasses);

        FBinnedBucket_& bk = GBinnedThreadBuckets_[ch->SizeClass];

        if (Likely((bk.UsedChunks == ch) & (bk.NumBlocksInUse > 1))) {
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
        LargeFree_(ptr);
    }
}
//----------------------------------------------------------------------------
void* FMallocBinned::Realloc(void* const ptr, size_t size) {
    AssertRelease(ptr || size);

    if (Likely(ptr)) {
        void* newp = nullptr;

        if (Likely(size)) {
            const size_t old = FMallocBinned::RegionSize(ptr);
            Assert_NoAssume(SnapSize(old) == old);

            // skip reallocation if no growth is needed
            if (Unlikely(SnapSize(size) == old))
                return ptr;

            // for small blocks < 32kb:
            if (Likely((size <= FMallocBinned::MaxSmallBlockSize) | (old <= FMallocBinned::MaxSmallBlockSize))) {
                // get a new block
                newp = FMallocBinned::Malloc(size);

                // need to align on 16 for Memstream()
                const size_t cpy = ROUND_TO_NEXT_16(Min(old, size));
                Assert_NoAssume(cpy <= RegionSize(newp));

                // copy previous data to new block without polluting caches
				FPlatformMemory::Memstream(newp, ptr, cpy);
            }
			// for large blocks we get other opportunities:
            else {
                ONLY_IF_ASSERT(FBinnedStats_::Get().OnFree(old));
                ONLY_IF_ASSERT(FBinnedStats_::Get().OnAlloc(size));

                return LargeRealloc_(ptr, size, old);
            }
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
    void* const p = FMallocBinned::Malloc(Meta::RoundToNextPow2(size, alignment));
    Assert_NoAssume(Meta::IsAlignedPow2(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
void FMallocBinned::AlignedFree(void* const ptr) {
    return FMallocBinned::Free(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBinned::AlignedRealloc(void* const ptr, size_t size, size_t alignment) {
    void* const p = FMallocBinned::Realloc(ptr, Meta::RoundToNextPow2(size, alignment));
    Assert_NoAssume(0 == size || Meta::IsAlignedPow2(alignment, p));
    return p;
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleaseCacheMemory() {
    PPE_LOG(MallocBinned, Debug, "release cache memory in {0}", std::this_thread::get_id());

    auto& tc = FBinnedThreadCache_::Get();
    ReleaseDanglingBlocks_(tc);
    ReleaseFreeChunks_(tc);

    ReleaseFreeBatches_(FBinnedGlobalCache_::Get());
    ReleaseLargeBlocks_(FBinnedLargeBlocks_::Get());

#if USE_MALLOCBINNED_BITMAPS
    FMallocBitmap::MemoryTrim();
#endif
#if USE_MALLOCBINNED_MIPMAPS
    FMallocMipMap::MemoryTrim();
#endif
}
//----------------------------------------------------------------------------
void FMallocBinned::ReleasePendingBlocks() {
    //PPE_LOG(MallocBinned, Debug, "release pending blocks in {0}", std::this_thread::get_id()); // too verbose

    ReleaseDanglingBlocks_(FBinnedThreadCache_::Get());
}
//----------------------------------------------------------------------------
size_t FMallocBinned::SnapSize(size_t size) NOEXCEPT {
    Assert(size);

    const size_t sizeClass = MakeBinnedClass_(size);
    return (Likely(sizeClass < FMallocBinned::NumSizeClasses)
        ? FMallocBinned::SizeClasses[sizeClass]
        : NonSmallBlockSnapSize_(size) );
}
//----------------------------------------------------------------------------
size_t FMallocBinned::RegionSize(void* ptr) {
    Assert(ptr);

    return (Likely(not Meta::IsAlignedPow2(FBinnedChunk_::ChunkSizeInBytes, ptr))
        ? FMallocBinned::SizeClasses[ChunkFromBlock_((FBinnedBlock_*)ptr)->SizeClass]
        : NonSmallBlockRegionSize_(ptr) );
}
//----------------------------------------------------------------------------
size_t FMallocBinned::SizeClass(size_t size) NOEXCEPT {
    return MakeBinnedClass_(SnapSize(size));
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocBinned::DumpMemoryInfo(FWTextWriter& oss) {
#if USE_MALLOCBINNED_BITMAPS
    FMallocBitmap::DumpHeapInfo(oss);
#elif USE_MALLOCBINNED_MIPMAPS
    FMallocMipMap::DumpMemoryInfo(oss);
#endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_MSVC_WARNING_POP()
