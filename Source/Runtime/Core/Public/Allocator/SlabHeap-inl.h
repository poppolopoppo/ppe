#pragma once

#include "Allocator/SlabHeap.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap() NOEXCEPT
:   _trackingData("SlabHeap", allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(_Allocator&& ralloc) NOEXCEPT
:   _Allocator(std::move(ralloc))
,   _trackingData("SlabHeap", allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(const _Allocator& alloc) NOEXCEPT
:   _Allocator(alloc)
,   _trackingData("SlabHeap", allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::~TSlabHeap() {
    ONLY_IF_MEMORYDOMAINS(AssertRelease_NoAssume(_trackingData.User().NumAllocs == 0));
    ReleaseAll();
    ONLY_IF_MEMORYDOMAINS(UnregisterTrackingData(&_trackingData));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::SetSlabSize(size_t value) NOEXCEPT {
    _slabSize = checked_cast<u32>(allocator_traits::SnapSize(value));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
FSlabMarker TSlabHeap<_Allocator>::Tell() const noexcept {
    FSlabMarker marker;
    marker.Origin = _tell;
#if USE_PPE_MEMORYDOMAINS
    // remember allocations for Rewind()
    const auto snapshot = _trackingData.User();
    marker.Snapshot.NumAllocs = checked_cast<size_t>(snapshot.NumAllocs);
    marker.Snapshot.TotalSize = checked_cast<size_t>(snapshot.TotalSize);
#endif
    return marker;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::Rewind(const FSlabMarker& marker) noexcept {
    Assert(marker.Origin <= _tell);

#if USE_PPE_MEMORYDOMAINS
    const auto snapshot = _trackingData.User();
    _trackingData.ReleaseBatchUser(
        checked_cast<size_t>(snapshot.NumAllocs - marker.Snapshot.NumAllocs),
        checked_cast<size_t>(snapshot.TotalSize - marker.Snapshot.TotalSize) );
#endif

    reverseforeachitem(it, _slabs) {
        if (it->Origin + it->Offset >= marker.Origin) {
            ONLY_IF_ASSERT(const auto off = it->Offset);
            it->Offset = checked_cast<u32>( marker.Origin >= it->Origin ? marker.Origin - it->Origin : 0);
            ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(static_cast<u8*>(it->Ptr) + it->Offset, off - it->Offset));
        }
        else
            break;
    }

    _tell = marker.Origin;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Reallocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize) {
    Assert(oldSize);
    newSize = SnapSize(newSize);
    oldSize = SnapSize(oldSize);
    Assert(newSize < _slabSize);

    Assert(not _slabs.empty());
    FSlabPtr* const pSlab = SeekSlab_(_tell);
    Assert(pSlab);

    AssertRelease(static_cast<u8*>(pSlab->Ptr) + pSlab->Offset - oldSize == ptr);
    Assert(pSlab->Offset >= oldSize);
    Assert(_tell >= static_cast<std::streamsize>(oldSize));
    Assert_NoAssume(pSlab->Origin + pSlab->Offset == _tell);

    _tell -= oldSize;
    pSlab->Offset -= checked_cast<u32>(oldSize);
    ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(oldSize));

    if (Likely(pSlab->Offset + newSize <= pSlab->Size)) {
        _tell += newSize;
        pSlab->Offset += checked_cast<u32>(newSize);
        ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(newSize));

        return ptr; // enough space to resize the slab
    }

    // allocate a new block and copy old content
    Assert(newSize > 0);
    void* const newp = Allocate_FromNewSlab_(pSlab, newSize);
    Assert(newp);
    Assert_NoAssume(not FPlatformMemory::Memoverlap(ptr, oldSize, newp, newSize));

    FPlatformMemory::Memcpy(newp, ptr, Min(newSize, oldSize));
    return newp;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::Deallocate_AssumeLast(void* ptr, size_t size) {
    Verify(Deallocate_ReturnIfLast(ptr, size));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
bool TSlabHeap<_Allocator>::Deallocate_ReturnIfLast(void* ptr, size_t size) {
    Assert(ptr);
    Assert(size);
    size = SnapSize(size);

    Assert(not _slabs.empty());
    FSlabPtr* const pSlab = SeekSlab_(_tell);
    Assert(pSlab);
    Assert_NoAssume(pSlab->Origin + pSlab->Offset == _tell);

    if (Likely(static_cast<u8*>(pSlab->Ptr) + pSlab->Offset - size == ptr)) {
        Assert(pSlab->Offset >= size);
        Assert(_tell >= static_cast<std::streamsize>(size));
        Assert_NoAssume(_tell >= pSlab->Origin);

        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(size));

        pSlab->Offset -= checked_cast<u32>(size);
        _tell -= size;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::DiscardAll() NOEXCEPT {
    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());

    for (FSlabPtr& slab : _slabs) {
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(slab.Ptr, slab.Offset));
        slab.Offset = 0;
    }

    _tell = 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::ReleaseAll() {
    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());

    auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
    using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

    for (FSlabPtr& slab : _slabs) {
        const FAllocatorBlock blk{ slab.Ptr, slab.Size };
        allocator_traits_without_tracking::Deallocate(allocator, blk);
    }

    _slabs.clear_ReleaseMemory();
    _tell = 0;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::TrimMemory() {
    if (_slabs.size() < 2)
        return; // keep one slab here for hysteresis

    Remove_If(_slabs, [this](FSlabPtr& slab) -> bool {
        if (slab.Offset == 0) {
            Assert_NoAssume(_tell <= slab.Origin);
            ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(slab.Size));

            auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
            using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

            const FAllocatorBlock blk{ slab.Ptr, slab.Size };
            allocator_traits_without_tracking::Deallocate(allocator, blk);
            return true;
        }
        return false;
    });

    _slabs.shrink_to_fit();
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename _Allocator>
bool TSlabHeap<_Allocator>::AliasesToHeap(void* ptr) const NOEXCEPT {
    Assert(ptr);

    for (const FSlabPtr& slab : _slabs) {
        if (FPlatformMemory::Memaliases(slab.Ptr, slab.Size, ptr))
            return true;
    }

    return false;
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Allocate_FromNewSlab_(FSlabPtr* pSlab, size_t size) {
    Assert(size > 0 && size < _slabSize);
    Assert_NoAssume(SnapSize(size) == size);

    if (pSlab) {
        Assert(_slabs.AliasesToContainer(*pSlab));
        Assert_NoAssume(_tell >= pSlab->Origin);
        Assert_NoAssume(pSlab->Offset + size > pSlab->Size); // exhausted

        const size_t pos = checked_cast<size_t>(pSlab - _slabs.data());
        _tell = pSlab->Origin + pSlab->Size;
        pSlab = nullptr;

        if (pos + 1 < _slabs.size()) {
            pSlab = std::addressof(_slabs[pos + 1]);
            Assert_NoAssume(_tell == pSlab->Origin);
        }
    }

    if (nullptr == pSlab) {
        auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
        using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

        const FAllocatorBlock blk = allocator_traits_without_tracking::Allocate(allocator, _slabSize);
        AssertRelease(blk.Data);
        ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateSystem(blk.SizeInBytes));

        _slabs.emplace_back(_tell, blk.Data, 0u, checked_cast<u32>(blk.SizeInBytes));
        pSlab = std::addressof(_slabs.back());
    }

    AssertRelease(pSlab->Offset + size <= pSlab->Size);
    Assert_NoAssume(_tell == pSlab->Origin + pSlab->Offset);

    void* const result = static_cast<u8*>(pSlab->Ptr) + pSlab->Offset;
    pSlab->Offset += checked_cast<u32>(size);
    _tell += size;

    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(size));
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::DiscardAll() NOEXCEPT {
    _heap.DiscardAll();

    _spares = nullptr;
    for (void*& head : _pools)
        head = nullptr;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::ReleaseAll() {
    _heap.ReleaseAll();

    _spares = nullptr;
    for (void*& head : _pools)
        head = nullptr;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::TrimMemory() {
    // this is gonna be slow :/
    for (bool reclaim = true; reclaim; ) {
        reclaim = false;

        // first trim the pools
        forrange(pool, 0, lengthof(_pools)) {
            for (void* it = _pools[pool], *prev = nullptr; it; ) {
                void* const pNext = *static_cast<void**>(it);

                if (_heap.Deallocate_ReturnIfLast(it, FAllocatorBinning::BinSizes[pool])) {
                    reclaim = true;
                    if (prev)
                        *static_cast<void**>(prev) = pNext;
                    else
                        _pools[pool] = pNext;
                }
                else {
                    prev = it;
                }

                it = pNext;
            }
        }

        // trim spare blocks in second
        for (FSpareBlock_* it = _spares, *prev = nullptr; it; it = it->pNext) {
            FSpareBlock_* const pNext = it->pNext;

            if (_heap.Deallocate_ReturnIfLast(it, it->BlockSize)) {
                reclaim = true;
                if (prev)
                    prev->pNext = pNext;
                else
                    _spares = pNext;
            }
            else {
                prev = it;
            }
        }
    }

    _heap.TrimMemory();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TPoolingSlabHeap<_Allocator>::Allocate_SpareBlock_(size_t size) {
    Assert_NoAssume(size);
    size = SnapSize(size);

    // fall back on regular alloc if exceeds slab size
    if (Unlikely(size >= _heap.SlabSize())) {
        const FAllocatorBlock blk{ allocator_traits::Allocate(_heap.Allocator(), size) };
        return blk.Data;
    }

    // test if we got larger blocks already available
    for (FSpareBlock_* it = _spares, *prev = nullptr; it; it = it->pNext) {
        // can't split smaller than sizeof(FSpareBlock_)
        STATIC_ASSERT(sizeof(FSpareBlock_) <= ALLOCATION_BOUNDARY);

        if (it->BlockSize == size || it->BlockSize >= size + sizeof(FSpareBlock_)) {
            it->BlockSize -= size;
            void* const ptr = it->EndPtr();

            // remove spare block from list if exhausted
            if (0 == it->BlockSize) {
                if (prev)
                    prev->pNext = it->pNext;
                else
                    _spares = it->pNext;
            }

            ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(ptr, size));
            return ptr;
        }

        prev = it;
    }

    // fall back to inner slab heap
    return _heap.Allocate(size);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::Deallocate_SpareBlock_(void* ptr, size_t size) NOEXCEPT {
    Assert(ptr);
    Assert(size > FAllocatorBinning::MaxBinSize);
    size = TSlabHeap<_Allocator>::SnapSize(size);

    // fall back on regular free() if exceeds slab size
    if (Unlikely(size >= _heap.SlabSize())) {
        const FAllocatorBlock blk{ ptr, size };
        allocator_traits::Deallocate(_heap.Allocator(), blk);
        return;
    }

    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(ptr, size));

    FSpareBlock_* const pBlock = INPLACE_NEW(ptr, FSpareBlock_);
    pBlock->BlockSize = size;
    pBlock->pNext = nullptr;

    FSpareBlock_* prev = nullptr;
    for (FSpareBlock_ *it = _spares; it; it = it->pNext) {
        if (it->EndPtr() == pBlock) {
            it->BlockSize += pBlock->BlockSize;
            Assert_NoAssume(it->EndPtr() == pBlock->EndPtr());
            break;
        }
        if (pBlock->EndPtr() == it) {
            pBlock->pNext = it->pNext;
            pBlock->BlockSize += it->BlockSize;
            Assert_NoAssume(pBlock->EndPtr() == it->EndPtr());
            if (prev)
                prev->pNext = pBlock;
            else
                _spares = pBlock;
            break;
        }
        if (it > pBlock) {
            pBlock->pNext = it;
            if (prev)
                prev->pNext = pBlock;
            else
                _spares = pBlock;
            break;
        }

        Assert(it != pBlock);
        prev = it;
    }

    if (prev)
        prev->pNext = pBlock;
    else
        _spares = pBlock;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
