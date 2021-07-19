#pragma once

#include "Core_fwd.h"

#include "Container/Array.h"
#include "Container/BitSet.h"
#include "Meta/Functor.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
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

    explicit TMemoryPool(const allocator_type& allocator) : allocator_type(allocator) {}
    explicit TMemoryPool(allocator_type&& rallocator) : allocator_type(std::move(rallocator)) {}

    ~TMemoryPool() {
        Clear_AssertCompletelyEmpty();
    }

    allocator_type& Allocator() { return (*this); }
    const allocator_type& Allocator() const { return (*this); }

    index_type NumCommittedBlocks() const NOEXCEPT;

    NODISCARD index_type Allocate();
    void Deallocate(index_type block);

    block_type* At(index_type block) const NOEXCEPT;
    block_type* operator [](index_type block) const NOEXCEPT;

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

    static constexpr std::memory_order memorder_release_ =
        (EThreadBarrier_Safe(_Barrier)
            ? std::memory_order_release
            : std::memory_order_relaxed );

    TThreadSafe<FInternalState_, _Barrier> _state;

    NO_INLINE index_type AllocateBlockFromNewChunk_();
    NO_INLINE void ReleaseFreeChunk_(index_type ch);
    FORCE_INLINE void Clear_ReleaseMemory_AssumeLocked_(FInternalState_& state);
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::NumCommittedBlocks() const NOEXCEPT -> index_type {
    index_type numCommittedChunks = 0;

    const auto shared(_state.LockShared());
    for (block_type* pBlocks : shared->_blocks) {
        if (pBlocks)
            ++numCommittedChunks;
    }

    return (numCommittedChunks * ChunkSize);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Allocate() -> index_type {
    {
        const auto shared(_state.LockShared());

        forrange(ch, 0, MaxChunks) {
            FPool_& pool = shared->_pools[ch];

            index_type id = pool.FreeList.load(std::memory_order_relaxed);
            for (; id < ChunkSize; ) {
                block_type* const pBlocks = shared->_blocks[ch];
                Assert(pBlocks);

                const index_type nextId = reinterpret_cast<const index_type&>(pBlocks[id]);
                if (pool.FreeList.compare_exchange_weak(id, nextId,
                                                        memorder_release_,
                                                        std::memory_order_relaxed)) {

                    Verify(pool.NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) < _ChunkSize);
                    ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(pBlocks + id, sizeof(block_type)));

                    ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateUser(sizeof(block_type)) );

                    return (ch * ChunkSize + id); // return global index in the pool (vs local chunk)
                }
            }
        }
    }

    return AllocateBlockFromNewChunk_(); // cold path
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Deallocate(index_type block) {
    Assert(block < MaxSize);
    const index_type ch = (block / ChunkSize);
    const index_type id = (block - ch * ChunkSize);

    bool releaseChunk;
    {
        const auto shared(_state.LockShared());

        block_type* const pBlocks = shared->_blocks[ch];
        Assert(pBlocks);

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(pBlocks + id, sizeof(block_type)));
        index_type& nextId = reinterpret_cast<index_type&>(pBlocks[id]);

        FPool_& pool = shared->_pools[ch];
        nextId = pool.FreeList.load(std::memory_order_relaxed);
        for (;;) {
            Assert_NoAssume(nextId != id);
            Assert_NoAssume(nextId <= ChunkSize);
            if (pool.FreeList.compare_exchange_weak(nextId, id,
                                                    memorder_release_,
                                                    std::memory_order_relaxed)) {
                releaseChunk = (pool.NumLiveBlocks.fetch_sub(1, std::memory_order_relaxed) == 1);

                ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateUser(sizeof(block_type)) );

                break;
            }
        }
    }

    if (Unlikely(releaseChunk))
        ReleaseFreeChunk_(ch); // cold path
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::At(index_type block) const NOEXCEPT -> block_type* {
    Assert(block < MaxSize);
    const index_type ch = (block / ChunkSize);
    const index_type id = (block - ch * ChunkSize);

#if 0 // This should not be necessary due to pool lifetime guarantee
    const auto shared(_state.LockShared());
#else
    const FInternalState_* shared = &_state.Value_NotThreadSafe();
#endif

    Assert(shared->_blocks[ch]);
    Assert_NoAssume(shared->_pools[ch].NumLiveBlocks.load(std::memory_order_relaxed) > 0);

    return (shared->_blocks[ch] + id);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::operator[](index_type block) const NOEXCEPT -> block_type* {
    Assert(block < MaxSize);
    if (block >= MaxSize)
        return nullptr;

    return At(block);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
template <typename _ForEach>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Each(_ForEach&& pred) {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    STACKLOCAL_BITSET_INIT(freeBlocks, _ChunkSize, false);

    forrange(ch, 0, MaxChunks) {
        block_type* const pBlocks = exclusive->_blocks[ch];
        if (nullptr == pBlocks)
            continue;

        const FPool_& pool = exclusive->_pools[ch];
        if (0 == pool.NumLiveBlocks.load(std::memory_order_relaxed))
            continue;

        for (   index_type id = pool.FreeList.load(std::memory_order_relaxed);
                id < ChunkSize;
                id = reinterpret_cast<const index_type&>(pBlocks[id]) ) {
            Assert_NoAssume(not freeBlocks.Get(id));
            freeBlocks.SetTrue(id);
        }

        forrange(id, 0, ChunkSize) {
            if (not freeBlocks.Unset(id))
                Meta::VariadicFunctor(pred, pBlocks + id, static_cast<index_type>(ch * ChunkSize + id));
        }

        Assert_NoAssume(freeBlocks.AllFalse());
    }
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Clear_AssertCompletelyEmpty() {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

#if USE_PPE_DEBUG
    forrange(ch, 0, MaxChunks) {
        const FPool_& pool = exclusive->_pools[ch];
        Assert(pool.NumLiveBlocks.load(std::memory_order_relaxed) == 0);
    }
#endif

    Clear_ReleaseMemory_AssumeLocked_(*exclusive);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Clear_IgnoreLeaks() {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    Clear_ReleaseMemory_AssumeLocked_(*exclusive);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
bool TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    const auto exclusive(const_cast<decltype(_state)&>(_state).LockExclusive()); ; // exclusive access is needed for safety

    index_type totalLiveChunks = 0;
    index_type totalLiveBlocks = 0;
    index_type totalFreeBlocks = 0;
    forrange(ch, 0, _MaxChunks) {
        const FPool_& pool = exclusive->_pools[ch];
        const block_type* const pBlocks = exclusive->_blocks[ch];

        if (pBlocks)
            ++totalLiveChunks;

        index_type numFreeBlocks = 0;
        for (index_type id = pool.FreeList.load(std::memory_order_relaxed); id < ChunkSize;) {
            const index_type nextId = reinterpret_cast<const index_type&>(pBlocks[id]);
            id = nextId;
            ++numFreeBlocks;
            Assert_NoAssume(numFreeBlocks <= ChunkSize); // detect loops
        }

        const index_type numLiveBlocks = pool.NumLiveBlocks.load(std::memory_order_relaxed);
        Assert_NoAssume(!pBlocks || numLiveBlocks + numFreeBlocks == ChunkSize);

        totalLiveBlocks += numLiveBlocks;
        totalFreeBlocks += numFreeBlocks;
    }

    Assert_NoAssume(totalFreeBlocks + totalLiveBlocks == totalLiveChunks * ChunkSize);
#endif

    return true;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::AllocateBlockFromNewChunk_() -> index_type {

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    // check if somebody allocated something meanwhile due to the exclusive barrier
    index_type nextChunkToAlloc = MaxChunks;
    forrange(ch, 0, MaxChunks) {
        block_type* const pBlocks = exclusive->_blocks[ch];
        if (nullptr == pBlocks) {
            nextChunkToAlloc = Min(ch, nextChunkToAlloc);
            continue;
        }

        FPool_& pool = exclusive->_pools[ch];

        if (index_type freeBlock = pool.FreeList.load(std::memory_order_relaxed); Unlikely(freeBlock < ChunkSize)) {
            // found a free block, skip chunk allocation (lock contention)
            const index_type nextId = reinterpret_cast<const index_type&>(pBlocks[freeBlock]);
            pool.FreeList.store(nextId, std::memory_order_relaxed);

            ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateUser(sizeof(block_type)) );

            Verify(pool.NumLiveBlocks.fetch_add(1, std::memory_order_relaxed) < ChunkSize);
            return static_cast<index_type>(ch * ChunkSize + freeBlock); // return global index in the pool (vs local chunk)
        }
    }

    AssertRelease(nextChunkToAlloc < MaxChunks); // OOM, no more space available in pool
    Assert(nullptr == exclusive->_blocks[nextChunkToAlloc]);

    // nothing available, need to allocate a new chunk
    auto& allocator = static_cast<_Allocator&>(*this);
    block_type* const newChunk = allocator_traits::template AllocateT<block_type>(allocator, _ChunkSize).data();

    ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateSystem(_ChunkSize * sizeof(block_type)) );
    ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateUser(sizeof(block_type)) );

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
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::ReleaseFreeChunk_(index_type ch) {
    Assert(ch < MaxChunks);

    const auto exclusive(_state.LockExclusive()); ; // exclusive access is needed for safety

    if (nullptr == exclusive->_blocks[ch])
        return;

    // check that spare chunk is still completely free
    if (exclusive->_spareChunk < MaxChunks &&
        exclusive->_pools[exclusive->_spareChunk].NumLiveBlocks.load(std::memory_order_relaxed) > 0) {
        exclusive->_spareChunk = MaxChunks; // spare chunk was used, revert bookkeeping
    }

    // hysteresis to always keep one spare chunk
    if (exclusive->_spareChunk < MaxChunks) {
        if (exclusive->_spareChunk == ch)
            return;

        if (ch > exclusive->_spareChunk)
            std::swap(ch, exclusive->_spareChunk); // keep this chunk and remove old one
    }
    else {
        exclusive->_spareChunk = ch;
        return; // early-out, let other thread consume this chunk if needed
    }

    block_type* const pBlocks = exclusive->_blocks[ch];
    AssertRelease(pBlocks);

    FPool_& pool = exclusive->_pools[ch];
    Assert_NoAssume(pool.NumLiveBlocks.load(std::memory_order_relaxed) == 0);
    Assert_NoAssume(pool.FreeList.load(std::memory_order_relaxed) < _ChunkSize);

#if USE_PPE_DEBUG
    // check that every block from the chunk are present in the free list
    index_type numFreeBlocks = 0;
    index_type blk = pool.FreeList.load(std::memory_order_relaxed);
    while (blk < ChunkSize) {
        blk = reinterpret_cast<const index_type&>(pBlocks[blk]);
        ++numFreeBlocks;
    }
    AssertRelease(ChunkSize == blk);
    AssertRelease(ChunkSize == numFreeBlocks);
#endif

    ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateSystem(_ChunkSize * sizeof(block_type)) );

    // reset free list in the pool
    pool.FreeList.store(ChunkSize, std::memory_order_relaxed);

    // release the allocated chunk
    auto& allocator = static_cast<_Allocator&>(*this);
    allocator_traits::DeallocateT(allocator, pBlocks, _ChunkSize);

    exclusive->_blocks[ch] = nullptr;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier,_Allocator>::Clear_ReleaseMemory_AssumeLocked_(FInternalState_& state) {

    // reset spare chunk
    state._spareChunk = MaxChunks;

    // reset all pools
    for (FPool_& pool : state._pools) {
        pool.NumLiveBlocks.store(0, std::memory_order_relaxed);
        pool.FreeList.store(ChunkSize, std::memory_order_relaxed);
    }

    // release all blocks
    auto& allocator = static_cast<_Allocator&>(*this);
    for (block_type*& pBlocks : state._blocks) {
        if (pBlocks) {
            ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateSystem(_ChunkSize * sizeof(block_type)) );

            allocator_traits::DeallocateT(allocator, pBlocks, _ChunkSize);
            pBlocks = nullptr;
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
class TTypedMemoryPool : public TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator> {
    using parent_type = TMemoryPool<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator>;
public:
    using typename parent_type::allocator_type;
    using typename parent_type::allocator_traits;
    using typename parent_type::index_type;
    using typename parent_type::block_type;

    using value_type = T;
    STATIC_ASSERT(sizeof(value_type) >= sizeof(typename parent_type::block_type));
    STATIC_ASSERT(alignof(typename parent_type::block_type) % alignof(value_type) == 0);

    using parent_type::BlockSize;
    using parent_type::ChunkSize;
    using parent_type::MaxChunks;
    using parent_type::MaxSize;

    using parent_type::Allocator;
    using parent_type::NumCommittedBlocks;

    using parent_type::Allocate;
    using parent_type::Deallocate;
    using parent_type::Clear_AssertCompletelyEmpty;
    using parent_type::CheckInvariants;

    TTypedMemoryPool() = default;

    explicit TTypedMemoryPool(const allocator_type& allocator) : parent_type(allocator) {}
    explicit TTypedMemoryPool(allocator_type&& rallocator) : parent_type(std::move(rallocator)) {}

    value_type* At(index_type block) const NOEXCEPT {
        return reinterpret_cast<value_type*>(parent_type::At(block));
    }
    value_type* operator [](index_type block) const NOEXCEPT {
        return reinterpret_cast<value_type*>(parent_type::operator[](block));
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
