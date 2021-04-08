#pragma once

#include "Core.h"

#include "Container/Array.h"
#include "Container/FixedSizeHashTable.h"
#include "Meta/Functor.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
class TMemoryPool : Meta::FNonCopyableNorMovable, _Allocator {
public:
    STATIC_ASSERT(Meta::IsPow2(_ChunkSize));
    STATIC_ASSERT(_BlockSize > 0);
    STATIC_ASSERT(_MaxChunks > 0);

    STATIC_CONST_INTEGRAL(size_t, MaxSize, _ChunkSize * _MaxChunks);

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using block_type = std::aligned_storage_t<_BlockSize, _Align>;
    using index_type = std::conditional_t<(MaxSize > UINT32_MAX), u64,
        std::conditional_t<(MaxSize > UINT16_MAX), u32, u16> >;
    STATIC_ASSERT(sizeof(block_type) >= sizeof(index_type));

    STATIC_CONST_INTEGRAL(index_type, BlockSize, static_cast<index_type>(_BlockSize));
    STATIC_CONST_INTEGRAL(index_type, ChunkSize, static_cast<index_type>(_ChunkSize));
    STATIC_CONST_INTEGRAL(index_type, MaxChunks, static_cast<index_type>(_MaxChunks));

    TMemoryPool() {
        Broadcast(_state.Value_NotThreadSafe()._blocks.MakeView(), nullptr);
    }

    TMemoryPool(const allocator_type& allocator) : allocator_type(allocator) {}
    TMemoryPool(allocator_type&& rallocator) : allocator_type(std::move(rallocator)) {}

    ~TMemoryPool() {
        Clear_AssertCompletelyEmpty();
    }

    NODISCARD index_type Allocate();
    void Deallocate(index_type block);

    block_type* At(index_type block) const NOEXCEPT;
    const block_type* operator [](index_type id) const NOEXCEPT;

    template <typename _ForEach>
    void Each(_ForEach&& pred);

    void Clear_AssertCompletelyEmpty();
    void Clear_IgnoreLeaks();

    bool CheckInvariants() const;

private:
    using chunk_type = TStaticArray<block_type, _ChunkSize>;

    struct CACHELINE_ALIGNED FPool_ {
        std::atomic<index_type> FreeList{ ChunkSize };
        std::atomic<index_type> NumLiveBlocks{ 0u };
    };

    struct FInternalState_ {
        index_type _spareChunk{ MaxChunks }; // hysteresis for free chunk deallocation
        TStaticArray<block_type*, _MaxChunks> _blocks; // storage only (cache friendly)
        mutable TStaticArray<FPool_, _MaxChunks> _pools; // atomic free block linked lists
    };

    static constexpr std::memory_order memorder_release_ = (_Safe
        ? std::memory_order_release
        : std::memory_order_relaxed );

    TThreadSafe<FInternalState_, _Safe> _state;

    NO_INLINE index_type AllocateBlockFromNewChunk_();
    NO_INLINE void ReleaseFreeChunk_(index_type chk);
    FORCE_INLINE void Clear_ReleaseMemory_AssumeLocked_(FInternalState_& state);
};
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Allocate() -> index_type {
    {
        const auto shared(_state.LockShared());

        forrange(chk, 0, MaxChunks) {
            FPool_& pool = shared->_pools[chk];

            index_type id = pool.FreeList.load(std::memory_order_relaxed);
            for (; id < ChunkSize; ) {
                block_type* const pblocks = shared->_blocks[chk];
                Assert(pblocks);

                const index_type nextId = reinterpret_cast<const index_type&>(pblocks[id]);
                if (pool.FreeList.compare_exchange_weak(id, nextId,
                                                        memorder_release_,
                                                        std::memory_order_relaxed)) {

                    Verify(pool.NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) < _ChunkSize);
                    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(pblocks + id, sizeof(block_type)));

                    return (chk * ChunkSize + id); // return global index in the pool (vs local chunk)
                }
            }
        }
    }

    return AllocateBlockFromNewChunk_(); // cold path
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Deallocate(index_type block) {
    Assert(block < MaxSize);
    const index_type chk = (block / ChunkSize);
    const index_type id = (block - chk * ChunkSize);

    bool releaseChunk;
    {
        const auto shared(_state.LockShared());

        block_type* const pblocks = shared->_blocks[chk];
        Assert(pblocks);

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(pblocks + id, sizeof(block_type)));
        index_type& nextId = reinterpret_cast<index_type&>(pblocks[id]);

        FPool_& pool = shared->_pools[chk];
        nextId = pool.FreeList.load(std::memory_order_relaxed);
        for (;;) {
            Assert_NoAssume(nextId != id);
            Assert_NoAssume(nextId <= ChunkSize);
            if (pool.FreeList.compare_exchange_weak(nextId, id,
                                                    memorder_release_,
                                                    std::memory_order_relaxed)) {
                releaseChunk = (pool.NumLiveBlocks.fetch_sub(1, std::memory_order_relaxed) == 1);
                break;
            }
        }
    }

    if (Unlikely(releaseChunk))
        ReleaseFreeChunk_(chk); // cold path
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::At(index_type block) const NOEXCEPT -> block_type* {
    Assert(block < MaxSize);
    const index_type chk = (block / ChunkSize);
    const index_type id = (block - chk * ChunkSize);

#if 0 // This should not be necessary due to pool lifetime guarantee
    const auto shared(_state.LockShared());
#else
    const FInternalState_* shared = &_state.Value_NotThreadSafe();
#endif

    Assert(shared->_blocks[chk]);
    Assert_NoAssume(shared->_pools[chk].NumLiveBlocks.load(std::memory_order_relaxed) > 0);

    return (shared->_blocks[chk] + id);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
const typename TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe, _Allocator>::block_type* TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe, _Allocator>::operator[](index_type block) const noexcept {
    if (block >= MaxSize)
        return nullptr;

    return At(block);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
template <typename _ForEach>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Each(_ForEach&& pred) {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    TFixedSizeHashSet<index_type, _ChunkSize> freeBlocks;

    forrange(chk, 0, MaxChunks) {
        block_type* const pblocks = exclusive->_blocks[chk];
        if (not pblocks)
            continue;

        const FPool_& pool = exclusive->_pools[chk];
        if (0 == pool.NumLiveBlocks.load(std::memory_order_relaxed))
            continue;

        freeBlocks.clear();

        for (   index_type id = pool.FreeList.load(std::memory_order_relaxed);
                id < ChunkSize;
                id = reinterpret_cast<const index_type&>(pblocks[id]) )
            freeBlocks.Add_AssertUnique(id);

        forrange(id, 0, ChunkSize) {
            if (not freeBlocks.Contains(id))
                Meta::VariadicFunctor(pred, pblocks + id, static_cast<index_type>(chk * ChunkSize + id));
        }
    }
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Clear_AssertCompletelyEmpty() {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

#if USE_PPE_DEBUG
    forrange(chk, 0, MaxChunks) {
        const FPool_& pool = exclusive->_pools[chk];
        Assert(pool.NumLiveBlocks.load(std::memory_order_relaxed) == 0);
    }
#endif

    Clear_ReleaseMemory_AssumeLocked_(*exclusive);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Clear_IgnoreLeaks() {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    Clear_ReleaseMemory_AssumeLocked_(*exclusive);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
bool TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    const auto exclusive(const_cast<decltype(_state)&>(_state).LockExclusive()); ; // exclusive access is needed for safety

    index_type totalLiveChunks = 0;
    index_type totalLiveBlocks = 0;
    index_type totalFreeBlocks = 0;
    forrange(chk, 0, _MaxChunks) {
        const FPool_& pool = exclusive->_pools[chk];
        const block_type* const pblocks = exclusive->_blocks[chk];

        if (pblocks)
            totalLiveChunks++;

        index_type numFreeBlocks = 0;
        for (index_type id = pool.FreeList.load(std::memory_order_relaxed); id < ChunkSize;) {
            const index_type nextId = reinterpret_cast<const index_type&>(pblocks[id]);
            id = nextId;
            numFreeBlocks++;
            Assert_NoAssume(numFreeBlocks <= ChunkSize); // detect loops
        }

        const index_type numLiveBlocks = pool.NumLiveBlocks.load(std::memory_order_relaxed);
        Assert_NoAssume(!pblocks || numLiveBlocks + numFreeBlocks == ChunkSize);

        totalLiveBlocks += numLiveBlocks;
        totalFreeBlocks += numFreeBlocks;
    }

    Assert_NoAssume(totalFreeBlocks + totalLiveBlocks == totalLiveChunks * ChunkSize);
#endif

    return true;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::AllocateBlockFromNewChunk_() -> index_type {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    // check if somebody allocated something meanwhile due to the exclusive barrier
    index_type nextChunkToAlloc = MaxChunks;
    forrange(chk, 0, MaxChunks) {
        block_type* const pblocks = exclusive->_blocks[chk];
        if (nullptr == pblocks) {
            nextChunkToAlloc = Min(chk, nextChunkToAlloc);
            continue;
        }

        FPool_& pool = exclusive->_pools[chk];
        index_type freeBlock = pool.FreeList.load(std::memory_order_relaxed);

        if (Unlikely(freeBlock < ChunkSize)) {
            // found a free block, skip chunk allocation (lock contention)
            const index_type nextId = reinterpret_cast<const index_type&>(pblocks[freeBlock]);
            pool.FreeList.store(nextId, std::memory_order_relaxed);

            Verify(pool.NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) < ChunkSize);
            return static_cast<index_type>(chk * ChunkSize + freeBlock); // return global index in the pool (vs local chunk)
        }
    }

    AssertRelease(nextChunkToAlloc < MaxChunks); // OOM, no more space available in pool
    Assert(nullptr == exclusive->_blocks[nextChunkToAlloc]);

    // nothing available, need to allocate a new chunk
    auto& allocator = static_cast<_Allocator&>(*this);
    block_type* const newChunk = allocator_traits::template AllocateT<block_type>(allocator, _ChunkSize).data();

    // initialize free list indices
    forrange(blk, 0, ChunkSize)
        reinterpret_cast<index_type&>(newChunk[blk]) = static_cast<index_type>(blk + 1);

    // initialize the pool, and allocate the first block for this thread
    FPool_& pool = exclusive->_pools[nextChunkToAlloc];
    Assert_NoAssume(pool.NumLiveBlocks.load(std::memory_order_relaxed) == 0);
    Assert_NoAssume(pool.FreeList.load(std::memory_order_relaxed) == ChunkSize);

    pool.NumLiveBlocks.store(1, std::memory_order_relaxed);
    pool.FreeList.store(1, std::memory_order_relaxed);

    // finally register allocation pointer and return the new global block index
    exclusive->_blocks[nextChunkToAlloc] = newChunk;
    return (nextChunkToAlloc * ChunkSize + 0/* allocated the first block */);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::ReleaseFreeChunk_(index_type chk) {
    Assert(chk < MaxChunks);

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    if (nullptr == exclusive->_blocks[chk])
        return;

    // check that spare chunk is still completely free
    if (exclusive->_spareChunk < MaxChunks &&
        exclusive->_pools[exclusive->_spareChunk].NumLiveBlocks.load(std::memory_order_relaxed) > 0) {
        exclusive->_spareChunk = MaxChunks; // spare chunk was used, revert bookkeeping
    }

    // hysteresis to always keep one spare chunk
    if (exclusive->_spareChunk < MaxChunks) {
        if (exclusive->_spareChunk == chk)
            return;

        if (chk > exclusive->_spareChunk)
            std::swap(chk, exclusive->_spareChunk); // keep this chunk and remove old one
    }
    else {
        exclusive->_spareChunk = chk;
        return; // early-out, let other thread consume this chunk if needed
    }

    block_type* const pblocks = exclusive->_blocks[chk];
    AssertRelease(pblocks);

    FPool_& pool = exclusive->_pools[chk];
    Assert_NoAssume(pool.NumLiveBlocks.load(std::memory_order_relaxed) == 0);
    Assert_NoAssume(pool.FreeList.load(std::memory_order_relaxed) < _ChunkSize);

#if USE_PPE_DEBUG
    // check that every block from the chunk are present in the free list
    index_type numFreeBlocks = 0;
    index_type blk = pool.FreeList.load(std::memory_order_relaxed);
    while (blk < ChunkSize) {
        blk = reinterpret_cast<const index_type&>(pblocks[blk]);
        ++numFreeBlocks;
    }
    AssertRelease(ChunkSize == blk);
    AssertRelease(ChunkSize == numFreeBlocks);
#endif

    // reset free list in the pool
    pool.FreeList.store(ChunkSize, std::memory_order_relaxed);

    // release the allocated chunk
    auto& allocator = static_cast<_Allocator&>(*this);
    allocator_traits::DeallocateT(allocator, pblocks, _ChunkSize);

    exclusive->_blocks[chk] = nullptr;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Safe,_Allocator>::Clear_ReleaseMemory_AssumeLocked_(FInternalState_& state) {

    // reset spare chunk
    state._spareChunk = MaxChunks;

    // reset all pools
    for (FPool_& pool : state._pools) {
        pool.NumLiveBlocks.store(0, std::memory_order_relaxed);
        pool.FreeList.store(ChunkSize, std::memory_order_relaxed);
    }

    // release all blocks
    auto& allocator = static_cast<_Allocator&>(*this);
    for (block_type*& pblocks : state._blocks) {
        if (pblocks) {
            allocator_traits::DeallocateT(allocator, pblocks, _ChunkSize);
            pblocks = nullptr;
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks, bool _Safe, typename _Allocator>
class TTypedMemoryPool : public TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Safe,_Allocator> {
    using parent_type = TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Safe,_Allocator>;
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

    TTypedMemoryPool() = default;

    TTypedMemoryPool(const allocator_type& allocator) : parent_type(allocator) {}
    TTypedMemoryPool(allocator_type&& rallocator) : parent_type(std::move(rallocator)) {}

    value_type* At(index_type block) const NOEXCEPT {
        return reinterpret_cast<value_type*>(parent_type::At(block));
    }
    value_type* operator [](index_type id) const NOEXCEPT {
        return reinterpret_cast<value_type*>(parent_type::operator[](id));
    }

    template <typename _ForEach>
    void Each(_ForEach&& pred) {
        parent_type::Each([&pred](typename parent_type::block_type* storage, index_type block) {
            Meta::VariadicFunctor(pred, reinterpret_cast<value_type*>(storage), block);
        });
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
