#pragma once

#include "Allocator/SlabHeap.h"

#include "Meta/TypeInfo.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap() NOEXCEPT
#if USE_PPE_MEMORYDOMAINS
:   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#else
 = default;
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(Meta::FForceInit) NOEXCEPT
:   allocator_type(Meta::MakeForceInit<allocator_type>())
,   _slabs(static_cast<allocator_type&>(*this))
{// used for non default-constructable allocators
#if USE_PPE_MEMORYDOMAINS
    RegisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(_Allocator&& ralloc) NOEXCEPT
:   _Allocator(TAllocatorTraits<_Allocator>::SelectOnMove(std::move(ralloc)))
,   _slabs(static_cast<allocator_type&>(*this))
#if USE_PPE_MEMORYDOMAINS
,   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
#else
{
#endif
}
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(const _Allocator& alloc) NOEXCEPT
:   _Allocator(TAllocatorTraits<_Allocator>::SelectOnCopy(alloc))
,   _slabs(static_cast<allocator_type&>(*this))
#if USE_PPE_MEMORYDOMAINS
,   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
#else
{
#endif
}
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::~TSlabHeap() {
    Assert_NoAssume(CheckCanary_());
    ONLY_IF_MEMORYDOMAINS(Assert_NoAssume(_trackingData.User().NumAllocs == 0));
    ReleaseAll();
    ONLY_IF_MEMORYDOMAINS(UnregisterTrackingData(&_trackingData));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::SetSlabSize(size_t value) NOEXCEPT {
    Assert_NoAssume(CheckCanary_());
    _slabSize = checked_cast<u32>(value);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Allocate(size_t size) {
    Assert_NoAssume(CheckCanary_());
    if (size == 0)
        return nullptr;

    Assert_NoAssume(CheckInvariants());

    void* newp = nullptr;
    FBlockHeader upperBlock{};
    upperBlock.BlockSize = checked_cast<u32>(SnapSize(size));
    if (FBlockHeader* const upperBound = _freeBlocks.UpperBound(upperBlock); upperBound) {
        Assert(upperBound);
        newp = FSlab::Allocate(*this, upperBound, size);
    }

    if (newp == nullptr)
        newp = Allocate_FromNewSlab_(size);

    Assert_NoAssume(FBlockHeader::FromData(newp)->BlockSize >= size);
    Assert_NoAssume(Meta::IsAligned(ALLOCATION_BOUNDARY, newp));
    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(size));
    ONLY_IF_ASSERT(++_numLiveBlocks);
    Assert_NoAssume(CheckInvariants());
    return newp;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Reallocate(void* p, size_t newSize, size_t oldSize) {
    if (not p) {
        Assert_NoAssume(oldSize == 0);
        return Allocate(newSize);
    }
    Assert_NoAssume(oldSize > 0);
    if (newSize == 0) {
        Deallocate(p, oldSize);
        return nullptr;
    }

    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    FBlockHeader* const oldBlock = FBlockHeader::FromData(p);
    Assert_NoAssume(not oldBlock->Available);
    Assert_NoAssume(oldBlock->BlockSize >= oldSize);

    void* newp = FSlab::Reallocate(*this, oldBlock, newSize);

    if (Likely(newp)) {
        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(oldSize));
        ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(newSize));

        Assert_NoAssume(CheckInvariants());
    }
    else {
        // need to fully reallocate the block
        newp = Allocate(newSize);
        Assert(newp);

        FPlatformMemory::Memcpy(newp, p, Min(newSize, oldSize));

        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(oldSize));
        Assert_NoAssume(_numLiveBlocks > 0);
        ONLY_IF_ASSERT(--_numLiveBlocks);

        FSlab::Deallocate(*this, oldBlock);

        Assert_NoAssume(CheckInvariants());
    }

    return newp;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::Deallocate(void* p, size_t sz) {
    Unused(sz);
    if (not p) {
        Assert_NoAssume(sz == 0);
        return;
    }

    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    Assert_NoAssume(sz > 0);
    ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(sz));
    Assert_NoAssume(_numLiveBlocks > 0);
    ONLY_IF_ASSERT(--_numLiveBlocks);


    FBlockHeader* const block = FBlockHeader::FromData(p);
    Assert_NoAssume(not block->Available);
    Assert_NoAssume(block->BlockSize >= sz);

    FSlab::Deallocate(*this, block);

    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::DiscardAll() NOEXCEPT {
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());
    ONLY_IF_ASSERT(_numLiveBlocks = 0);

    _freeBlocks.Root = nullptr;

    for (FSlab& slab : _slabs) {
        const FAllocatorBlock alloc = slab.Allocation();
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(alloc.Data, alloc.SizeInBytes));

        slab.Reset(alloc);

        Assert_NoAssume(not slab.HasLiveAllocations());
        Assert_NoAssume(slab.FirstBlock->Available);
        Assert_NoAssume(not slab.FirstBlock->HasPrevBlock());
        Assert_NoAssume(not BlockFooter(slab.FirstBlock)->HasNextBlock());

        _freeBlocks.Insert(slab.FirstBlock); // refresh free blocks balanced tree
    }

    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::ReleaseAll() {
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    ONLY_IF_MEMORYDOMAINS(const FMemoryTracking::FThreadScope threadTracking{ _trackingData });
    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());
    ONLY_IF_ASSERT(_numLiveBlocks = 0);

    auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
    using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

    for (FSlab& slab : _slabs) {
        const FAllocatorBlock alloc = slab.Allocation();
        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(alloc.SizeInBytes));

        allocator_traits_without_tracking::Deallocate(allocator, alloc);
    }

    _freeBlocks.Root = nullptr;
    _slabs.clear_ReleaseMemory();

    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::TrimMemory() {
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    if (_slabs.size() < 2)
        return; // keep one slab here for hysteresis

    ONLY_IF_MEMORYDOMAINS(const FMemoryTracking::FThreadScope threadTracking{ _trackingData });

    Remove_If(_slabs, [this](FSlab& slab) -> bool {
        if (not _slabs.empty() && not slab.HasLiveAllocations()) {
            Assert_NoAssume(slab.FirstBlock->Available);
            Assert_NoAssume(not slab.FirstBlock->HasPrevBlock());
            Assert_NoAssume(not BlockFooter(slab.FirstBlock)->HasNextBlock());

            _freeBlocks.Erase(slab.FirstBlock); // only one big free block in slab if empty

            const FAllocatorBlock alloc = slab.Allocation();
            ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(alloc.SizeInBytes));

            auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
            using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

            allocator_traits_without_tracking::Deallocate(allocator, alloc);
            return true;
        }
        return false;
    });

    _slabs.shrink_to_fit();

    Assert_NoAssume(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TSlabHeap<_Allocator>::RegionSize(void* p) const NOEXCEPT {
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    FBlockHeader* const block = FBlockHeader::FromData(p);
    Assert_NoAssume(not block->Available);
    return block->BlockSize;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TSlabHeap<_Allocator>::AliasesToHeap(void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(CheckCanary_());
    Assert_NoAssume(CheckInvariants());

    for (const FSlab& slab : _slabs) {
        const FAllocatorBlock alloc = slab.Allocation();
        if (FPlatformMemory::Memaliases(alloc.Data, alloc.SizeInBytes, ptr))
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Allocate_FromNewSlab_(size_t size) {
    Assert(size);

    ONLY_IF_MEMORYDOMAINS(const FMemoryTracking::FThreadScope threadTracking{ _trackingData });

    auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
    using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

    size_t thisSlabSize = allocator_traits::SnapSize(*this, _slabSize * (1 + _slabs.size() / 2));
    if (size * 2 > thisSlabSize)
        thisSlabSize = allocator_traits::SnapSize(*this, size * 2);

    const FAllocatorBlock alloc = allocator_traits_without_tracking::Allocate(allocator, thisSlabSize);
    Assert(alloc.Data);
    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateSystem(alloc.SizeInBytes));

    FSlab& slab = _slabs.push_back_Default();
    slab.Reset(alloc);
    _freeBlocks.Insert(slab.FirstBlock);

    void* const newp = slab.Allocate(*this, slab.FirstBlock, size);
    Assert_NoAssume(newp);
    return newp;
}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
template <typename _Allocator>
bool TSlabHeap<_Allocator>::CheckInvariants() const NOEXCEPT {
#if USE_PPE_MEMORY_DEBUGGING
    size_t totalReservedSize = 0;

    u32 numBlocksAllocated = 0;
    u32 smallestBlockAllocated = UINT32_MAX;
    u32 largestBlockAllocated = 0;
    size_t totalAllocatedSize = 0;

    u32 numBlocksFree = 0;
    u32 smallestBlockFree = UINT32_MAX;
    u32 largestBlockFree = 0;
    size_t totalFreeSize = 0;

    for (const FSlab& slab : _slabs) {
        const size_t slabReservedSize = slab.Capacity;
        size_t slabAllocatedSize = 0;
        size_t slabFreeSize = 0;

        for (FBlockHeader* block = slab.FirstBlock; block; block = NextBlock(block)) {
            slabAllocatedSize += BlockOverhead;

            if (block->Available) {
                ++numBlocksFree;
                slabFreeSize += block->BlockSize;
                smallestBlockFree = Min(smallestBlockFree, checked_cast<u32>(block->BlockSize));
                largestBlockFree = Max(largestBlockFree, checked_cast<u32>(block->BlockSize));

                AssertRelease(_freeBlocks.Contains(block));
            }
            else {
                ++numBlocksAllocated;
                slabAllocatedSize += block->BlockSize;
                smallestBlockAllocated = Min(smallestBlockAllocated, checked_cast<u32>(block->BlockSize));
                largestBlockAllocated = Max(largestBlockAllocated, checked_cast<u32>(block->BlockSize));

                AssertRelease(not _freeBlocks.Contains(block));
            }
        }

        AssertRelease(slabAllocatedSize + slabFreeSize == slabReservedSize);

        totalReservedSize += slabReservedSize;
        totalAllocatedSize += slabAllocatedSize;
        totalFreeSize += slabFreeSize;
    }

    const size_t numBlocksInTree = FFreeBlockTree::CountNodes_Slow(_freeBlocks.Root);
    AssertRelease(numBlocksFree == numBlocksInTree);

    AssertRelease(totalReservedSize == totalFreeSize + totalAllocatedSize);

    Assert_NoAssume(_numLiveBlocks == numBlocksAllocated);

#endif
    return true;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::FSlab::Allocate(TSlabHeap& heap, FBlockHeader* block, size_t userSize) {
    userSize = SnapSize(userSize);

    Assert(block);
    Assert_NoAssume(block->Available);
    Assert_NoAssume(block->BlockSize >= userSize);
    Assert_NoAssume(userSize == SnapSize(userSize));

    heap._freeBlocks.Erase(block);

    SplitBlockIFP(heap, block, userSize);

    Assert_NoAssume(block->BlockSize >= userSize);
    block->Available = false;

    ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(block->Data(), block->BlockSize));
    return block->Data();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::FSlab::Reallocate(TSlabHeap& heap, FBlockHeader* block, size_t newSize) {
    newSize = SnapSize(newSize);

    Assert_NoAssume(not block->Available);
    const size_t oldSize = block->BlockSize;
    Assert_NoAssume(SnapSize(oldSize) == oldSize);

    // check if there is enough space in this block or in neighborhood
    if (block->BlockSize < newSize) {

        // check first right: extending to the right is trivial if space is available there
        if (const FBlockHeader* nextNode = NextBlock(block); nextNode && nextNode->Available && block->BlockSize + nextNode->BlockSize + BlockOverhead >= newSize) {
            block = CoalesceRight(heap, block); // retrieve free space on the right

            // split the new bigger block if necessary
            SplitBlockIFP(heap, block, newSize);

            Assert_NoAssume(not block->Available);
            Assert_NoAssume(block->BlockSize >= newSize);

            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(static_cast<u8*>(block->Data()) + oldSize, block->BlockSize - oldSize));
            return block->Data();
        }

        // can also check left, but it's less trivial: in this case extending requires memmove()
        if (const FBlockHeader* prevNode = PrevBlock(block); prevNode && prevNode->Available && block->BlockSize + prevNode->BlockSize + BlockOverhead >= newSize) {
            FBlockHeader* const newBlock = CoalesceLeft(heap, block); // retrieve free space on the left
            Assert(newBlock != block);
            FPlatformMemory::Memmove(newBlock->Data(), block->Data(), oldSize);

            // split the new bigger block if necessary
            SplitBlockIFP(heap, block, newSize);

            Assert_NoAssume(not newBlock->Available);
            Assert_NoAssume(newBlock->BlockSize >= newSize);

            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(static_cast<u8*>(newBlock->Data()) + oldSize, block->BlockSize - oldSize));
            return newBlock->Data();
        }

        return nullptr; // not enough memory: need a new memory block
    }

    if (FBlockHeader* freeBlock = SplitBlockIFP(heap, block, newSize); freeBlock) // maybe we can shrink this node?
        freeBlock = CoalesceRight(heap, freeBlock);

    return block->Data();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::FSlab::Deallocate(TSlabHeap& heap, FBlockHeader* block) {
    Assert_NoAssume(not block->Available);

    block->Available = true;

    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(block->Data(), block->BlockSize));

    block = CoalesceLeft(heap, block);
    block = CoalesceRight(heap, block);

    block = CoalesceLeft(heap, block);
    block = CoalesceRight(heap, block);

    heap._freeBlocks.Insert(block);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TSlabHeap<_Allocator>::FSlab::CoalesceLeft(TSlabHeap& heap, FBlockHeader* block) -> FBlockHeader* {
    FBlockHeader* const prev = PrevBlock(block);
    if (not prev || not prev->Available)
        return block;

    heap._freeBlocks.Erase(prev);

    FBlockFooter* const footer = BlockFooter(block);
    Assert_NoAssume(footer->Header() == block);

    prev->BlockSize += (block->BlockSize + BlockOverhead);

    Assert_NoAssume(BlockFooter(prev) == footer);
    footer->HeaderWFlags.Set(prev);

    return prev;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TSlabHeap<_Allocator>::FSlab::CoalesceRight(TSlabHeap& heap, FBlockHeader* block) -> FBlockHeader* {
    FBlockFooter* footer = BlockFooter(block);
    Assert_NoAssume(footer->Header() == block);

    FBlockHeader* const next = NextBlock(footer);
    if (not next || not next->Available)
        return block;

    heap._freeBlocks.Erase(next);

    block->BlockSize += (next->BlockSize + BlockOverhead);

    footer = BlockFooter(block);
    Assert_NoAssume(footer->Header() == next);
    footer->HeaderWFlags.Set(block);

    return block;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TSlabHeap<_Allocator>::FSlab::SplitBlockIFP(TSlabHeap& heap, FBlockHeader* block, size_t userSize) -> FBlockHeader* {
    Assert(block->BlockSize >= userSize);
    Assert_NoAssume(SnapSize(userSize) == userSize);
    Assert_NoAssume(SnapSize(block->BlockSize) == block->BlockSize || NextBlock(block) == nullptr);

    const size_t usedSize = (userSize + BlockOverhead);
    const size_t oldBlockSize = (block->BlockSize + BlockOverhead);

    if (usedSize + MinBlockSize > oldBlockSize)
        return nullptr; // not enough memory: can't split this block anymore

    FBlockFooter* const freeFooter = BlockFooter(block);
    Assert_NoAssume(freeFooter->Header() == block);

    // keep the free part of the right side of the list, this is why we always start iteration with last block
    block->BlockSize = usedSize - BlockOverhead;
    block->Available = false;

    FBlockFooter* const usedFooter = BlockFooter(block);
    usedFooter->HeaderWFlags.Reset(block, true, false);

    FBlockHeader* const freeBlock = NextBlock(usedFooter);
    freeBlock->BlockSize = ((oldBlockSize - usedSize) - BlockOverhead);
    freeBlock->Available = true;
    freeBlock->FirstBlock = false;

    Assert_NoAssume(BlockFooter(freeBlock) == freeFooter);
    Assert_NoAssume(freeFooter->Header() == block);
    freeFooter->HeaderWFlags.Set(freeBlock);

    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(freeBlock->Data(), freeBlock->BlockSize));

    heap._freeBlocks.Insert(freeBlock);

    return freeBlock;
}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
template <typename _Allocator>
bool TSlabHeap<_Allocator>::FSlab::CheckPredicates_() const {
#if USE_PPE_MEMORY_DEBUGGING
    size_t numBlocks = 0;
    size_t blockSizeInBytes = 0;
    size_t usedBlocks = 0;
    size_t usedSizeInBytes = 0;
    size_t largestFreeBlock = 0;
    size_t largestUsedBlock = 0;

    for (FBlockHeader* block = FirstBlock; block; block = NextBlock(block)) {
        Assert_NoAssume(block >= FirstBlock);

        numBlocks++;
        blockSizeInBytes += block->BlockSize;
        if (not block->Available) {
            usedBlocks++;
            usedSizeInBytes += block->BlockSize;
            largestUsedBlock = Max(largestUsedBlock, block->BlockSize);
        }
        else {
            largestFreeBlock = Max(largestFreeBlock, block->BlockSize);
        }
    }

    Unused(largestUsedBlock, largestFreeBlock);

    const size_t totalSizeAllocated = (blockSizeInBytes + numBlocks * BlockOverhead);
    if (totalSizeAllocated != Capacity)
        return false;

#endif
    return true;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
