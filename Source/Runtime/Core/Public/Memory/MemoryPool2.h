#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/Array.h"
#include "Container/BitSet.h"
#include "Container/BitTree.h"
#include "Memory/MemoryView.h"
#include "Meta/Functor.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
class TMemoryPool2 : Meta::FNonCopyableNorMovable, _Allocator {
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

    STATIC_CONST_INTEGRAL(index_type, MaxSize, static_cast<index_type>(_ChunkSize * _MaxChunks));
    STATIC_CONST_INTEGRAL(index_type, BlockSize, static_cast<index_type>(_BlockSize));
    STATIC_CONST_INTEGRAL(index_type, ChunkSize, static_cast<index_type>(_ChunkSize));
    STATIC_CONST_INTEGRAL(index_type, MaxChunks, static_cast<index_type>(_MaxChunks));

    TMemoryPool2() { InitializeInternalPool_(*_pool.LockExclusive()); }
    explicit TMemoryPool2(const allocator_type& allocator) : allocator_type(allocator) { InitializeInternalPool_(*_pool.LockExclusive()); }
    explicit TMemoryPool2(allocator_type&& rallocator) : allocator_type(std::move(rallocator)) { InitializeInternalPool_(*_pool.LockExclusive());}

    ~TMemoryPool2();

    index_type NumFreeBlocks() const NOEXCEPT;
    index_type NumLiveBlocks() const NOEXCEPT;

    NODISCARD index_type Allocate();
    void Deallocate(index_type block);

    block_type* At(index_type block) const NOEXCEPT;
    block_type* operator [](index_type block) const NOEXCEPT { return At(block); }

    template <typename _ForEach>
    void Each(_ForEach&& pred);

    void Clear_AssertCompletelyEmpty();
    void Clear_IgnoreLeaks();

    bool CheckInvariants() const;

private:
    using word_type =
        std::conditional_t<Meta::IsPowOf<64>(ChunkSize), u64,
        std::conditional_t<Meta::IsPowOf<32>(ChunkSize), u32,
        void/* error */ >>;

    using FFreeBlocks_ = TBitTree<word_type, false>;

    struct FInternalPool_ {
        FFreeBlocks_ FreeBlocks;
        block_type** pChunks{ nullptr };
        index_type* pNumLiveBlocks{ nullptr };
        index_type SpareChunk{ UMax };

        TMemoryView<block_type*> Chunks() const { return { pChunks, ChunkSize}; }
        TMemoryView<index_type> NumLiveBlocks() const { return { pNumLiveBlocks, ChunkSize}; }
    };

    void InitializeInternalPool_(FInternalPool_& pool);
    NODISCARD NO_INLINE index_type AllocateFromNewChunk_(FInternalPool_& pool);
    NO_INLINE void ReleaseFreeChunk_(FInternalPool_& pool, index_type ch);
    FORCE_INLINE void Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool);

    TThreadSafe<FInternalPool_, _Barrier> _pool;
};
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::~TMemoryPool2() {
    const auto exclusivePool = _pool.LockExclusive();
    Clear_ReleaseMemory_AssumeLocked_(*exclusivePool);

    size_t totalSizeInBytes = 0;
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(exclusivePool->FreeBlocks.AllocationSize());
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(block_type*) * MaxChunks);
    totalSizeInBytes += (sizeof(index_type) * MaxChunks);

    FAllocatorBlock blk{ exclusivePool->FreeBlocks.Bits, totalSizeInBytes };
    allocator_traits::Deallocate(*this, blk);

#if USE_PPE_ASSERT
    exclusivePool->SpareChunk = UMax;
    exclusivePool->FreeBlocks = Default;
    exclusivePool->pChunks = nullptr;
    exclusivePool->pNumLiveBlocks = nullptr;
#endif
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::NumFreeBlocks() const NOEXCEPT -> index_type {
    const auto sharedPool = _pool.LockShared();
    Assert(sharedPool->FreeBlocks.Bits);

    const index_type numUnavailable = checked_cast<index_type>(sharedPool->FreeBlocks.CountOnes(sharedPool->FreeBlocks.DesiredSize));
    Assert(numUnavailable <= MaxSize);

    return (MaxSize - numUnavailable);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::NumLiveBlocks() const NOEXCEPT -> index_type {
    const auto sharedPool = _pool.LockShared();

    return sharedPool->NumLiveBlocks().Sum();
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Allocate() -> index_type {
    const auto exclusivePool = _pool.LockExclusive();
    Assert(exclusivePool->FreeBlocks.Bits);

    const auto block = exclusivePool->FreeBlocks.Allocate();
    if (Likely(UMax != block)) {
        Assert(block < MaxSize);
        const auto ch = (block / ChunkSize);
        Assert(ch < ChunkSize);
        ++exclusivePool->pNumLiveBlocks[ch];

        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(exclusivePool->Chunks()[ch] + block % ChunkSize, sizeof(block_type)));
        ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateUser(sizeof(block_type)) );

        return checked_cast<index_type>(block);
    }

    return AllocateFromNewChunk_(*exclusivePool);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Deallocate(index_type block) {
    Assert(block < MaxSize);

    const index_type ch = (block / ChunkSize);
    const auto exclusivePool = _pool.LockExclusive();
    Assert(exclusivePool->FreeBlocks.Bits);
    Assert(exclusivePool->pNumLiveBlocks[ch] > 0);

    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(exclusivePool->Chunks()[ch] + block % ChunkSize, sizeof(block_type)));
    ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateUser(sizeof(block_type)) );

    exclusivePool->FreeBlocks.Deallocate(block);

    if (--exclusivePool->pNumLiveBlocks[ch] == 0)
        ReleaseFreeChunk_(*exclusivePool, ch);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_AssertCompletelyEmpty() {
    const auto exclusivePool = _pool.LockExclusive();

    Assert_NoAssume(exclusivePool->NumLiveBlocks().Sum() == 0);

    Clear_ReleaseMemory_AssumeLocked_(*exclusivePool);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_IgnoreLeaks() {
    const auto exclusivePool = _pool.LockExclusive();

    Broadcast(exclusivePool->NumLiveBlocks(), Zero);

    Clear_ReleaseMemory_AssumeLocked_(*exclusivePool);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::At(index_type block) const NOEXCEPT -> block_type* {
    const auto sharedPool = _pool.LockShared();
    const auto chunk = (block / ChunkSize);

    Assert(sharedPool->Chunks()[chunk]);
    return (sharedPool->Chunks()[chunk] + (block % ChunkSize));
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
template <typename _ForEach>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Each(_ForEach&& pred) {
    const auto exclusivePool = _pool.LockExclusive();

    using word_t = typename FFreeBlocks_::word_t;
    const auto leaves = exclusivePool->FreeBlocks.Leaves();

    forrange(ch, 0, MaxChunks) {
        ONLY_IF_ASSERT(index_type numLiveBlocks_forAssert = 0);
        if (block_type* const pBlocks = exclusivePool->Chunks()[ch]) {
            forrange(w, ch * ChunkSize / FFreeBlocks_::WordBitCount, ChunkSize / FFreeBlocks_::WordBitCount) {
                for (word_t m{ leaves[w] }; m; ) {
                    const auto rel = (w * FFreeBlocks_::WordBitCount + m.PopFront_AssumeNotEmpty());
                    Meta::VariadicFunctor(pred, pBlocks + rel, ch * ChunkSize + rel);
                    ONLY_IF_ASSERT(++numLiveBlocks_forAssert);
                }
            }
        }
        Assert_NoAssume(exclusivePool->NumLiveBlocks()[ch] == numLiveBlocks_forAssert);
    }
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
bool TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::CheckInvariants() const {
#if USE_PPE_DEBUG
    const auto sharedPool = _pool.LockShared();

    using mask_t = typename FFreeBlocks_::mask_t;
    const auto leaves = sharedPool->FreeBlocks.Leaves();

    STATIC_ASSERT(Meta::IsAligned(FFreeBlocks_::WordBitCount, ChunkSize));
    const auto wordsPerChunk = (ChunkSize / FFreeBlocks_::WordBitCount);

    forrange(ch, 0, MaxChunks) {
        const auto wordOffset = ch * wordsPerChunk;
        size_t numLiveBlocks_forAssert = 0;
        if (sharedPool->Chunks()[ch]) {
            forrange(w, wordOffset, wordOffset + wordsPerChunk) {
                const mask_t m{ leaves[w] };
                numLiveBlocks_forAssert += m.Count();
            }
        }
        else {
            forrange(w, wordOffset, wordOffset + wordsPerChunk) {
                const mask_t m{ leaves[w] };
                Assert_NoAssume(m.AllTrue()); // unallocated chunks are full
            }
        }
        Assert_NoAssume(sharedPool->NumLiveBlocks()[ch] == checked_cast<index_type>(numLiveBlocks_forAssert));
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
auto TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::AllocateFromNewChunk_(FInternalPool_& pool) -> index_type {
    forrange(ch, 0, MaxChunks) {
        if (not pool.Chunks()[ch]) {
            Assert_NoAssume(0 == pool.NumLiveBlocks()[ch]);

            const FAllocatorBlock blk = allocator_traits::Allocate(*this, ChunkSize * _BlockSize );

            ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateSystem(sizeof(block_type) * ChunkSize) );

            pool.pChunks[ch] = static_cast<block_type*>(blk.Data);
            pool.FreeBlocks.DeallocateRange(ch * ChunkSize, ch * ChunkSize + ChunkSize);

            const index_type firstFree = (ch * ChunkSize);
            pool.FreeBlocks.AllocateBit(firstFree);
            pool.pNumLiveBlocks[ch] = 1;

            ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).AllocateUser(sizeof(block_type)) );
            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(pool.pChunks[ch], sizeof(block_type)));

            return firstFree;
        }
    }

    AssertReleaseFailed(L"OOM in pool, no more chunk to allocate");
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::ReleaseFreeChunk_(FInternalPool_& pool, index_type ch) {
    Assert(ch < _MaxChunks);
    Assert(pool.pChunks[ch]);
    Assert(0 == pool.pNumLiveBlocks[ch]);

    std::swap(ch, pool.SpareChunk); // deallocation hysteresis: always keep one spare chunk

    if (ch != UMax && 0 == pool.pNumLiveBlocks[ch] && pool.pChunks[ch]) {
        const u32 first = checked_cast<u32>(ch * ChunkSize);
        pool.FreeBlocks.AllocateRange(first, first + ChunkSize);

        ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateSystem(sizeof(block_type) * ChunkSize) );

        FAllocatorBlock blk{ pool.pChunks[ch], ChunkSize * _BlockSize };
        allocator_traits::Deallocate(*this, blk);

        pool.pChunks[ch] = nullptr;
    }
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::Clear_ReleaseMemory_AssumeLocked_(FInternalPool_& pool) {

    forrange(ch, 0, MaxChunks) {
        if (block_type* const pBlocks = pool.pChunks[ch]) {
            ONLY_IF_MEMORYDOMAINS( MEMORYDOMAIN_TRACKING_DATA(MemoryPool).DeallocateSystem(sizeof(block_type) * ChunkSize) );

            FAllocatorBlock blk{ pBlocks, ChunkSize * _BlockSize };
            allocator_traits::Deallocate(*this, blk);

            pool.pChunks[ch] = nullptr;
            pool.pNumLiveBlocks[ch] = 0;
        }
    }

    pool.SpareChunk = UMax;
    pool.FreeBlocks.Initialize(pool.FreeBlocks.Bits, true);
}
//----------------------------------------------------------------------------
template <size_t _BlockSize, size_t _Align, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
void TMemoryPool2<_BlockSize, _Align, _ChunkSize, _MaxChunks, _Barrier, _Allocator>::InitializeInternalPool_(FInternalPool_& pool) {
    Assert_NoAssume(UMax == pool.SpareChunk);
    Assert_NoAssume(nullptr == pool.FreeBlocks.Bits);
    Assert_NoAssume(nullptr == pool.pChunks);
    Assert_NoAssume(nullptr == pool.pNumLiveBlocks);

    pool.FreeBlocks.SetupMemoryRequirements(MaxSize);

    size_t totalSizeInBytes = 0;
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(pool.FreeBlocks.AllocationSize());
    Assert(Meta::IsAligned(sizeof(block_type**), totalSizeInBytes));
    const size_t chunksOffsetInBlocks = (totalSizeInBytes / sizeof(block_type*));
    totalSizeInBytes += ROUND_TO_NEXT_CACHELINE(sizeof(block_type*) * MaxChunks);
    Assert(Meta::IsAligned(sizeof(index_type), totalSizeInBytes));
    const size_t numLiveBlockOffsetInWords = (totalSizeInBytes / sizeof(index_type));
    totalSizeInBytes += (sizeof(index_type) * MaxChunks);

    const FAllocatorBlock blk = allocator_traits::Allocate(*this, totalSizeInBytes);
    pool.FreeBlocks.Initialize(static_cast<typename FFreeBlocks_::word_t*>(blk.Data), true);
    pool.pChunks = static_cast<block_type**>(blk.Data) + chunksOffsetInBlocks;
    pool.pNumLiveBlocks = static_cast<index_type*>(blk.Data) + numLiveBlockOffsetInWords;

    Broadcast(pool.Chunks(), nullptr);
    Broadcast(pool.NumLiveBlocks(), Zero);
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks, EThreadBarrier _Barrier, typename _Allocator>
class TTypedMemoryPool2 : public TMemoryPool2<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator> {
    using parent_type = TMemoryPool2<sizeof(T), alignof(T), _ChunkSize, _MaxChunks, _Barrier,_Allocator>;
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

    TTypedMemoryPool2() = default;

    explicit TTypedMemoryPool2(const allocator_type& allocator) : parent_type(allocator) {}
    explicit TTypedMemoryPool2(allocator_type&& rallocator) : parent_type(std::move(rallocator)) {}

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
