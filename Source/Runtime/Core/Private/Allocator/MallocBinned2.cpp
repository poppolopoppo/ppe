﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/MallocBinned2.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/InitSegAllocator.h"
#include "Allocator/MallocBitmap.h"

#include "Container/BitTree.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformMemory.h"
#include "Meta/ThreadResource.h"
#include "Meta/Utility.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Thread/CriticalSection.h"

#if USE_PPE_ASSERT
#   include "Diagnostic/DecodedCallstack.h"
#endif
#if USE_PPE_MEMORYDOMAINS
#   include "IO/Format.h"
#   include "IO/StaticString.h"
#   include "Memory/MemoryTracking.h"
#endif
#if USE_PPE_LOGGER
#   include "Thread/ThreadContext.h"
#endif

#define PPE_MALLOCBINNED2_USE_DETAILED_TRACKING (0) // enable/disable per pool tracking data
#define PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS (1) // enable/disable FMallocBitmap

#define PPE_MALLOCBINNED2_BOUNDARY (ALLOCATION_BOUNDARY)

#define PPE_MALLOCBINNED2_BUNDLE_MAX_COUNT (64u)
#define PPE_MALLOCBINNED2_BUNDLE_MAX_SIZE (8_KiB)
#define PPE_MALLOCBINNED2_BUNDLE_MAX_GARBAGE (8u)

#define PPE_MALLOCBINNED2_OS_PAGESIZE (FPlatformMemory::PageSize)
#define PPE_MALLOCBINNED2_OS_GRANULARITY (FPlatformMemory::AllocationGranularity)

#define PPE_MALLOCBINNED2_CHUNK_MAXSIZE (2u*PPE_MALLOCBINNED2_OS_GRANULARITY)
#define PPE_MALLOCBINNED2_CHUNK_OVERHEADSIZE sizeof(FBinnedSmallTable::FPoolChunk)
#define PPE_MALLOCBINNED2_CHUNK_MINBLOCKS (3u)
#define PPE_MALLOCBINNED2_CHUNK_MAXBLOCKS (1000u)

#define PPE_MALLOCBINNED2_SMALLPOOL_COUNT (FMallocBinned2::NumSmallBlockSizes)
#define PPE_MALLOCBINNED2_SMALLPOOL_MIN_SIZE (PPE_MALLOCBINNED2_BOUNDARY)
#define PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE (FMallocBinned2::MaxSmallBlockSize)
#define PPE_MALLOCBINNED2_SMALLPOOL_RESERVE (CODE3264(16_MiB,512_MiB))

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment

PRAGMA_DISABLE_RUNTIMECHECKS // %_NOCOMMIT%

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, MallocBinned2)
//------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
namespace {
//------------------------------------------------------------------------------
struct FBinnedBundleNode;
struct FBinnedBundle;
struct FBinnedGlobalRecycler;
struct FBinnedFreeBlockList;
struct FBinnedPerThreadFreeBlockList;
struct FBinnedSmallTable;
//------------------------------------------------------------------------------
// FBinnedBundleNode
//------------------------------------------------------------------------------
struct ALIGN(PPE_MALLOCBINNED2_SMALLPOOL_MIN_SIZE) FBinnedBundleNode {
    FBinnedBundleNode* NextNode;
    union {
        FBinnedBundleNode* NextBundle;
        u32 Count;
    };
};
STATIC_ASSERT(sizeof(FBinnedBundleNode) == PPE_MALLOCBINNED2_SMALLPOOL_MIN_SIZE);
//------------------------------------------------------------------------------
// FBinnedBundle
//------------------------------------------------------------------------------
struct FBinnedBundle {
    STATIC_CONST_INTEGRAL(u32, MaxCount, PPE_MALLOCBINNED2_BUNDLE_MAX_COUNT);
    STATIC_CONST_INTEGRAL(u32, MaxSizeInBytes, PPE_MALLOCBINNED2_BUNDLE_MAX_SIZE);

    FBinnedBundleNode* Head;
    u32 Count;

    FORCE_INLINE FBinnedBundle() {
        Reset();
    }

    FORCE_INLINE bool Full(u32 blockSize) const {
        return (Count >= FBinnedBundle::MaxCount || Count * blockSize >= FBinnedBundle::MaxSizeInBytes);
    }

    FORCE_INLINE void Reset() {
        Head = nullptr;
        Count = 0;
    }

    FORCE_INLINE void PushHead(FBinnedBundleNode* node) {
        Assert(node);
        node->NextNode = Head;
        node->NextBundle = nullptr;
        Head = node;
        ++Count;
    }

    FORCE_INLINE FBinnedBundleNode* PopHead() {
        Assert(Head);
        Assert(Count);

        FBinnedBundleNode* result = Head;
        Head = Head->NextNode;
        --Count;

#if USE_PPE_ASSERT
        if (result) // poison pointers in debug
            FPlatformMemory::Memuninitialized(result, sizeof(*result));
#endif

        return result;
    }
};
//------------------------------------------------------------------------------
// FBinnedGlobalRecycler
//------------------------------------------------------------------------------
struct FBinnedGlobalRecycler : Meta::FNonCopyableNorMovable {
    STATIC_CONST_INTEGRAL(u32, MaxGarbage, PPE_MALLOCBINNED2_BUNDLE_MAX_GARBAGE);
    struct CACHELINE_ALIGNED FBundleGarbage {
        FBinnedBundleNode* FreeBundles[MaxGarbage] = { 0 };
    };
    CACHELINE_ALIGNED FBundleGarbage Pools[PPE_MALLOCBINNED2_SMALLPOOL_COUNT];

    bool PushBundle(u32 pool, FBinnedBundleNode* bundle) {
        Assert(pool < lengthof(Pools));
        Assert(bundle);

        FBundleGarbage& p = Pools[pool];
        forrange(i, 0, MaxGarbage) {
            if (!p.FreeBundles[i]) {
                if (not FPlatformAtomics::CompareExchangePtr<void>((volatile void**)&p.FreeBundles[i], bundle, nullptr)) {
                    FPlatformAtomics::MemoryBarrier();
                    return true;
                }
            }
        }

        return false;
    }

    FBinnedBundleNode* PopBundle(u32 pool) {
        Assert(pool < lengthof(Pools));

        FBundleGarbage& p = Pools[pool];
        forrange(i, 0, MaxGarbage) {
            if (FBinnedBundleNode* result = p.FreeBundles[i]) {
                if (FPlatformAtomics::CompareExchangePtr<void>((volatile void**)&p.FreeBundles[i], nullptr, result) == result) {
                    FPlatformAtomics::MemoryBarrier();
                    return result;
                }
            }
        }

        return nullptr;
    }

    u32 PopBundles(u32 pool, FBundleGarbage* gc) {
        Assert(pool < lengthof(Pools));

        u32 n = 0;
        FBundleGarbage& p = Pools[pool];
        forrange(i, 0, MaxGarbage) {
            while (FBinnedBundleNode* result = p.FreeBundles[i]) {
                if (FPlatformAtomics::CompareExchangePtr<void>((volatile void**)&p.FreeBundles[i], nullptr, result) == result) {
                    FPlatformAtomics::MemoryBarrier();
                    gc->FreeBundles[n++] = result;
                }
            }
        }

        return n;
    }

};
static FBinnedGlobalRecycler GBinnedGlobalRecycler;
//------------------------------------------------------------------------------
// FBinnedFreeBlockList
//------------------------------------------------------------------------------
struct FBinnedFreeBlockList {
    FBinnedBundle PartialBundle;
    FBinnedBundle FullBundle;

    NODISCARD FORCE_INLINE bool PushToFront(void* ptr, u32 blockSize) {
        Assert(ptr);
        Assert_NoAssume(blockSize >= sizeof(FBinnedBundleNode));
        if (PartialBundle.Full(blockSize)) {
            if (FullBundle.Head)
                return false;

            FullBundle = PartialBundle;
            PartialBundle.Reset();
        }
        PartialBundle.PushHead((FBinnedBundleNode*)ptr);
        return true;
    }
    NODISCARD FORCE_INLINE bool CanPushToFront(u32 blockSize) const {
        return !(!!FullBundle.Head && PartialBundle.Full(blockSize));
    }
    FORCE_INLINE void* PopFromFront() {
        if (Unlikely((!PartialBundle.Head) & (!!FullBundle.Head))) {
            Assert_NoAssume(PartialBundle.Count == 0);
            PartialBundle = FullBundle;
            FullBundle.Reset();
        }
        return (PartialBundle.Head ? PartialBundle.PopHead() : nullptr);
    }

    NODISCARD bool ObtainPartial(u32 pool);
    void PushBundle(FBinnedBundle&& rbundle);
    NODISCARD FBinnedBundleNode* PopBundles();
    // tries to recycle the full bundle, if that fails, it is returned for freeing
    NODISCARD FBinnedBundleNode* RecycleFull(u32 pool);
};
//------------------------------------------------------------------------------
bool FBinnedFreeBlockList::ObtainPartial(u32 pool) {
    if (not PartialBundle.Head) {
        Assert_NoAssume(PartialBundle.Count == 0);

        if (FullBundle.Head) {
            PartialBundle = FullBundle;
            FullBundle.Reset();

            return  true;
        }

        PartialBundle.Count = 0;
        PartialBundle.Head = GBinnedGlobalRecycler.PopBundle(pool);

        if (PartialBundle.Head) {
            PartialBundle.Count = PartialBundle.Head->Count;
            PartialBundle.Head->NextBundle = nullptr;

            Assert_NoAssume(PartialBundle.Count > 0);
            return true;
        }

        Assert_NoAssume(not FullBundle.Head);
        Assert_NoAssume(not PartialBundle.Head);
        return false;
    }

    Assert_NoAssume(PartialBundle.Head);
    Assert_NoAssume(PartialBundle.Count > 0);
    return true;
}
//------------------------------------------------------------------------------
void FBinnedFreeBlockList::PushBundle(FBinnedBundle&& rbundle) {
    Assert_NoAssume(rbundle.Head);
    Assert_NoAssume(rbundle.Count > 0);
    Assert_NoAssume(not PartialBundle.Head);
    Assert_NoAssume(not FullBundle.Head);

    PartialBundle = std::move(rbundle);
}
//------------------------------------------------------------------------------
FBinnedBundleNode* FBinnedFreeBlockList::PopBundles() {
    FBinnedBundleNode* Partial = PartialBundle.Head;
    if (Partial) {
        PartialBundle.Reset();
        Partial->NextBundle = nullptr;
    }

    FBinnedBundleNode* Full = FullBundle.Head;
    if (Full) {
        FullBundle.Reset();
        Full->NextBundle = nullptr;
    }

    FBinnedBundleNode* Result = Partial;
    if (Result)
        Result->NextBundle = Full;
    else
        Result = Full;

    return Result;
}
//------------------------------------------------------------------------------
FBinnedBundleNode* FBinnedFreeBlockList::RecycleFull(u32 pool) {
    FBinnedBundleNode* result = nullptr;
    if (FullBundle.Head) {
        FullBundle.Head->Count = FullBundle.Count;

        if (not GBinnedGlobalRecycler.PushBundle(pool, FullBundle.Head)) {
            result = FullBundle.Head;
            result->NextBundle = nullptr;
        }

        FullBundle.Reset();
    }
    return result;
}
//------------------------------------------------------------------------------
// FBinnedTrackingData
//------------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS && PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
//------------------------------------------------------------------------------
struct CACHELINE_ALIGNED FBinnedTrackingData {
    TStaticString<32> Name;
    FMemoryTracking Data;
};
static CACHELINE_ALIGNED FBinnedTrackingData GBinnedTrackingDataPerPool[PPE_MALLOCBINNED2_SMALLPOOL_COUNT];
//------------------------------------------------------------------------------
static void BinnedTrackingDataStart_() {
    forrange(pool, 0, PPE_MALLOCBINNED2_SMALLPOOL_COUNT) {
        FBinnedTrackingData& tracking = GBinnedTrackingDataPerPool[pool];
        tracking.Name.Len = Format(tracking.Name.Data, "Pool[{0:#5}]", FMallocBinned2::SmallPoolIndexToBlockSize(pool));
        tracking.Data.Reparent(tracking.Name.NullTerminated(), &MEMORYDOMAIN_TRACKING_DATA(SmallTables));
        RegisterTrackingData(&tracking.Data);
    }
}
//------------------------------------------------------------------------------
static void BinnedTrackingDataShutdown_() {
    forrange(pool, 0, PPE_MALLOCBINNED2_SMALLPOOL_COUNT) {
        FBinnedTrackingData& tracking = GBinnedTrackingDataPerPool[pool];
        UnregisterTrackingData(&tracking.Data);
    }
}
//------------------------------------------------------------------------------
#endif //!USE_PPE_MEMORYDOMAINS
//------------------------------------------------------------------------------
// Small pools
//------------------------------------------------------------------------------
struct FBinnedSmallTable : Meta::FNonCopyableNorMovable {

    NORETURN static void OnOutOfMemory(u32 sizeInBytes) {
        FPlatformMemory::OnOutOfMemory(sizeInBytes, PPE_MALLOCBINNED2_BOUNDARY);
        AssertNotReached();
    }

    static inline byte* GSmallPoolVirtualMemory{ static_cast<byte*>(nullptr) };

    FORCE_INLINE static bool IsSmallBlock(const void* p) NOEXCEPT {
        const size_t delta = static_cast<size_t>(static_cast<const byte*>(p) - GSmallPoolVirtualMemory);
        return (!!GSmallPoolVirtualMemory &&
            (delta < PPE_MALLOCBINNED2_SMALLPOOL_COUNT * PPE_MALLOCBINNED2_SMALLPOOL_RESERVE) );
    }

    FORCE_INLINE static u32 SmallPoolIndexFromPtr(const void* p) NOEXCEPT {
        Assert(GSmallPoolVirtualMemory);
        const ptrdiff_t delta = (static_cast<const byte*>(p) - GSmallPoolVirtualMemory);
        Assert(delta >= 0);

        const u32 pool = checked_cast<u32>(delta / PPE_MALLOCBINNED2_SMALLPOOL_RESERVE);
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        return pool;
    }

    struct FSmallPoolInfo;
    struct CACHELINE_ALIGNED FPoolChunk {
        FBinnedBundleNode* FullBundles;
        FBinnedBundle PartialBundle;

        u16 NumBlocksTaken;
        u16 HighestBundleUnused;

        FORCE_INLINE byte* BundleData() const {
            const void* p = (this + 1);
            return static_cast<byte*>(const_cast<void*>(p));
        }

        FORCE_INLINE void InitializeBundles() {
            Assert_NoAssume(IsSmallBlock(this));

            FullBundles = nullptr;
            PartialBundle.Reset();
            NumBlocksTaken = 0;
            HighestBundleUnused = 0;
        }

        NODISCARD FORCE_INLINE FBinnedBundle AcquireNewBundle(const FSmallPoolInfo& info) {
            Assert_NoAssume(HighestBundleUnused < info.NumBundlesPerChunk());

            const u32 firstIndex = HighestBundleUnused * info.MaxBlocksPerBundle;
            const u32 numBlocks = (Min(firstIndex + info.MaxBlocksPerBundle, info.NumBlocksPerChunk()) - firstIndex);

            HighestBundleUnused++;
            NumBlocksTaken += checked_cast<u16>(numBlocks);

            // initialize the new bundle:
            // - avoid initializing all bundles at the same time
            // - instead load balance by tracking count of already initialized bundles
            // - could also initialize the blocks inside a bundle lazily, but the rest of the code should then be aware of the packing
            // - current solution seems to be a good trade-off between performance and code complexity
            FBinnedBundle bundle;
            forrange(b, firstIndex, firstIndex + numBlocks)
                bundle.PushHead(reinterpret_cast<FBinnedBundleNode*>(BundleData() + b * info.BlockSize));

            return bundle;
        }

        NODISCARD FORCE_INLINE bool AllocateBundle_ReturnIfExhausted(const FSmallPoolInfo& info, FBinnedBundle* pBundle) {
            Assert(pBundle);

            if (Likely(FullBundles)) { // look first for a full bundle
                pBundle->Head = FullBundles;
                pBundle->Count = info.MaxBlocksPerBundle;
                FullBundles = FullBundles->NextBundle;
                NumBlocksTaken += info.MaxBlocksPerBundle;
            }
            else if (HighestBundleUnused < info.NumBundlesPerChunk()) { // initialize a new bundle IFP
                *pBundle = AcquireNewBundle(info);
            }
            else { // eventually give up the partial bundle when nothing else available
                *pBundle = PartialBundle;
                NumBlocksTaken += checked_cast<u16>(PartialBundle.Count);
                PartialBundle.Reset();
            }

            Assert(pBundle->Head); // guaranteed by callee
            Assert_NoAssume(NumBlocksTaken <= info.NumBlocksPerChunk());
            return (NumBlocksTaken == info.NumBlocksPerChunk());
        }

        NODISCARD FORCE_INLINE bool ReleaseNode_ReturnWasExhausted(const FSmallPoolInfo& info, FBinnedBundleNode* node) {
            Assert(node);
            Assert(PartialBundle.Count < info.MaxBlocksPerBundle);
            Assert(NumBlocksTaken);

            const bool wasExhausted = (NumBlocksTaken == info.NumBlocksPerChunk());

            NumBlocksTaken--;
            PartialBundle.PushHead(node);

            if (PartialBundle.Count == info.MaxBlocksPerBundle) {
                PartialBundle.Head->NextBundle = FullBundles;
                FullBundles = PartialBundle.Head;
                PartialBundle.Reset();
            }

            return wasExhausted;
        }

    };
    STATIC_ASSERT(sizeof(FPoolChunk) == CACHELINE_SIZE);

    struct CACHELINE_ALIGNED FSmallPoolInfo {

        struct FDeferredPageDecommit {
            const FSmallPoolInfo& SmallPool;
            FBinnedBundleNode* PagesToRelease{ nullptr };

            explicit FDeferredPageDecommit(const FSmallPoolInfo& smallPool) NOEXCEPT
                : SmallPool(smallPool)
            {}

            ~FDeferredPageDecommit() {
                while (PagesToRelease != nullptr) {
                    FBinnedBundleNode* nextPage = PagesToRelease->NextNode;
                    SmallPool.DecommitChunk(PagesToRelease);
                    PagesToRelease = nextPage;
                }
            }

            void Push_AssumeLocked(FPoolChunk* chunk) NOEXCEPT {
                reinterpret_cast<FBinnedBundleNode*>(chunk)->NextNode = PagesToRelease;
                PagesToRelease = reinterpret_cast<FBinnedBundleNode*>(chunk);
            }
        };

        FCriticalSection Mutex;

        byte* VirtualMemory;

        u32 MaxChunksEverAllocated;
        u16 BlockSize;
        u8 NumPagesPerChunk;
        u8 MaxBlocksPerBundle;

        TBitTree<u64> CommittedChunks;
        TBitTree<u64> ExhaustedChunks;

        NODISCARD size_t ChunkSize() const {
            return NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE;
        }

#if USE_PPE_MEMORYDOMAINS
        FMemoryTracking& TrackingData() const {
#if PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
            const u32 pool = FMallocBinned2::SmallPoolIndex(BlockSize);
            Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
            return GBinnedTrackingDataPerPool[pool].Data;
#else
            return MEMORYDOMAIN_TRACKING_DATA(SmallTables);
#endif
        }
#endif

        NODISCARD FORCE_INLINE u32 NumReservedChunks() const {
            Assert_NoAssume(CommittedChunks.Capacity() == ExhaustedChunks.Capacity());
            return CommittedChunks.Capacity();
        }
        NODISCARD FORCE_INLINE u32 NumBlocksPerChunk() const {
            const u32 chunkSize = NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE;
            const u32 availSize = chunkSize - PPE_MALLOCBINNED2_CHUNK_OVERHEADSIZE;

            Assert_NoAssume((availSize % BlockSize) <= PPE_MALLOCBINNED2_CHUNK_OVERHEADSIZE);
            return (availSize / BlockSize);
        }
        NODISCARD FORCE_INLINE u32 NumBundlesPerChunk() const {
            return (NumBlocksPerChunk() + MaxBlocksPerBundle - 1) / MaxBlocksPerBundle;
        }

        NODISCARD FORCE_INLINE FPoolChunk* ChunkPtr(u32 chunk) const {
            Assert_NoAssume(chunk < NumReservedChunks());
            Assert_NoAssume(chunk < MaxChunksEverAllocated);
            Assert_NoAssume(CommittedChunks.IsAllocated(chunk));

            return reinterpret_cast<FPoolChunk*>(VirtualMemory + chunk * NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE);
        }

        NODISCARD FORCE_INLINE u32 ChunkIndexFromPtr(const void* ptr) const {
            const u32 chunkSize = NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE;
            const u32 chunkIndex = checked_cast<u32>((static_cast<const byte*>(ptr) - VirtualMemory) / chunkSize);

            Assert_NoAssume(chunkIndex < NumReservedChunks());
            Assert_NoAssume(chunkIndex < MaxChunksEverAllocated);
            return chunkIndex;
        }

        NODISCARD FORCE_INLINE FPoolChunk* FrontChunk() const {
            const u32 chunk = ExhaustedChunks.NextAllocateBit();
            if (Likely(UMax != chunk))
                return ChunkPtr(chunk);

            return nullptr;
        }

        NODISCARD FPoolChunk* AllocateChunk() {
            const u32 chunk = CommittedChunks.NextAllocateBit();
            if (Unlikely(UMax == chunk))
                OnOutOfMemory(BlockSize + 1);

            CommittedChunks.AllocateBit(chunk);
            ExhaustedChunks.Deallocate(chunk);
            MaxChunksEverAllocated = Max(MaxChunksEverAllocated, chunk + 1);

            const u32 chunkSize = NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE;

            FPoolChunk* const pChunk = ChunkPtr(chunk);
            FVirtualMemory::PageCommit(pChunk, chunkSize
#if USE_PPE_MEMORYDOMAINS
                , this->TrackingData()
#endif
                );

            pChunk->InitializeBundles();
            return pChunk;
        }

        NODISCARD FORCE_INLINE FBinnedBundle AllocateBundle() {
            // check for an already committed chunk
            FPoolChunk* pChunk = FrontChunk();
            if (not pChunk) // need to commit a new chunk
                pChunk = AllocateChunk();

            FBinnedBundle bundle;
            if (pChunk->AllocateBundle_ReturnIfExhausted(*this, &bundle))
                ExhaustedChunks.AllocateBit( ChunkIndexFromPtr(pChunk) );

            Assert(bundle.Head);
            Assert(bundle.Count);
            return bundle;
        }

        FORCE_INLINE void ReleaseBundle(FDeferredPageDecommit& releaseChunksOutsideLock, FBinnedBundleNode* bundle) {
            Assert(bundle);

            while (bundle) {
                FBinnedBundleNode* const nextNode = bundle->NextNode;

                const u32 chunk = ChunkIndexFromPtr(bundle);
                FPoolChunk* const pChunk = ChunkPtr(chunk);

                if (pChunk->ReleaseNode_ReturnWasExhausted(*this, bundle))
                    ExhaustedChunks.Deallocate(chunk);

                if (pChunk->NumBlocksTaken == 0) // release empty chunks
                    ReleaseChunk(releaseChunksOutsideLock, chunk);

                bundle = nextNode;
            }
        }

        void ReleaseChunk(FDeferredPageDecommit& releaseChunksOutsideLock, u32 chunk) {
            Assert(CommittedChunks.IsAllocated(chunk));
            Assert_NoAssume(not ExhaustedChunks.IsAllocated(chunk));

            FPoolChunk* const pChunk = ChunkPtr(chunk);
            Assert_NoAssume(pChunk->NumBlocksTaken == 0);

            CommittedChunks.Deallocate(chunk);
            ExhaustedChunks.AllocateBit(chunk);

            releaseChunksOutsideLock.Push_AssumeLocked(pChunk);
        }

        void DecommitChunk(void* pChunk) const {
            FVirtualMemory::PageDecommit(pChunk,
                NumPagesPerChunk * PPE_MALLOCBINNED2_OS_PAGESIZE
                ARGS_IF_MEMORYDOMAINS(this->TrackingData()));
        }

    };

    FAllocatorBlock VM;
    FAllocatorBlock Meta;

    FSmallPoolInfo SmallPools[PPE_MALLOCBINNED2_SMALLPOOL_COUNT];

#if USE_PPE_ASSERT
    bool _available{false};
#endif

    // don't inline ctor/dtor to help codegen size in allocation hotpath
    NO_INLINE FBinnedSmallTable() NOEXCEPT;
    NO_INLINE ~FBinnedSmallTable() NOEXCEPT;

    FORCE_INLINE FBinnedBundle AllocateBundle(u32 pool) {
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);

        FSmallPoolInfo& smallPool = SmallPools[pool];

        const FCriticalScope scopeLock( &smallPool.Mutex );

        return smallPool.AllocateBundle();
    }

    void ReleaseBundle(u32 pool, FBinnedBundleNode* bundle) {
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);

        FSmallPoolInfo& smallPool = SmallPools[pool];

        FSmallPoolInfo::FDeferredPageDecommit releaseChunksOutsideLock(smallPool);

        const FCriticalScope scopeLock( &smallPool.Mutex );

        return smallPool.ReleaseBundle(releaseChunksOutsideLock, bundle);
    }

    void ReleasePendingBundles() {
        FBinnedGlobalRecycler::FBundleGarbage gc;

        forrange(pool, 0, PPE_MALLOCBINNED2_SMALLPOOL_COUNT) {
            const u32 numBundles = GBinnedGlobalRecycler.PopBundles(pool, &gc);

            FSmallPoolInfo& smallPool = SmallPools[pool];
            FSmallPoolInfo::FDeferredPageDecommit releaseChunksOutsideLock(smallPool);

            const FCriticalScope scopeLock( &smallPool.Mutex );
            forrange(b, 0, numBundles)
                smallPool.ReleaseBundle(releaseChunksOutsideLock, gc.FreeBundles[b]);
        }
    }

    FORCE_INLINE static FBinnedSmallTable& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBinnedSmallTable>, GInstance, 5000);
        Assert_Lightweight(GInstance.Get()->_available);
        return GInstance;
    }

};
//------------------------------------------------------------------------------
FBinnedSmallTable::FBinnedSmallTable() NOEXCEPT {
    STATIC_ASSERT(sizeof(FSmallPoolInfo) == 2*CACHELINE_SIZE);

#if PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
    ONLY_IF_MEMORYDOMAINS(BinnedTrackingDataStart_());
#endif

    const u32 osPageSize = checked_cast<u32>(PPE_MALLOCBINNED2_OS_PAGESIZE);
    const u32 osGranularity = checked_cast<u32>(PPE_MALLOCBINNED2_OS_GRANULARITY);
    const u32 maxChunkSize = checked_cast<u32>(PPE_MALLOCBINNED2_CHUNK_MAXSIZE);
    const u32 chunkOverheadSize = checked_cast<u32>(PPE_MALLOCBINNED2_CHUNK_OVERHEADSIZE);
    const u32 minBlocksPerChunk = checked_cast<u32>(PPE_MALLOCBINNED2_CHUNK_MINBLOCKS);
    const u32 maxBlocksPerChunk = checked_cast<u32>(PPE_MALLOCBINNED2_CHUNK_MAXBLOCKS);

    for (u32 i = 0; i < PPE_MALLOCBINNED2_SMALLPOOL_COUNT; ++i) {
        FSmallPoolInfo& pool = SmallPools[i];
        pool.BlockSize = checked_cast<u16>(FMallocBinned2::SmallPoolIndexToBlockSize(i));
        pool.MaxChunksEverAllocated = 0;
        pool.MaxBlocksPerBundle = checked_cast<u8>(Min( // bundle size doesn't depend on chunk size
            static_cast<size_t>(PPE_MALLOCBINNED2_BUNDLE_MAX_COUNT),
            static_cast<size_t>((PPE_MALLOCBINNED2_BUNDLE_MAX_SIZE + pool.BlockSize - 1u) / pool.BlockSize) ));
        Assert_NoAssume(pool.MaxBlocksPerBundle > 0);

        u32 bestWastedSize = UMax, bestPagesPerChunk = 0;
        for (u32 p = 1; p < UINT8_MAX && p * osPageSize <= maxChunkSize; ++p) {
            const u32 chunkSize = (osPageSize * p);
            const u32 blocksPerChunk = (chunkSize - chunkOverheadSize) / pool.BlockSize;
            const u32 wastedSize = (chunkSize - chunkOverheadSize) - blocksPerChunk * pool.BlockSize;

            if (bestPagesPerChunk && blocksPerChunk > maxBlocksPerChunk)
                break;
            if (blocksPerChunk < minBlocksPerChunk)
                continue;
            if (bestWastedSize >= wastedSize) {
                bestWastedSize = wastedSize;
                bestPagesPerChunk = p;
            }
        }
        AssertRelease(bestPagesPerChunk);

        pool.NumPagesPerChunk = checked_cast<u8>(bestPagesPerChunk);
        pool.CommittedChunks.SetupMemoryRequirements( checked_cast<u32>(PPE_MALLOCBINNED2_SMALLPOOL_RESERVE / (osPageSize * pool.NumPagesPerChunk)));
        pool.ExhaustedChunks.SetupMemoryRequirements(checked_cast<u32>(PPE_MALLOCBINNED2_SMALLPOOL_RESERVE / (osPageSize * pool.NumPagesPerChunk)));

        Meta.SizeInBytes += ROUND_TO_NEXT_CACHELINE(pool.CommittedChunks.AllocationSize());
        Meta.SizeInBytes += ROUND_TO_NEXT_CACHELINE(pool.ExhaustedChunks.AllocationSize());

        VM.SizeInBytes += PPE_MALLOCBINNED2_SMALLPOOL_RESERVE;
    }

    Meta.SizeInBytes = Meta::RoundToNextPow2(Meta.SizeInBytes, osGranularity);
    Meta.Data = FVirtualMemory::InternalAlloc(Meta.SizeInBytes TRACKINGDATA_PRM_IFP(SmallPoolInfo));
    AssertRelease(Meta.Data);

    VM.Data = FVirtualMemory::PageReserve(PPE_MALLOCBINNED2_SMALLPOOL_RESERVE, VM.SizeInBytes);
    AssertRelease(VM.Data);

    byte* pVM = static_cast<byte*>(VM.Data);
    byte* pMetadata = static_cast<byte*>(Meta.Data);
    for (u32 i = 0; i < PPE_MALLOCBINNED2_SMALLPOOL_COUNT; ++i) {
        FSmallPoolInfo& pool = SmallPools[i];

        pool.VirtualMemory = pVM;
        pVM += PPE_MALLOCBINNED2_SMALLPOOL_RESERVE;

        pool.CommittedChunks.Initialize(reinterpret_cast<u64*>(pMetadata), false);
        pMetadata += ROUND_TO_NEXT_CACHELINE(pool.CommittedChunks.AllocationSize());

        pool.ExhaustedChunks.Initialize(reinterpret_cast<u64*>(pMetadata), true );
        pMetadata += ROUND_TO_NEXT_CACHELINE(pool.ExhaustedChunks.AllocationSize());

        Assert_NoAssume(pool.CommittedChunks.Empty_ForAssert());
        Assert_NoAssume(pool.ExhaustedChunks.Full());
    }
    Assert_NoAssume(pMetadata <= static_cast<byte*>(Meta.Data) + Meta.SizeInBytes);

    FBinnedSmallTable::GSmallPoolVirtualMemory = static_cast<byte*>(VM.Data);

    ONLY_IF_ASSERT(_available = true);
}
//------------------------------------------------------------------------------
FBinnedSmallTable::~FBinnedSmallTable() NOEXCEPT {
#if USE_PPE_ASSERT
    Assert_NoAssume(_available);
    _available = false;
#endif

    ReleasePendingBundles();

#if USE_PPE_ASSERT
    for (const FSmallPoolInfo& pool : SmallPools) {
        const FCriticalScope scopeLock( &pool.Mutex );
        Assert_Lightweight(pool.CommittedChunks.Empty_ForAssert());
        Assert_Lightweight(pool.ExhaustedChunks.Full());
    }
#endif

    FVirtualMemory::PageRelease(VM.Data, VM.SizeInBytes);
    FVirtualMemory::InternalFree(Meta.Data, Meta.SizeInBytes TRACKINGDATA_PRM_IFP(SmallPoolInfo));

    ONLY_IF_ASSERT(FBinnedSmallTable::GSmallPoolVirtualMemory = nullptr);

#if PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
    ONLY_IF_MEMORYDOMAINS(BinnedTrackingDataShutdown_());
#endif
}
//------------------------------------------------------------------------------
// FBinnedPerThreadFreeBlockList
//------------------------------------------------------------------------------
struct FBinnedPerThreadFreeBlockList : Meta::FNonCopyableNorMovable, Meta::FThreadResource {

    static FORCE_INLINE FBinnedPerThreadFreeBlockList& Get() {
        ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FBinnedPerThreadFreeBlockList, GInstanceTLS);
        Assert_NoAssume(GInstanceTLS.AliveForDebug);
        return GInstanceTLS;
    }

    FBinnedFreeBlockList FreeLists[PPE_MALLOCBINNED2_SMALLPOOL_COUNT];

#if USE_PPE_ASSERT
    bool AliveForDebug{ true };
#endif

    FBinnedPerThreadFreeBlockList() = default;
    NO_INLINE ~FBinnedPerThreadFreeBlockList() NOEXCEPT {
        ReleaseCacheMemory();
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(FreeLists, sizeof(FreeLists))); // poison to crash if used after destructor
        ONLY_IF_ASSERT(AliveForDebug = false);
    }

    void RecycleFullBundles() NOEXCEPT;
    void ReleaseCacheMemory() NOEXCEPT;

    NODISCARD FORCE_INLINE void* Malloc(u32 pool) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        void* const result = FreeLists[pool].PopFromFront();
#if PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
        ONLY_IF_MEMORYDOMAINS(if (result) GBinnedTrackingDataPerPool[pool].Data.AllocateUser(FMallocBinned2::SmallPoolIndexToBlockSize(pool)));
#else
        ONLY_IF_MEMORYDOMAINS(if (result) MEMORYDOMAIN_TRACKING_DATA(SmallTables).AllocateUser(FMallocBinned2::SmallPoolIndexToBlockSize(pool)));
#endif
        return result;
    }
    // return true if the pointer was pushed
    NODISCARD FORCE_INLINE bool Free(u32 pool, void* ptr, u32 blockSize) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(ptr);
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        Assert_NoAssume(FMallocBinned2::SmallPoolIndexToBlockSize(pool) == blockSize);
        const bool couldFree = FreeLists[pool].PushToFront(ptr, blockSize);
#if PPE_MALLOCBINNED2_USE_DETAILED_TRACKING
        ONLY_IF_MEMORYDOMAINS(if (couldFree) GBinnedTrackingDataPerPool[pool].Data.DeallocateUser(blockSize));
#else
        ONLY_IF_MEMORYDOMAINS(if (couldFree) MEMORYDOMAIN_TRACKING_DATA(SmallTables).DeallocateUser(blockSize));
#endif
        return couldFree;
    }
    // return null if the block could not be reallocated
    NODISCARD FORCE_INLINE Meta::TOptional<FAllocatorBlock> Realloc(void* ptr, u32 size) {
        u32 oldPool = UMax, oldBlockSize = 0;
        const u32 newPool = FMallocBinned2::SmallPoolIndex(checked_cast<u16>(size));

        bool canFree = true;
        if (!!ptr) {
            oldPool = FBinnedSmallTable::SmallPoolIndexFromPtr(ptr);
            oldBlockSize = FMallocBinned2::SmallPoolIndexToBlockSize(oldPool);

            if (!!size && oldPool == newPool)// no need to resize the block?
                return FAllocatorBlock{ ptr, oldBlockSize };

            canFree = CanFree(oldPool, oldBlockSize);
        }

        if (Likely(canFree)) {
            void* newp = nullptr;
            if (size)
                newp = Malloc(newPool);

            if (!!newp || !size) {
                if (!!newp && !!ptr) {
                    Assert(oldPool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
                    // copy previous data to new block without polluting caches
                    FPlatformMemory::Memstream(newp, ptr,
                        ROUND_TO_NEXT_16(Min(checked_cast<u32>(size), oldBlockSize)));
                }

                if (!!ptr)
                    VerifyRelease(Free(oldPool, ptr, oldBlockSize));

                Assert_NoAssume(!newp || FBinnedSmallTable::IsSmallBlock(newp));
                Assert_NoAssume(!newp || FBinnedSmallTable::SmallPoolIndexFromPtr(newp) == newPool);

                if (!!newp) {
                    const size_t newBlockSize = FMallocBinned2::SmallPoolIndexToBlockSize(newPool);
                    return FAllocatorBlock{ newp, newBlockSize };
                }
            }
        }

        return Meta::TOptional<FAllocatorBlock>{};
    }
    // return true if a pointer can be pushed
    NODISCARD FORCE_INLINE bool CanFree(u32 pool, u32 blockSize) const {
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        Assert_NoAssume(FMallocBinned2::SmallPoolIndexToBlockSize(pool) == blockSize);
        return FreeLists[pool].CanPushToFront(blockSize);
    }
    // register a new bundle allocated from the global pool table
    void PushNewBundle(u32 pool, FBinnedBundle&& rbundle) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        return FreeLists[pool].PushBundle(std::move(rbundle));
    }
    // returns a bundle that needs to be freed if it can't be recycled
    NODISCARD FBinnedBundleNode* RecycleFullBundle(u32 pool) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        return FreeLists[pool].RecycleFull(pool);
    }
    // returns true if we have anything to pop
    NODISCARD bool ObtainRecycledPartial(u32 pool) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        return FreeLists[pool].ObtainPartial(pool);
    }
    NODISCARD FBinnedBundleNode* PopBundles(u32 pool) {
        THIS_THREADRESOURCE_CHECKACCESS();
        Assert(pool < PPE_MALLOCBINNED2_SMALLPOOL_COUNT);
        return FreeLists[pool].PopBundles();
    }

};
//------------------------------------------------------------------------------
void FBinnedPerThreadFreeBlockList::RecycleFullBundles() NOEXCEPT {
    forrange(pool, 0, PPE_MALLOCBINNED2_SMALLPOOL_COUNT) {
        for (FBinnedBundleNode* bundles = RecycleFullBundle(pool); bundles; ) {
            FBinnedBundleNode* const nextBundles = bundles->NextBundle;
            FBinnedSmallTable::Get().ReleaseBundle(pool, bundles);
            bundles = nextBundles;
        }
    }
}
//------------------------------------------------------------------------------
void FBinnedPerThreadFreeBlockList::ReleaseCacheMemory() NOEXCEPT {
    forrange(pool, 0, PPE_MALLOCBINNED2_SMALLPOOL_COUNT) {
        for (FBinnedBundleNode* bundles = PopBundles(pool); bundles; ) {
            FBinnedBundleNode* const nextBundles = bundles->NextBundle;
            FBinnedSmallTable::Get().ReleaseBundle(pool, bundles);
            bundles = nextBundles;
        }
    }
}
//------------------------------------------------------------------------------
// Allocation
//------------------------------------------------------------------------------
static FAllocatorBlock BinnedMallocFallback_(size_t size) {
    Assert(size);

    if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE)) {
        const u32 pool = FMallocBinned2::SmallPoolIndex(checked_cast<u16>(size));

        // try to recycle a bundle from the global recycler
        auto& freeBlocks = FBinnedPerThreadFreeBlockList::Get();
        if (freeBlocks.ObtainRecycledPartial(pool)) {
            Assert_NoAssume(freeBlocks.FreeLists[pool].PartialBundle.Head);
            if (void* const result = freeBlocks.Malloc(pool))
                return FAllocatorBlock(result, FMallocBinned2::SmallPoolIndexToBlockSize(pool));

            AssertNotReached(); // but we just recycled a bundle ?!
        }

        Assert_NoAssume(freeBlocks.FreeLists[pool].PartialBundle.Head == nullptr);
        Assert_NoAssume(freeBlocks.FreeLists[pool].FullBundle.Head == nullptr);

        // need to acquire a new bundle from the global pool table
        FBinnedBundle bundle = FBinnedSmallTable::Get().AllocateBundle(pool);

        Assert_NoAssume(freeBlocks.FreeLists[pool].PartialBundle.Head == nullptr);
        Assert_NoAssume(freeBlocks.FreeLists[pool].FullBundle.Head == nullptr);

        freeBlocks.PushNewBundle(pool, std::move(bundle));

        void* const result = freeBlocks.Malloc(pool);
        Assert(result);
        Assert_NoAssume(FBinnedSmallTable::IsSmallBlock(result));

        return FAllocatorBlock(result, FMallocBinned2::SmallPoolIndexToBlockSize(pool));

    }
    else {
        // external allocator for larger blocks
        FAllocatorBlock result;

#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
        if (Likely(size <= FMallocBitmap::MaxAllocSize))
            result = FMallocBitmap::HeapAlloc(size, PPE_MALLOCBINNED2_BOUNDARY);
#endif

        if (not result) {
            size = FVirtualMemory::SnapSize(size);
            result.Data = FVirtualMemory::Alloc(size ARGS_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(VeryLargeBlocks)));
            result.SizeInBytes = size;
        }

        Assert_NoAssume(not FBinnedSmallTable::IsSmallBlock(result.Data));
        return result;
    }
}
//------------------------------------------------------------------------------
FORCE_INLINE static void BinnedFreeSmallBlock_(void* ptr) {
    const u32 pool = FBinnedSmallTable::SmallPoolIndexFromPtr(ptr);
    const u32 blockSize = FMallocBinned2::SmallPoolIndexToBlockSize(pool);

    auto& freeBlocks = FBinnedPerThreadFreeBlockList::Get();
    FBinnedBundleNode* const recycling = freeBlocks.RecycleFullBundle(pool);

    // guaranteed to have free space thanks to RecycleFullBundle() ^^^
    Verify(freeBlocks.Free(pool, ptr, blockSize));

    // maybe the global recycler was full ?
    if (Unlikely(recycling))
        FBinnedSmallTable::Get().ReleaseBundle(pool, recycling);
}
//------------------------------------------------------------------------------
static void BinnedFreeVeryLargeBlock_(void* ptr, size_t size) {
    FVirtualMemory::Free(ptr, size
        ARGS_IF_MEMORYDOMAINS(MEMORYDOMAIN_TRACKING_DATA(VeryLargeBlocks)));
}
//------------------------------------------------------------------------------
static void BinnedFreeFallback_(void* ptr) {
    Assert(ptr);

    if (Likely(FBinnedSmallTable::IsSmallBlock(ptr))) {
        BinnedFreeSmallBlock_(ptr);
    }
    else {
        // large blocks are handled externally
#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
        if (Unlikely(not FMallocBitmap::HeapFree_ReturnIfAliases(ptr)))
#endif
            BinnedFreeVeryLargeBlock_(ptr,
#if USE_PPE_ASSERT
                FVirtualMemory::SizeInBytes(ptr)
#else
                0 // size is only used for assertion
#endif
            );
    }
}
//------------------------------------------------------------------------------
static void BinnedFreeForDeleteFallback_(void* ptr, size_t size) {
    Assert(ptr);

    if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE)) {
        Assert_NoAssume(FBinnedSmallTable::IsSmallBlock(ptr));

        BinnedFreeSmallBlock_(ptr);
    }
    // large blocks are handled externally
#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    else if (size <= FMallocBitmap::MediumMaxAllocSize)
        FMallocBitmap::MediumFree(ptr);
    else if (size <= FMallocBitmap::LargeMaxAllocSize)
        FMallocBitmap::LargeFree(ptr);
#endif
    else
        BinnedFreeVeryLargeBlock_(ptr, size);
}
//------------------------------------------------------------------------------
static void* BinnedReallocFallback_(void* ptr, size_t newSize) {
    Assert(!!ptr | !!newSize);

    // naive path: allocate a new block, copy data and release old block
    FAllocatorBlock nblk;
    if (newSize)
        nblk = FMallocBinned2::MallocForNew(newSize);
    Assert_NoAssume(!!nblk || !newSize);

    if (!!nblk && !!ptr) {
        Assert(nblk.Data != ptr);
        const size_t oldSize = FMallocBinned2::RegionSize(ptr);
        Assert(oldSize != newSize); // don't want to realloc into a block of same size a-priori
        FPlatformMemory::Memcpy(nblk.Data, ptr, Min(newSize, oldSize));
    }

    if (ptr)
        FMallocBinned2::Free(ptr);

    return nblk.Data;
}
//------------------------------------------------------------------------------
static FAllocatorBlock BinnedReallocForNewFallback_(FAllocatorBlock blk, size_t size) {
    Assert(!!blk.Data | !!size);

#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    // bitmap path: check if bitmap can reallocate
    if (size > PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE && blk.SizeInBytes > PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE) {
        if (size <= FMallocBitmap::MediumMaxAllocSize && blk.SizeInBytes <= FMallocBitmap::MediumMaxAllocSize) {
            const size_t alignedSize = FMallocBitmap::MediumSnapSize(size);
            if (void* const newp = FMallocBitmap::MediumReallocIFP(blk.Data, alignedSize); !!newp)
                return FAllocatorBlock{ newp, alignedSize };
        }
        else if (
            size > FMallocBitmap::MediumMaxAllocSize && blk.SizeInBytes > FMallocBitmap::MediumMaxAllocSize &&
            size <= FMallocBitmap::LargeMaxAllocSize && blk.SizeInBytes <= FMallocBitmap::LargeMaxAllocSize) {
            const size_t alignedSize = FMallocBitmap::LargeSnapSize(size);
            if (void* const newp = FMallocBitmap::LargeReallocIFP(blk.Data, alignedSize); !!newp)
                return FAllocatorBlock{ newp, alignedSize };
        }
    }
#endif

    // naive path: allocate a new block, copy data and release old block
    FAllocatorBlock nblk;
    if (size)
        nblk = FMallocBinned2::MallocForNew(size);
    Assert_NoAssume(!!nblk | !size);

    if (!!nblk && !!blk) {
        Assert(blk.Data != nblk.Data);
        Assert_NoAssume(FMallocBinned2::SnapSize(blk.SizeInBytes) != FMallocBinned2::SnapSize(size)); // should not realloc into a block of same size
        FPlatformMemory::Memstream(nblk.Data, blk.Data, Min(ROUND_TO_NEXT_16(size), blk.SizeInBytes));
    }

    if (blk)
        FMallocBinned2::FreeForDelete(blk);

    return nblk;
}
//------------------------------------------------------------------------------
static void CheckBlockInvariants_(const FAllocatorBlock& blk) {
#if USE_PPE_MEMORY_DEBUGGING
    if (blk.Data != nullptr) {
        AssertRelease_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, blk.Data));
        const size_t regionSize = FMallocBinned2::RegionSize(blk.Data);
        AssertRelease_NoAssume(blk.SizeInBytes == regionSize);
        AssertRelease_NoAssume(FMallocBinned2::SnapSize(blk.SizeInBytes) == regionSize);
        Unused(regionSize);
    }
    else {
        AssertRelease_NoAssume(blk.SizeInBytes == 0);
    }
#else
    Unused(blk);
#endif
}
//------------------------------------------------------------------------------
} //!namespace
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void* FMallocBinned2::Malloc(size_t size) {
    // old school C api
    return MallocForNew(size).Data;
}
//------------------------------------------------------------------------------
void FMallocBinned2::Free(void* const ptr) {
    AssertRelease(ptr);

    if (Likely(FBinnedSmallTable::IsSmallBlock(ptr))) {
        const u32 pool = FBinnedSmallTable::SmallPoolIndexFromPtr(ptr);
        const u32 blockSize = SmallPoolIndexToBlockSize(pool);

        auto& freeBlocks = FBinnedPerThreadFreeBlockList::Get();
        if (freeBlocks.Free(pool, ptr, blockSize))
            return;
    }

    Meta::unlikely(&BinnedFreeFallback_, ptr);
}
//------------------------------------------------------------------------------
void* FMallocBinned2::Realloc(void* const ptr, size_t size) {
    AssertRelease(ptr || size);

    if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE)) {
        if (!ptr || FBinnedSmallTable::IsSmallBlock(ptr)) {
            const Meta::TOptional<FAllocatorBlock> nblk = FBinnedPerThreadFreeBlockList::Get().Realloc(
                ptr, checked_cast<u32>(size));

            if (Likely(nblk.has_value())) {
                Assert_NoAssume(nblk->SizeInBytes >= size);
                CheckBlockInvariants_(*nblk);
                return nblk->Data;
            }
        }
    }

    return Meta::unlikely(&BinnedReallocFallback_, ptr, size);
}
//------------------------------------------------------------------------------
FAllocatorBlock FMallocBinned2::MallocForNew(size_t size) {
    AssertRelease(size);

    FAllocatorBlock result;

    if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE)) {
        const u32 pool = SmallPoolIndex(checked_cast<u16>(size));
        result.Data = FBinnedPerThreadFreeBlockList::Get().Malloc(pool);
        result.SizeInBytes = SmallPoolIndexToBlockSize(pool);
    }

    if (Unlikely(!result)) // fallback for large blocks or exhausted pools
        result = Meta::unlikely(&BinnedMallocFallback_, size);
    else {
        Assert_NoAssume(FBinnedSmallTable::IsSmallBlock(result.Data));
        Assert_NoAssume(FBinnedSmallTable::SmallPoolIndexFromPtr(result.Data) == SmallPoolIndex(checked_cast<u16>(size)));
    }

    CheckBlockInvariants_(result);
    Assert_NoAssume(result.SizeInBytes == SnapSize(size));
    return result;
}
//------------------------------------------------------------------------------
FAllocatorBlock FMallocBinned2::ReallocForNew(FAllocatorBlock blk, size_t size) {
    AssertRelease(blk.Data || size);
    //Assert_NoAssume(size != blk.SizeInBytes);
    CheckBlockInvariants_(blk);

    if (SnapSize(size) == blk.SizeInBytes)
        return blk;

    if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE && blk.SizeInBytes <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE)) {
        Assert_NoAssume(!blk.Data || FBinnedSmallTable::IsSmallBlock(blk.Data));
        const Meta::TOptional<FAllocatorBlock> nblk = FBinnedPerThreadFreeBlockList::Get().Realloc(blk.Data, checked_cast<u32>(size));

        if (Likely(nblk.has_value())) {
            CheckBlockInvariants_(*nblk);
            return *nblk;
        }
    }

    const FAllocatorBlock result = Meta::unlikely(&BinnedReallocForNewFallback_, blk, size);
    CheckBlockInvariants_(result);
    Assert_NoAssume(result.SizeInBytes == SnapSize(size));
    return result;
}
//------------------------------------------------------------------------------
void FMallocBinned2::FreeForDelete(FAllocatorBlock blk) {
    AssertRelease(blk);
    CheckBlockInvariants_(blk);

    if (Likely(blk.SizeInBytes <= MaxSmallBlockSize)) {
        Assert_NoAssume(FBinnedSmallTable::IsSmallBlock(blk.Data));

        const u32 pool = FBinnedSmallTable::SmallPoolIndexFromPtr(blk.Data);
        const u32 blockSize = SmallPoolIndexToBlockSize(pool);
        Assert_NoAssume(blockSize >= blk.SizeInBytes);

        auto& freeBlocks = FBinnedPerThreadFreeBlockList::Get();
        if (freeBlocks.Free(pool, blk.Data, blockSize))
            return;
    }

    Meta::unlikely(&BinnedFreeForDeleteFallback_, blk.Data, blk.SizeInBytes);
}
//------------------------------------------------------------------------------
size_t FMallocBinned2::SnapSize(size_t size) NOEXCEPT {
    if (Likely(size)) {
        if (Likely(size <= PPE_MALLOCBINNED2_SMALLPOOL_MAX_SIZE))
            return BoundSizeToSmallPool(checked_cast<u32>(size));

#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
        if (Likely(size <= FMallocBitmap::MaxAllocSize))
            return FMallocBitmap::SnapSize(size);
#endif

        return FVirtualMemory::SnapSize(size);
    }

    return 0;
}
//------------------------------------------------------------------------------
size_t FMallocBinned2::RegionSize(void* ptr) NOEXCEPT {
    Assert(ptr);

    if (Likely(FBinnedSmallTable::IsSmallBlock(ptr)))
        return SmallPoolIndexToBlockSize(FBinnedSmallTable::SmallPoolIndexFromPtr(ptr));

#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    size_t sizeInBytes;
    if (Likely(FMallocBitmap::RegionSize_ReturnIfAliases(&sizeInBytes, ptr)))
        return sizeInBytes;
#endif

    return FVirtualMemory::SizeInBytes(ptr);
}
//------------------------------------------------------------------------------
void FMallocBinned2::ReleaseCacheMemory() {
    PPE_LOG(MallocBinned2, Debug, "release cache memory in {0}", std::this_thread::get_id());

    FBinnedPerThreadFreeBlockList::Get().ReleaseCacheMemory();
    FBinnedSmallTable::Get().ReleasePendingBundles();

#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    FMallocBitmap::MemoryTrim();
#endif
}
//------------------------------------------------------------------------------
void FMallocBinned2::ReleasePendingBlocks() {
    //PPE_LOG(MallocBinned2, Debug, "release pending blocks in {0}", std::this_thread::get_id()); // too verbose
    FBinnedPerThreadFreeBlockList::Get().RecycleFullBundles();
}
//------------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocBinned2::DumpMemoryInfo(FTextWriter& oss) {
#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    FMallocBitmap::DumpHeapInfo(oss);
#else
    Unused(oss);
#endif
}
void FMallocBinned2::DumpMemoryInfo(FWTextWriter& oss) {
#if PPE_MALLOCBINNED2_USE_BITMAP_FOR_LARGE_BLOCKS
    FMallocBitmap::DumpHeapInfo(oss);
#else
    Unused(oss);
#endif
}
#endif
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_RESTORE_RUNTIMECHECKS // %_NOCOMMIT%

PRAGMA_MSVC_WARNING_POP()
