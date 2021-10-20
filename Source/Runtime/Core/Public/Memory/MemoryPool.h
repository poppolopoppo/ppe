#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/Array.h"
#include "Container/BitTree.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryView.h"
#include "Meta/Functor.h"
#include "Meta/TypeInfo.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
class CACHELINE_ALIGNED TMemoryPool : Meta::FNonCopyableNorMovable {
public:
    STATIC_ASSERT(Meta::IsPow2(_ChunkSize));
    STATIC_ASSERT(_BlockSize >= sizeof(intptr_t));
    STATIC_ASSERT(_MaxChunks > 0);

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using block_type = std::aligned_storage_t<_BlockSize, _Align>;
    using index_type =
        std::conditional_t<(_ChunkSize * _MaxChunks > UINT32_MAX), u64,
        std::conditional_t<(_ChunkSize * _MaxChunks > UINT16_MAX), u32, u16> >;
    STATIC_ASSERT(sizeof(block_type) >= sizeof(index_type));

    STATIC_CONST_INTEGRAL(index_type, MaxSize, static_cast<index_type>(_ChunkSize * _MaxChunks));
    STATIC_CONST_INTEGRAL(index_type, BlockSize, static_cast<index_type>(_BlockSize));
    STATIC_CONST_INTEGRAL(index_type, ChunkSize, static_cast<index_type>(_ChunkSize));
    STATIC_CONST_INTEGRAL(index_type, MaxChunks, static_cast<index_type>(_MaxChunks));
    STATIC_CONST_INTEGRAL(index_type, ThreadGranularity, 4/* good trade-off from empirical benchmarks */);

    TMemoryPool() {
        InitializeInternalPool_(*_pool.LockExclusive());
    }
    explicit TMemoryPool(_Allocator&& ralloc) : _pool(std::move(ralloc)) {
        InitializeInternalPool_(*_pool.LockExclusive());
    }
    explicit TMemoryPool(const _Allocator& alloc) : _pool(alloc) {
        InitializeInternalPool_(*_pool.LockExclusive());
    }

    ~TMemoryPool();

    _Allocator& Allocator() { return _pool.Value_NotThreadSafe(); }
    const _Allocator& Allocator() const { return _pool.Value_NotThreadSafe(); }

#if USE_PPE_DEBUG
    u32 NumLiveBlocks_ForDebug() const { return _pool.Value_NotThreadSafe().NumLiveBlocks.load(std::memory_order_relaxed); }
#endif

    static u32 BucketHint(hash_t seed) { return BucketIndexTLS_(seed); }

    NODISCARD index_type Allocate() { return Allocate(BucketIndexTLS_()); }
    NODISCARD index_type Allocate(u32 bucket);

    void Deallocate(index_type block) { Deallocate(block, BucketIndexTLS_()); }
    void Deallocate(index_type block, u32 bucket);

    void* At(index_type block) const NOEXCEPT { return PoolNode_(_pool.Value_NotThreadSafe().pChunks, block); }
    void* operator [](index_type block) const NOEXCEPT { return At(block); }

    void Clear_AssertCompletelyEmpty();
    void Clear_IgnoreLeaks();

    void ReleaseCacheMemory();

    bool CheckInvariants() const;

    static CONSTEXPR size_t AllocationSize() { return ChunkAllocationSize; }

private:
    using FBitTree_ = TBitTree<size_t, false>;

    struct FPoolNode_ {
        index_type NextNode;
        union {
            index_type NextBundle;
            index_type BundleCount;
        };
    };
    STATIC_ASSERT(sizeof(FPoolNode_) <= BlockSize);

    STATIC_CONST_INTEGRAL(index_type, BundleMaxGarbage, 8u);
    STATIC_CONST_INTEGRAL(index_type, BundleMaxSizeInBytes, 32_KiB);
    STATIC_CONST_INTEGRAL(index_type, BundleMaxCount, Min(ChunkSize, Min(ChunkSize / ThreadGranularity, BundleMaxSizeInBytes / BlockSize)));
    STATIC_CONST_INTEGRAL(index_type, BundleCountPerChunk, (ChunkSize + BundleMaxCount - 1u) / BundleMaxCount);

    struct FPoolChunk_;

    FORCE_INLINE static FPoolNode_* PoolNode_(FPoolChunk_** const pChunks, const index_type id) {
        Assert(pChunks);
        Assert(id < MaxSize);
        const index_type ch = (id / ChunkSize);
        Assert(ch < MaxChunks);
        Assert(pChunks[ch]);
        return reinterpret_cast<FPoolNode_*>(pChunks[ch]->BundleData() + (id - ch * ChunkSize));
    }

    struct FPoolBundle_ {
        index_type Head{ UMax };
        index_type Count{ 0 };

        FORCE_INLINE bool Empty() const {
            Assert(Count || UMax == Head);
            return (0 == Count);
        }
        FORCE_INLINE bool Full() const {
            Assert(Count <= BundleMaxCount);
            return (BundleMaxCount == Count);
        }

        FORCE_INLINE void Reset() {
            Head = UMax;
            Count = 0;
        }

        FORCE_INLINE void PushHead(index_type id, FPoolNode_* node) {
            Assert(node);
            node->NextNode = Head;
            node->NextBundle = UMax;
            Head = id;
            ++Count;
        }

        FORCE_INLINE index_type PopHead(FPoolChunk_** pChunks) {
            Assert(UMax != Head);
            Assert(Count > 0);

            const index_type id = Head;
            FPoolNode_* const node = PoolNode_(pChunks, id);
            Head = node->NextNode;
            --Count;

            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(node, BlockSize));
            return id;
        }

    };

    struct FBundleRecycler_ {
        std::atomic<index_type> FreeBundles[BundleMaxGarbage];
        STATIC_ASSERT(std::atomic<index_type>::is_always_lock_free);

        bool PushBundle(index_type bundle) {
            Assert(UMax != bundle);

            forrange(i, 0, BundleMaxGarbage) {
                index_type head = UMax;
                if (FreeBundles[i].compare_exchange_weak(head, bundle,
                        std::memory_order_release, std::memory_order_relaxed)) {
                    return true;
                }
            }

            return false;
        }

        index_type PopBundle() {
            forrange(i, 0, BundleMaxGarbage) {
                index_type head = FreeBundles[i].load(std::memory_order_relaxed);
                if (UMax != head && FreeBundles[i].compare_exchange_weak(head, UMax,
                        std::memory_order_release, std::memory_order_relaxed)) {
                    return head;
                }
            }

            return UMax;
        }

        index_type StealBundles(index_type (&freeBundles)[BundleMaxGarbage]) {
            index_type n = 0;

            forrange(i, 0, BundleMaxGarbage) {
                for(index_type head = FreeBundles[i].load(std::memory_order_relaxed); head != UMax; ) {
                    if (FreeBundles[i].compare_exchange_weak(head, UMax,
                            std::memory_order_release, std::memory_order_relaxed)) {
                        Assert(n < BundleMaxGarbage);
                        freeBundles[n++] = head;
                    }
                }
            }

            return n;
        }
    };

    struct FFreeBlockList_ {
        FPoolBundle_ PartialBundle;
        FPoolBundle_ FullBundle;

        NODISCARD FORCE_INLINE bool PushToFront(index_type id, void* ptr) {
            Assert(id < MaxSize);
            Assert(ptr);

            if (PartialBundle.Full()) {
                if (not FullBundle.Empty())
                    return false;

                FullBundle = PartialBundle;
                PartialBundle.Reset();
            }

            PartialBundle.PushHead(id, static_cast<FPoolNode_*>(ptr));
            return true;
        }
        FORCE_INLINE index_type PopFromFront(FPoolChunk_** pChunks) {
            if (Unlikely(!PartialBundle.Count & (!!FullBundle.Count))) {
                Assert_NoAssume(PartialBundle.Empty());
                Assert_NoAssume(not FullBundle.Empty());

                PartialBundle = FullBundle;
                FullBundle.Reset();
            }

            return (!!PartialBundle.Count
                ? PartialBundle.PopHead(pChunks)
                : UMax );
        }

        void PushBundle(FPoolBundle_&& rbundle);
        NODISCARD index_type PopBundles(FPoolChunk_** pChunks);

        NODISCARD bool ObtainPartial(FPoolChunk_** pChunks, FBundleRecycler_& recycler);
        NODISCARD index_type RecycleFull(FPoolChunk_** pChunks, FBundleRecycler_& recycler);
    };

    struct CACHELINE_ALIGNED FPoolChunk_ {
        FPoolBundle_ PartialBundle;
        index_type FullBundles;

        index_type ChunkIndex;
        index_type NumBlocksTaken;
        index_type HighestBundleUnused;

        FORCE_INLINE block_type* BundleData() const {
            return reinterpret_cast<block_type*>(const_cast<FPoolChunk_*>(this + 1));
        }
        FORCE_INLINE FPoolNode_* ChunkNode(index_type id) const {
            Assert_NoAssume(id - ChunkIndex * ChunkSize < ChunkSize);
            return reinterpret_cast<FPoolNode_*>(BundleData() + (id - ChunkIndex * ChunkSize));
        }

        FORCE_INLINE void InitializeBundles(index_type chunkIndex) {
            Assert_NoAssume(chunkIndex < ChunkSize);

            PartialBundle.Reset();
            FullBundles = UMax;
            ChunkIndex = chunkIndex;
            NumBlocksTaken = 0;
            HighestBundleUnused = 0;
        }

        NODISCARD FORCE_INLINE FPoolBundle_ AcquireBundle() {
            Assert_NoAssume(HighestBundleUnused < BundleCountPerChunk);

            const index_type first = (HighestBundleUnused * BundleMaxCount);
            const index_type numBlocks = Min(checked_cast<index_type>(ChunkSize - first), BundleMaxCount);

            ++HighestBundleUnused;
            NumBlocksTaken += numBlocks;
            Assert_NoAssume(NumBlocksTaken <= ChunkSize);

            FPoolBundle_ bundle;
            forrange(b, first, checked_cast<index_type>(first + numBlocks)) {
                bundle.PushHead(
                    ChunkIndex * ChunkSize + b,
                    reinterpret_cast<FPoolNode_*>(BundleData() + b) );
            }

            return bundle;
        }

        NODISCARD FORCE_INLINE bool AllocateBundle_ReturnIfExhausted(FPoolBundle_* pBundle) {
            Assert(pBundle);

            if (Likely(UMax != FullBundles)) {
                // look first for a full bundle
                pBundle->Head = FullBundles;
                pBundle->Count = BundleMaxCount;

                FullBundles = ChunkNode(FullBundles)->NextBundle;
                Assert_NoAssume(FullBundles == UMax || FullBundles - ChunkIndex * ChunkSize < ChunkSize);
                NumBlocksTaken += BundleMaxCount;
            }
            else if (HighestBundleUnused < BundleCountPerChunk) {
                // initialize a new bundle IFP
                *pBundle = AcquireBundle();
            }
            else {
                // eventually give up partial bundle
                *pBundle = PartialBundle;
                NumBlocksTaken += PartialBundle.Count;
                PartialBundle.Reset();
            }

            Assert(not pBundle->Empty()); // guaranteed by callee
            Assert_NoAssume(NumBlocksTaken <= ChunkSize);
            return (NumBlocksTaken == ChunkSize);
        }

        NODISCARD FORCE_INLINE bool ReleaseNode_ReturnWasExhausted(index_type id, FPoolNode_* node) {
            Assert(id - ChunkIndex * ChunkSize < ChunkSize);
            Assert(node);
            Assert(PartialBundle.Count < BundleMaxCount);
            Assert(0 != NumBlocksTaken);

            const bool wasExhausted = (NumBlocksTaken == ChunkSize);

            --NumBlocksTaken;
            PartialBundle.PushHead(id, node);

            if (PartialBundle.Full()) {
                ChunkNode(PartialBundle.Head)->NextBundle = FullBundles;
                FullBundles = PartialBundle.Head;
                PartialBundle.Reset();
            }

            return wasExhausted;
        }
    };
    STATIC_ASSERT(sizeof(FPoolChunk_) == CACHELINE_SIZE);

    STATIC_CONST_INTEGRAL(size_t, ChunkAllocationSize, sizeof(FPoolChunk_) + ChunkSize * BlockSize);

    struct FInternalPool_ : _Allocator {
        FBitTree_ CommittedChunks;
        FBitTree_ ExhaustedChunks;

        FPoolChunk_** pChunks{ nullptr };

#if USE_PPE_DEBUG
        mutable std::atomic<u32> NumLiveBlocks{ 0 };
#endif

#if USE_PPE_MEMORYDOMAINS
        mutable FMemoryTracking TrackingData{
            Meta::type_info<TMemoryPool>.name,
            std::addressof(MEMORYDOMAIN_TRACKING_DATA(MemoryPool))
        };
        FInternalPool_() {
            RegisterTrackingData(&TrackingData);
        }
        ~FInternalPool_() {
            Assert_NoAssume(NumLiveBlocks == 0);
            UnregisterTrackingData(&TrackingData);
        }
#else
        FInternalPool_() = default;
#endif
        explicit FInternalPool_(_Allocator&& ralloc) : _Allocator(std::move(ralloc)) {
            ONLY_IF_MEMORYDOMAINS(RegisterTrackingData(&TrackingData));
        }
        explicit FInternalPool_(const _Allocator& alloc) : _Allocator(alloc) {
            ONLY_IF_MEMORYDOMAINS(RegisterTrackingData(&TrackingData));
        }

        FPoolChunk_* FrontChunk() const {
            Assert(pChunks);
            const auto ch = ExhaustedChunks.NextAllocateBit();
            if (UMax != ch) {
                Assert(ch < MaxChunks);
                Assert_NoAssume(pChunks[ch]->ChunkIndex == ch);
                return pChunks[ch];
            }

            return nullptr;
        }

        FPoolChunk_* AllocateChunk() {
            Assert(pChunks);
            const u32 ch = CommittedChunks.NextAllocateBit();
            if (Likely(UMax != ch)) {
                CommittedChunks.AllocateBit(ch);
                ExhaustedChunks.Deallocate(ch);

                const FAllocatorBlock blk = allocator_traits::Allocate(*this, ChunkAllocationSize);
                ONLY_IF_MEMORYDOMAINS( TrackingData.AllocateSystem(blk.SizeInBytes) );

                auto* const pChunk = pChunks[ch] = static_cast<FPoolChunk_*>(blk.Data);
                pChunk->InitializeBundles(checked_cast<index_type>(ch));
                return pChunk;
            }

            return nullptr;
        }

        FORCE_INLINE FPoolBundle_ AllocateBundle() {
            Assert(pChunks);
            FPoolChunk_* pChunk = FrontChunk();
            if (nullptr == pChunk) {
                pChunk = AllocateChunk();
                if (Unlikely(nullptr == pChunk))
                    return FPoolBundle_{};
            }
            Assert(pChunk);

            FPoolBundle_ bundle;
            if (pChunk->AllocateBundle_ReturnIfExhausted(&bundle))
                ExhaustedChunks.AllocateBit(pChunk->ChunkIndex);

            Assert(not bundle.Empty());
            return bundle;
        }

        FORCE_INLINE void ReleaseBundle(index_type bundle) {
            Assert(pChunks);
            Assert(UMax != bundle);

            do {
                FPoolNode_* const node = PoolNode_(pChunks, bundle);

                const index_type nextNode = node->NextNode;
                const index_type ch = (bundle / ChunkSize);
                Assert(ch < MaxChunks);

                if (pChunks[ch]->ReleaseNode_ReturnWasExhausted(bundle, node))
                    ExhaustedChunks.Deallocate(ch);

                if (0 == pChunks[ch]->NumBlocksTaken)
                    ReleaseChunk(ch);

                bundle = nextNode;

            } while (UMax != bundle);
        }

        void ReleaseChunk(index_type ch) {
            Assert(ch < MaxChunks);
            Assert(pChunks);
            Assert(CommittedChunks.IsAllocated(ch));
            Assert_NoAssume(not ExhaustedChunks.IsAllocated(ch));

            FPoolChunk_* const pChunk = pChunks[ch];
            Assert(pChunk);
            Assert_NoAssume(0 == pChunk->NumBlocksTaken);

            CommittedChunks.Deallocate(ch);
            ExhaustedChunks.AllocateBit(ch);

            const FAllocatorBlock blk{ pChunk, ChunkAllocationSize };
            ONLY_IF_MEMORYDOMAINS( TrackingData.DeallocateSystem(blk.SizeInBytes) );
            allocator_traits::Deallocate(*this, blk);

            pChunks[ch] = nullptr;
        }

        void ReleaseAllChunks_IgnoreLeaks() {
            Assert(pChunks);
            ONLY_IF_MEMORYDOMAINS( TrackingData.ReleaseAllUser() ); // ignore leaks here (checked before)

            CommittedChunks.Initialize(CommittedChunks.Bits, false);
            ExhaustedChunks.Initialize(ExhaustedChunks.Bits, true);

            forrange(ch, 0, MaxChunks) {
                if (FPoolChunk_* const pChunk = pChunks[ch]) {
                    const FAllocatorBlock blk{ pChunk, ChunkAllocationSize };

                    ONLY_IF_MEMORYDOMAINS( TrackingData.DeallocateSystem(blk.SizeInBytes) );

                    allocator_traits::Deallocate(*this, blk);

                    pChunks[ch] = nullptr;
                }
            }
        }
    };

    static u32 BucketIndexTLS_(hash_t seed = PPE_HASH_VALUE_SEED) {
        hash_combine(seed, std::hash<std::thread::id>{}( std::this_thread::get_id() ));
        return static_cast<u32>(seed % ThreadGranularity);
    }

    void InitializeInternalPool_(FInternalPool_& pool);
    NODISCARD NO_INLINE index_type AllocateFromNewBundle_AssumeUnlocked_(u32 firstBucket);
    NO_INLINE void RecyclePartialBundle_AssumeUnlocked_(u32 bucket, index_type block, FPoolNode_* node);
    FORCE_INLINE void Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool);

    class CACHELINE_ALIGNED FPoolBucket_ : public TThreadSafe<FFreeBlockList_, EThreadBarrier::CriticalSection>
    {};

    CACHELINE_ALIGNED TThreadSafe<FInternalPool_, EThreadBarrier::CriticalSection> _pool;
    TStaticArray<FPoolBucket_, ThreadGranularity> _buckets;
    CACHELINE_ALIGNED FBundleRecycler_ _recycler;
};
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::~TMemoryPool() {
    const auto exclusivePool = _pool.LockExclusive();

    Assert_NoAssume(0 == exclusivePool->NumLiveBlocks); // some blocks are still alive!

    exclusivePool->ReleaseAllChunks_IgnoreLeaks();

    size_t totalSizeInBytes = 0;
    totalSizeInBytes += exclusivePool->CommittedChunks.AllocationSize();
    totalSizeInBytes += exclusivePool->ExhaustedChunks.AllocationSize();
    totalSizeInBytes = ROUND_TO_NEXT_16(totalSizeInBytes);
    totalSizeInBytes += sizeof(FPoolChunk_*) * MaxChunks;

    FAllocatorBlock blk{ exclusivePool->CommittedChunks.Bits, totalSizeInBytes };
    blk.SizeInBytes = allocator_traits::SnapSize(*exclusivePool, totalSizeInBytes);
    allocator_traits::Deallocate(*exclusivePool, blk);

#if USE_PPE_ASSERT
    exclusivePool->CommittedChunks = Default;
    exclusivePool->ExhaustedChunks = Default;
    exclusivePool->pChunks = nullptr;

    forrange(i, 0, BundleMaxGarbage)
        _recycler.FreeBundles[i].store(MaxChunks, std::memory_order_relaxed);
#endif
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::InitializeInternalPool_(FInternalPool_& pool) {
    STATIC_ASSERT(std::atomic<index_type>::is_always_lock_free);

    Assert_NoAssume(nullptr == pool.CommittedChunks.Bits);
    Assert_NoAssume(nullptr == pool.CommittedChunks.Bits);
    Assert_NoAssume(nullptr == pool.pChunks);
    Assert_NoAssume(0 == pool.NumLiveBlocks);

    pool.CommittedChunks.SetupMemoryRequirements(MaxChunks);
    pool.ExhaustedChunks.SetupMemoryRequirements(MaxChunks);

    size_t totalSizeInBytes = 0;
    const size_t committedChunksOffsetInWords = 0;
    totalSizeInBytes += pool.CommittedChunks.AllocationSize();
    Assert(Meta::IsAligned(sizeof(FBitTree_::word_t), totalSizeInBytes));
    const size_t exhaustedChunksOffsetInWords = (totalSizeInBytes / sizeof(FBitTree_::word_t));
    totalSizeInBytes += ROUND_TO_NEXT_16(pool.ExhaustedChunks.AllocationSize());
    Assert(Meta::IsAligned(sizeof(FPoolChunk_*), totalSizeInBytes));
    const size_t chunksOffsetInPtrs = (totalSizeInBytes / sizeof(FPoolChunk_*));
    totalSizeInBytes += sizeof(FPoolChunk_*) * MaxChunks;

    const FAllocatorBlock blk = allocator_traits::Allocate(pool,
        allocator_traits::SnapSize(pool, totalSizeInBytes) );
    pool.CommittedChunks.Initialize(static_cast<FBitTree_::word_t*>(blk.Data) + committedChunksOffsetInWords, false);
    pool.ExhaustedChunks.Initialize(static_cast<FBitTree_::word_t*>(blk.Data) + exhaustedChunksOffsetInWords, true);
    pool.pChunks = (static_cast<FPoolChunk_**>(blk.Data) + chunksOffsetInPtrs);

    Broadcast(TMemoryView(pool.pChunks, MaxChunks), nullptr);

    forrange(i, 0, BundleMaxGarbage)
        _recycler.FreeBundles[i].store(UMax, std::memory_order_relaxed);

    std::atomic_thread_fence(std::memory_order_release);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::Allocate(u32 bucket) -> index_type {
    const index_type freeBlock = _buckets[bucket].LockExclusive()->PopFromFront(_pool.Value_NotThreadSafe().pChunks);

    if (UMax != freeBlock) {
        ONLY_IF_MEMORYDOMAINS( _pool.Value_NotThreadSafe().TrackingData.AllocateUser(BlockSize) );
        ONLY_IF_ASSERT( _pool.Value_NotThreadSafe().NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) );

        return freeBlock;
    }

    return AllocateFromNewBundle_AssumeUnlocked_(bucket);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::Deallocate(index_type block, u32 bucket) {
    Assert(block < MaxSize);

    FPoolNode_* node;
    {
        node = PoolNode_(_pool.Value_NotThreadSafe().pChunks, block);
        ONLY_IF_ASSERT( FPlatformMemory::Memdeadbeef(node, BlockSize) );

        if (_buckets[bucket].LockExclusive()->PushToFront(block, node)) {
            ONLY_IF_MEMORYDOMAINS( _pool.Value_NotThreadSafe().TrackingData.DeallocateUser(BlockSize) );
            Assert_NoAssume( _pool.Value_NotThreadSafe().NumLiveBlocks.fetch_sub(1, std::memory_order_relaxed) > 0 );
            return;
        }
    }

    RecyclePartialBundle_AssumeUnlocked_(bucket, block, node);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::Clear_AssertCompletelyEmpty() {
    const auto exclusivePool = _pool.LockExclusive();

    Assert_NoAssume(0 == exclusivePool->NumLiveBlocks); // some blocks are still alive!

    Clear_ReleaseMemory_AssumeLocked_(*exclusivePool);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::Clear_IgnoreLeaks() {
    Clear_ReleaseMemory_AssumeLocked_(*_pool.LockExclusive());
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool) {
    ONLY_IF_ASSERT(pool.NumLiveBlocks.store(0, std::memory_order_relaxed));

    pool.ReleaseAllChunks_IgnoreLeaks();

    for (auto& bucket : _buckets) {
        const auto exclusiveBucket = bucket.LockExclusive();

        exclusiveBucket->PartialBundle.Reset();
        exclusiveBucket->FullBundle.Reset();
    }

    forrange(i, 0, BundleMaxGarbage)
        _recycler.FreeBundles[i].store(UMax, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::ReleaseCacheMemory() {
    const auto exclusivePool = _pool.LockExclusive();

    for(auto& bucket : _buckets) {
        const auto exclusiveBucket = bucket.LockExclusive();

        for (index_type freeBundles = exclusiveBucket->PopBundles(exclusivePool->pChunks); UMax != freeBundles; ) {
            const index_type nextBundles = PoolNode_(exclusivePool->pChunks, freeBundles)->NextBundle;
            exclusivePool->ReleaseBundle(freeBundles);
            freeBundles = nextBundles;
        }
    }

    index_type freeBundles[BundleMaxGarbage];
    const index_type numGCBBundles = _recycler.StealBundles(freeBundles);

    forrange(gc, 0, numGCBBundles)
        exclusivePool->ReleaseBundle(freeBundles[gc]);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::AllocateFromNewBundle_AssumeUnlocked_(u32 firstBucket) -> index_type {
    const auto exclusivePool = _pool.LockExclusive();

    forrange(i, 0, ThreadGranularity) {
        const auto bucket = ((firstBucket + i) % ThreadGranularity);
        const auto exclusiveBucket = _buckets[bucket].LockExclusive();

        if (exclusiveBucket->ObtainPartial(exclusivePool->pChunks, _recycler)) {
            Assert_NoAssume(not exclusiveBucket->PartialBundle.Empty());

            const index_type freeBlock = exclusiveBucket->PopFromFront(exclusivePool->pChunks);
            if (UMax != freeBlock) {
                ONLY_IF_MEMORYDOMAINS( exclusivePool->TrackingData.AllocateUser(BlockSize) );
                ONLY_IF_ASSERT( exclusivePool->NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) );

                return freeBlock;
            }

            AssertNotReached(); // but we just recycled a bundle?!
        }

        Assert_NoAssume(exclusiveBucket->PartialBundle.Empty());
        Assert_NoAssume(exclusiveBucket->FullBundle.Empty());

        FPoolBundle_ bundle = exclusivePool->AllocateBundle();
        if (Likely(not bundle.Empty())) {
            exclusiveBucket->PushBundle(std::move(bundle));

            const index_type freeBlock = exclusiveBucket->PopFromFront(exclusivePool->pChunks);
            Assert(UMax != freeBlock);
            ONLY_IF_MEMORYDOMAINS( exclusivePool->TrackingData.AllocateUser(BlockSize) );
            ONLY_IF_ASSERT( exclusivePool->NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) );

            return freeBlock;
        }
        else {
            // high memory pressure: let the thread pick in other buckets
        }
    }

    AssertReleaseFailed(L"Out-of-memory: can't allocate a new chunk and all buckets are empty");
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::RecyclePartialBundle_AssumeUnlocked_(u32 bucket, index_type block, FPoolNode_* node) {
    Assert(node);

    index_type needRecycling = UMax;
    {
        const auto exclusiveBucket = _buckets[bucket].LockExclusive();

        needRecycling  = exclusiveBucket->RecycleFull(_pool.Value_NotThreadSafe().pChunks, _recycler);

        Verify( exclusiveBucket->PushToFront(block, node) );

        ONLY_IF_MEMORYDOMAINS( _pool.Value_NotThreadSafe().TrackingData.DeallocateUser(BlockSize) );
        Assert_NoAssume( _pool.Value_NotThreadSafe().NumLiveBlocks.fetch_sub(1, std::memory_order_relaxed) > 0 );
    }

    if (Unlikely(UMax != needRecycling))
        _pool.LockExclusive()->ReleaseBundle(needRecycling);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::FFreeBlockList_::PushBundle(FPoolBundle_&& rbundle) {
    Assert(not rbundle.Empty());
    Assert_NoAssume(PartialBundle.Empty());
    Assert_NoAssume(FullBundle.Empty());

    PartialBundle = std::move(rbundle);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::FFreeBlockList_::PopBundles(FPoolChunk_** pChunks) -> index_type {
    const index_type partial = PartialBundle.Head;
    if (UMax != partial) {
        PartialBundle.Reset();
        PoolNode_(pChunks, partial)->NextBundle = UMax;
    }

    const index_type full = FullBundle.Head;
    if (UMax != full) {
        FullBundle.Reset();
        PoolNode_(pChunks, full)->NextBundle = UMax;
    }

    index_type result = partial;
    if (UMax != result)
        PoolNode_(pChunks, result)->NextBundle = full;
    else
        result = full;

    return result;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::FFreeBlockList_::ObtainPartial(FPoolChunk_** pChunks, FBundleRecycler_& recycler) {
    if (PartialBundle.Empty()) {
        if (not FullBundle.Empty()) {
            PartialBundle = FullBundle;
            FullBundle.Reset();

            Assert(PartialBundle.Count > 0);
            return true;
        }

        PartialBundle.Count = 0;
        PartialBundle.Head = recycler.PopBundle();

        if (UMax != PartialBundle.Head) {
            FPoolNode_* const node = PoolNode_(pChunks, PartialBundle.Head);
            PartialBundle.Count = node->BundleCount;
            node->NextBundle = UMax;

            Assert(PartialBundle.Count > 0);
            return true;
        }

        Assert(PartialBundle.Empty());
        Assert(FullBundle.Empty());
        return false;
    }

    Assert(not PartialBundle.Empty());
    return true;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::FFreeBlockList_::RecycleFull(FPoolChunk_** pChunks, FBundleRecycler_& recycler) -> index_type {
    index_type needRecycling = UMax;

    if (not FullBundle.Empty()) {
        FPoolNode_* const node = PoolNode_(pChunks, FullBundle.Head);
        node->BundleCount = FullBundle.Count;

        if (not recycler.PushBundle(FullBundle.Head)) {
            needRecycling = FullBundle.Head;
            node->NextBundle = UMax;
        }

        FullBundle.Reset();
    }

    return needRecycling;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
bool TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG

    const auto exclusivePool = const_cast<TMemoryPool*>(this)->_pool.LockExclusive();

    const auto validateBundle = [pChunks{exclusivePool->pChunks}](index_type head, index_type count) {
        index_type n = 0;

        for (; UMax != head; ++n) {
            const FPoolNode_* const node = PoolNode_(pChunks, head);
            head = node->NextNode;
        }

        AssertRelease(n == count);
    };

    index_type totalFreeBlocks = 0;
    index_type totalCommittedBlocks = 0;

    forrange(ch, 0, MaxChunks) {
        if (FPoolChunk_* const pChunk = exclusivePool->pChunks[ch]) {
            index_type chunkFreeBlocks = 0;

            validateBundle(pChunk->PartialBundle.Head, pChunk->PartialBundle.Count);

            if (not pChunk->PartialBundle.Empty())
                chunkFreeBlocks += pChunk->PartialBundle.Count;

            for (index_type bundle = pChunk->FullBundles; UMax != bundle; ) {
                validateBundle(bundle, BundleMaxCount);
                chunkFreeBlocks += BundleMaxCount;
                bundle = pChunk->ChunkNode(bundle)->NextBundle;
            }

            const index_type numBlocksCommitted = Min(
                checked_cast<index_type>(pChunk->HighestBundleUnused * BundleMaxCount), ChunkSize);

            AssertRelease(chunkFreeBlocks + pChunk->NumBlocksTaken == numBlocksCommitted);
            totalFreeBlocks += chunkFreeBlocks;
            totalCommittedBlocks += numBlocksCommitted;
        }
    }

    for (auto& bucket : _buckets) {
        const auto sharedBucket = bucket.LockShared();

        validateBundle(sharedBucket->PartialBundle.Head, sharedBucket->PartialBundle.Count);
        validateBundle(sharedBucket->FullBundle.Head, sharedBucket->FullBundle.Count);

        totalFreeBlocks += sharedBucket->PartialBundle.Count;
        totalFreeBlocks += sharedBucket->FullBundle.Count;
    }

    forrange(i, 0, BundleMaxGarbage) {
        const index_type head = _recycler.FreeBundles[i].load(std::memory_order_relaxed);
        if (UMax != head) {
            const index_type count = PoolNode_(exclusivePool->pChunks, head)->BundleCount;
            validateBundle(head, count);
            totalFreeBlocks += count;
        }
    }

    AssertRelease(totalFreeBlocks + exclusivePool->NumLiveBlocks == totalCommittedBlocks);

#endif
    return true;
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks, typename _Allocator>
class TTypedMemoryPool : public TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Allocator> {
    using parent_type = TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Allocator>;
public:
    using typename parent_type::allocator_type;
    using typename parent_type::index_type;
    using typename parent_type::block_type;

    using value_type = T;
    STATIC_ASSERT(sizeof(value_type) >= sizeof(typename parent_type::block_type));
    STATIC_ASSERT(alignof(typename parent_type::block_type) % alignof(value_type) == 0);

    using parent_type::BlockSize;
    using parent_type::ChunkSize;
    using parent_type::MaxChunks;
    using parent_type::MaxSize;

    using parent_type::Allocate;
    using parent_type::Deallocate;
    using parent_type::Clear_AssertCompletelyEmpty;
    using parent_type::CheckInvariants;
    using parent_type::ReleaseCacheMemory;

    TTypedMemoryPool() = default;

    value_type* At(index_type block) const NOEXCEPT {
        return static_cast<value_type*>(parent_type::At(block));
    }
    value_type* operator [](index_type block) const NOEXCEPT {
        return static_cast<value_type*>(parent_type::operator[](block));
    }

    void Clear_IgnoreLeaks() {
        Assert(std::is_trivially_destructible_v<T>);
        Clear_IgnoreLeaks_AssumePOD();
    }

    void Clear_IgnoreLeaks_AssumePOD() {
        parent_type::Clear_IgnoreLeaks();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
