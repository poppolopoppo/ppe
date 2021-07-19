#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/Array.h"
#include "Container/BitTree.h"
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
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
class TMemoryPool3 : Meta::FNonCopyableNorMovable, _Allocator {
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

    TMemoryPool3()
#if USE_PPE_MEMORYDOMAINS
    :   _pool(std::addressof(MEMORYDOMAIN_TRACKING_DATA(MemoryPool)))
#endif
    {
        InitializeInternalPool_(*_pool.LockExclusive());
    }
    explicit TMemoryPool3(const allocator_type& allocator) : allocator_type(allocator) { InitializeInternalPool_(*_pool.LockExclusive()); }
    explicit TMemoryPool3(allocator_type&& rallocator) : allocator_type(std::move(rallocator)) { InitializeInternalPool_(*_pool.LockExclusive());}

    ~TMemoryPool3();

    allocator_type& Allocator() { return (*this); }
    const allocator_type& Allocator() const { return (*this); }

#if USE_PPE_DEBUG
    u32 NumLiveBlocks_ForDebug() const { return _pool.LockShared()->NumLiveBlocks.load(std::memory_order_relaxed); }
#endif

    NODISCARD index_type Allocate();
    void Deallocate(index_type block);

    void* At(index_type block) const NOEXCEPT { return PoolBlock_(_pool.Value_NotThreadSafe(), block); }
    void* operator [](index_type block) const NOEXCEPT { return At(block); }

    void Clear_AssertCompletelyEmpty();
    void Clear_IgnoreLeaks();

    bool CheckInvariants() const;

private:
    using FChunksBitTree_ = TBitTree<size_t, false>;

    struct FPoolBlock_ {
        index_type NextBlock;
    };
    STATIC_ASSERT(sizeof(FPoolBlock_) <= BlockSize);

    static constexpr std::memory_order memorder_release_ =
        (EThreadBarrier_Safe(_Barrier)
            ? std::memory_order_release
            : std::memory_order_relaxed );

    STATIC_CONST_INTEGRAL(u32, BundleMaxCount, 32);
    STATIC_CONST_INTEGRAL(u32, BundleMaxSizeInBytes, 2_KiB);
    FORCE_INLINE static bool IsBundleFull_(u32 count) {
        return (count >= BundleMaxCount ||
                count * sizeof(block_type) >= BundleMaxSizeInBytes );
    }

    template <bool _NeedAtomic>
    struct TPoolBundle_ {
        using counter_t = Meta::TConditional<_NeedAtomic, std::atomic<index_type>, index_type>;
        counter_t Head{ static_cast<index_type>(UMax) };
        counter_t Count{ 0 };
    };
    using FPoolBundle_ = TPoolBundle_<false>;
    using FAtomicPoolBundle_ = TPoolBundle_<true>;

    struct FInternalPool_ {
        mutable FAtomicPoolBundle_ PartialBundle;
        block_type** pBlocks{ nullptr };
        FPoolBundle_ FullBundle;
        FPoolBundle_* pChunks{ nullptr };
        FChunksBitTree_ FreeChunks;
        FChunksBitTree_ CommittedChunks;

#if USE_PPE_DEBUG
        mutable std::atomic<u32> NumLiveBlocks{ 0 };
#endif

#if USE_PPE_MEMORYDOMAINS
        mutable FMemoryTracking TrackingData;
        FInternalPool_(FMemoryTracking* parent) : TrackingData(Meta::type_info<TMemoryPool3>.name, parent) {
            RegisterTrackingData(&TrackingData);
        }
        ~FInternalPool_() {
            UnregisterTrackingData(&TrackingData);
        }
#endif
    };

    FORCE_INLINE static FPoolBlock_* PoolBlock_(const FInternalPool_& pool, index_type id) NOEXCEPT {
        Assert(id < MaxSize);
        const index_type ch = (id / ChunkSize);
        Assert(ch < MaxChunks);
        return reinterpret_cast<FPoolBlock_*>(pool.pBlocks[ch] + (id - ch * ChunkSize));
    }

    void InitializeInternalPool_(FInternalPool_& pool);
    NODISCARD NO_INLINE index_type AllocateFromNewBundle_AssumeUnlocked_();
    NO_INLINE void RecyclePartialBundle_AssumeUnlocked_();
    NO_INLINE void ReleaseFullBundle_(FInternalPool_& pool);
    FORCE_INLINE void Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool);
    FORCE_INLINE void DeallocateAllChunks_AssumeLocked_(FInternalPool_& pool);

    TThreadSafe<FInternalPool_, _Barrier> _pool;
};
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::~TMemoryPool3() {
    const auto exclusive = _pool.LockExclusive();

    Assert_NoAssume(0 == exclusive->NumLiveBlocks); // some blocks are still alive!

    DeallocateAllChunks_AssumeLocked_(*exclusive);

    size_t totalSizeInBytes = 0;
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(exclusive->FreeChunks.AllocationSize());
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(exclusive->CommittedChunks.AllocationSize());
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(block_type*) * MaxChunks);
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(FPoolBundle_) * MaxChunks);

    FAllocatorBlock blk{ exclusive->FreeChunks.Bits, totalSizeInBytes };
    allocator_traits::Deallocate(*this, blk);

#if USE_PPE_ASSERT
    exclusive->FreeChunks = Default;
    exclusive->CommittedChunks = Default;
    exclusive->pChunks = nullptr;
    exclusive->pBlocks = nullptr;

    FPlatformMemory::Memdeadbeef(&exclusive->PartialBundle, sizeof(exclusive->PartialBundle)); // crash if called after destruction
#endif
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Allocate() -> index_type {
    // fast path: look in the current partial bundle atomically, with a shared lock

    {
        const auto shared = _pool.LockShared();
        FAtomicPoolBundle_& bundle = shared->PartialBundle;

        for (index_type id = bundle.Head.load(std::memory_order_relaxed); id != UMax; ) {
            Assert(id < MaxSize);

            if (bundle.Head.compare_exchange_weak(id, PoolBlock_(*shared, id)->NextBlock,
                    memorder_release_, std::memory_order_relaxed )) {
                ONLY_IF_MEMORYDOMAINS( shared->TrackingData.AllocateUser(sizeof(block_type)) );

                Verify(bundle.Count.fetch_sub(1, std::memory_order_relaxed) > 0);
                ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(PoolBlock_(*shared, id), BlockSize));
                ONLY_IF_ASSERT(shared->NumLiveBlocks.fetch_add(1, std::memory_order_relaxed));
                return id;
            }
        }
    }

    // if fails, fallback on a new bundle with an exclusive lock

    return AllocateFromNewBundle_AssumeUnlocked_();
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Deallocate(index_type block) {
    Assert(block < MaxSize);

    {
        const auto shared = _pool.LockShared();
        FAtomicPoolBundle_& bundle = shared->PartialBundle;

        FPoolBlock_* const pBlock = PoolBlock_(*shared, block);
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(pBlock, BlockSize));

        pBlock->NextBlock = bundle.Head.load(std::memory_order_relaxed);
        for (;;) {
            Assert_NoAssume(pBlock->NextBlock == UMax || pBlock->NextBlock < MaxSize);

            if (bundle.Head.compare_exchange_weak(pBlock->NextBlock, block,
                    memorder_release_, std::memory_order_relaxed )) {
                ONLY_IF_MEMORYDOMAINS( shared->TrackingData.DeallocateUser(sizeof(block_type)) );
                Assert_NoAssume(shared->NumLiveBlocks.fetch_sub(1, std::memory_order_relaxed) > 0);

                if (Unlikely( IsBundleFull_(bundle.Count.fetch_add(1, std::memory_order_relaxed) + 1) ))
                    break;

                return;
            }
        }
    }

    RecyclePartialBundle_AssumeUnlocked_();
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_AssertCompletelyEmpty() {
    const auto exclusive = _pool.LockExclusive();

    Assert_NoAssume(0 == exclusive->NumLiveBlocks); // some blocks are still alive!

    Clear_ReleaseMemory_AssumeLocked_(*exclusive);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_IgnoreLeaks() {
    Clear_ReleaseMemory_AssumeLocked_(*_pool.LockExclusive());
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool) {
    ONLY_IF_ASSERT(pool.NumLiveBlocks.store(0, std::memory_order_relaxed));

    DeallocateAllChunks_AssumeLocked_(pool);

    pool.PartialBundle.Head.store(UMax, std::memory_order_relaxed);
    pool.PartialBundle.Count.store(0, std::memory_order_relaxed);

    pool.FullBundle.Head = UMax;
    pool.FullBundle.Count = 0;

    pool.FreeChunks.Initialize(pool.FreeChunks.Bits, true);
    pool.CommittedChunks.Initialize(pool.CommittedChunks.Bits, false);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
bool TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::CheckInvariants() const {
    STATIC_ASSERT(std::atomic<index_type>::is_always_lock_free);
    STATIC_ASSERT(Meta::IsAligned(FChunksBitTree_::WordBitCount, ChunkSize));
#if USE_PPE_DEBUG
    const auto exclusive = const_cast<TMemoryPool3*>(this)->_pool.LockExclusive();
    const FInternalPool_& pool = *exclusive;

    const auto bundleLength = [&pool](index_type id) NOEXCEPT {
        index_type len = 0;
        for (; id != UMax; id = PoolBlock_(pool, id)->NextBlock)
            ++len;
        return len;
    };

    index_type numFreeBlocks = 0;
    numFreeBlocks += pool.PartialBundle.Count.load(std::memory_order_relaxed);
    Assert_NoAssume(bundleLength(pool.PartialBundle.Head.load(std::memory_order_relaxed)) == numFreeBlocks);
    numFreeBlocks += pool.FullBundle.Count;
    Assert_NoAssume(bundleLength(pool.FullBundle.Head) == pool.FullBundle.Count);

    for(auto ch = pool.FreeChunks.NextAllocateBit();
        ch != UMax;
        ch = pool.FreeChunks.NextAllocateBit(ch + 1) ) {

        const FPoolBundle_& bundle = pool.pChunks[ch];
        Assert_NoAssume(bundle.Count > 0);
        Assert_NoAssume(bundleLength(bundle.Head) == bundle.Count);

        numFreeBlocks += bundle.Count;
    }

    const index_type numCommittedBlocks = checked_cast<index_type>(pool.CommittedChunks.CountOnes(MaxChunks) * ChunkSize);
    const index_type numLiveBlocks = checked_cast<index_type>(pool.NumLiveBlocks.load(std::memory_order_relaxed));
    Assert_NoAssume(numLiveBlocks + numFreeBlocks == numCommittedBlocks);

#endif
    return true;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::AllocateFromNewBundle_AssumeUnlocked_() -> index_type {
    const auto exclusive = _pool.LockExclusive();
    FInternalPool_& pool = *exclusive;

    if (pool.PartialBundle.Count.load(std::memory_order_relaxed) == 0) {
        Assert_NoAssume(pool.PartialBundle.Head.load(std::memory_order_relaxed) == UMax);

        if (pool.FullBundle.Count > 0) {
            Assert_NoAssume(pool.FullBundle.Head != UMax);

            // steal the full bundle

            pool.PartialBundle.Head.store(pool.FullBundle.Head, std::memory_order_relaxed);
            pool.PartialBundle.Count.store(pool.FullBundle.Count, std::memory_order_relaxed);
            pool.FullBundle = Default;
        }
        else {
            Assert_NoAssume(pool.FullBundle.Head == UMax);

            // look for a chunk already committed with free space

            auto ch = pool.FreeChunks.NextAllocateBit();
            if (UMax == ch) {

                // need to allocate a new chunk

                ch = pool.CommittedChunks.NextAllocateBit();
                AssertRelease(L"OOM in pool, no more chunk to allocate", UMax != ch);
                Assert_NoAssume(UMax == pool.pChunks[ch].Head);
                Assert_NoAssume(0 == pool.pChunks[ch].Count);
                Assert(nullptr == pool.pBlocks[ch]);

                const FAllocatorBlock blk = allocator_traits::Allocate(*this, ChunkSize * BlockSize );

                ONLY_IF_MEMORYDOMAINS( exclusive->TrackingData.AllocateSystem(blk.SizeInBytes) );

                pool.CommittedChunks.AllocateBit(ch);
                pool.FreeChunks.Deallocate(ch);
                pool.pBlocks[ch] = static_cast<block_type*>(blk.Data);

                // populate the new bundle

                forrange(i, 0, ChunkSize - 1) {
                    auto* const p = reinterpret_cast<FPoolBlock_*>(pool.pBlocks[ch] + i);
                    p->NextBlock = checked_cast<index_type>(ch * ChunkSize + i + 1);
                }
                {
                    auto* const p = reinterpret_cast<FPoolBlock_*>(pool.pBlocks[ch] + ChunkSize - 1);
                    p->NextBlock = UMax;
                }

                FPoolBundle_& bundle = pool.pChunks[ch];
                bundle.Head = checked_cast<index_type>(ch * ChunkSize);
                bundle.Count = ChunkSize;
            }

            Assert_NoAssume(pool.CommittedChunks.IsAllocated(ch));
            FPoolBundle_& bundle = pool.pChunks[ch];
            Assert_NoAssume(UMax != bundle.Head);
            Assert_NoAssume(0 < bundle.Count);
            Assert_NoAssume(ChunkSize >= bundle.Count);

            pool.FreeChunks.AllocateBit(ch); // dry out the chunk, no more space available

            pool.PartialBundle.Head.store(bundle.Head, std::memory_order_relaxed);
            pool.PartialBundle.Count.store(bundle.Count, std::memory_order_relaxed);

            bundle.Head = UMax;
            bundle.Count = 0;
        }
    }

    Assert_NoAssume(pool.PartialBundle.Count.load(std::memory_order_relaxed) > 0);
    ONLY_IF_MEMORYDOMAINS( pool.TrackingData.AllocateUser(sizeof(block_type)) );

    const index_type id = pool.PartialBundle.Head.load(std::memory_order_relaxed);
    const index_type nextBlock = PoolBlock_(pool, id)->NextBlock;
    Assert_NoAssume(nextBlock < MaxSize || nextBlock == UMax);

    pool.PartialBundle.Head.store(nextBlock, std::memory_order_relaxed);
    Verify(pool.PartialBundle.Count.fetch_sub(1, std::memory_order_relaxed) > 0);

    ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(PoolBlock_(*exclusive, id), BlockSize));
    ONLY_IF_ASSERT(pool.NumLiveBlocks.fetch_add(1, std::memory_order_relaxed));

    return id;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::ReleaseFullBundle_(FInternalPool_& pool) {
    Assert(pool.FullBundle.Count > 0);

    // give back the blocks listed in full bundle to each owning chunk

    ONLY_IF_ASSERT(index_type countForDebug = pool.FullBundle.Count);
    for (index_type id = pool.FullBundle.Head, nextBlockInBundle = UMax; id != UMax; id = nextBlockInBundle) {
        Assert(id < MaxSize);
        Assert_NoAssume(countForDebug-- > 0);

        FPoolBlock_* const pBlock = PoolBlock_(pool, id);
        const index_type ch = (id / ChunkSize);
        Assert(ch < MaxChunks);
        Assert_NoAssume(FPlatformMemory::Memaliases(pool.pBlocks[ch], ChunkSize * BlockSize, pBlock));

        nextBlockInBundle = pBlock->NextBlock;

        FPoolBundle_& bundle = pool.pChunks[ch];
        Assert(bundle.Count < ChunkSize);
        Assert_NoAssume(pool.CommittedChunks.IsAllocated(ch));

        pBlock->NextBlock = bundle.Head;
        bundle.Head = id;
        ++bundle.Count;

        if (Unlikely(1 == bundle.Count)) {
            // mark the chunk as non-full

            pool.FreeChunks.Deallocate(ch);
        }
        else if (Unlikely(ChunkSize == bundle.Count)) {
            // release the chunk if it's completely unused

            pool.FreeChunks.AllocateBit(ch);
            pool.CommittedChunks.Deallocate(ch);

            FAllocatorBlock blk;
            blk.Data = pool.pBlocks[ch];
            blk.SizeInBytes = (ChunkSize * BlockSize);

            ONLY_IF_MEMORYDOMAINS( pool.TrackingData.DeallocateSystem(blk.SizeInBytes) );

            allocator_traits::Deallocate(*this, blk);

            bundle.Head = UMax;
            bundle.Count = 0;

            pool.pBlocks[ch] = nullptr;
        }
    }
    Assert_NoAssume(0 == countForDebug);

    pool.FullBundle.Head = UMax;
    pool.FullBundle.Count = 0;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::RecyclePartialBundle_AssumeUnlocked_() {
    const auto exclusive = _pool.LockExclusive();

    if (exclusive->PartialBundle.Count.load(std::memory_order_relaxed) > 0) {
        if (exclusive->FullBundle.Count > 0)
            ReleaseFullBundle_(*exclusive);

        Assert_NoAssume(0 == exclusive->FullBundle.Count);

        // overwrite full bundle with partial bundle

        exclusive->FullBundle.Head = exclusive->PartialBundle.Head.load(std::memory_order_relaxed);
        exclusive->FullBundle.Count = exclusive->PartialBundle.Count.load(std::memory_order_relaxed);

        exclusive->PartialBundle.Head.store(UMax, std::memory_order_relaxed);
        exclusive->PartialBundle.Count.store(0, std::memory_order_relaxed);
    }

    Assert_NoAssume(exclusive->PartialBundle.Count.load(std::memory_order_relaxed) < ChunkSize);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::DeallocateAllChunks_AssumeLocked_(FInternalPool_& pool) {

    ONLY_IF_MEMORYDOMAINS( pool.TrackingData.ReleaseAllUser() ); // ignore leaks here (checked before)

    forrange(ch, 0, MaxChunks) {
        if (block_type* const pBlocks = pool.pBlocks[ch]) {
            FAllocatorBlock blk{ pBlocks, ChunkSize * BlockSize };

            ONLY_IF_MEMORYDOMAINS( pool.TrackingData.DeallocateSystem(blk.SizeInBytes) );

            allocator_traits::Deallocate(*this, blk);

            pool.pBlocks[ch] = nullptr;
            pool.pChunks[ch] = Default;
        }
    }
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool3<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::InitializeInternalPool_(FInternalPool_& pool) {
    STATIC_ASSERT(std::atomic<index_type>::is_always_lock_free);

    Assert_NoAssume(UMax == pool.PartialBundle.Head.load(std::memory_order_relaxed));
    Assert_NoAssume(UMax == pool.FullBundle.Head);
    Assert_NoAssume(nullptr == pool.FreeChunks.Bits);
    Assert_NoAssume(nullptr == pool.pBlocks);
    Assert_NoAssume(nullptr == pool.pChunks);

    pool.FreeChunks.SetupMemoryRequirements(MaxChunks);
    pool.CommittedChunks.SetupMemoryRequirements(MaxChunks);

    size_t totalSizeInBytes = 0;
    const size_t freeChunksOffsetInWords = 0;
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(pool.FreeChunks.AllocationSize());
    Assert(Meta::IsAligned(sizeof(FChunksBitTree_::word_t), totalSizeInBytes));
    const size_t committedChunksOffsetInWords = totalSizeInBytes;
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(pool.CommittedChunks.AllocationSize());
    Assert(Meta::IsAligned(sizeof(block_type**), totalSizeInBytes));
    const size_t blocksOffsetInBlocks = (totalSizeInBytes / sizeof(block_type*));
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(block_type*) * MaxChunks);
    Assert(Meta::IsAligned(sizeof(FPoolBundle_), totalSizeInBytes));
    const size_t chunksOffsetInBundles = (totalSizeInBytes / sizeof(FPoolBundle_));
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(FPoolBundle_) * MaxChunks);

    const FAllocatorBlock blk = allocator_traits::Allocate(*this, totalSizeInBytes);
    pool.FreeChunks.Initialize(static_cast<FChunksBitTree_::word_t*>(blk.Data) + freeChunksOffsetInWords, true);
    pool.CommittedChunks.Initialize(static_cast<FChunksBitTree_::word_t*>(blk.Data) + committedChunksOffsetInWords, false);
    pool.pBlocks = (static_cast<block_type**>(blk.Data) + blocksOffsetInBlocks);
    pool.pChunks = (static_cast<FPoolBundle_*>(blk.Data) + chunksOffsetInBundles);

    Broadcast(TMemoryView(pool.pBlocks, MaxChunks), nullptr);
    Broadcast(TMemoryView(pool.pChunks, MaxChunks), Default);
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
class TTypedMemoryPool3 : public TMemoryPool3<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator> {
    using parent_type = TMemoryPool3<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator>;
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

    TTypedMemoryPool3() = default;

    explicit TTypedMemoryPool3(const allocator_type& allocator) : parent_type(allocator) {}
    explicit TTypedMemoryPool3(allocator_type&& rallocator) : parent_type(std::move(rallocator)) {}

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
