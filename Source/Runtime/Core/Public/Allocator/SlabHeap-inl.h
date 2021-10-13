#pragma once

#include "Allocator/SlabHeap.h"

#include "Meta/TypeInfo.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap() NOEXCEPT
:   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(Meta::FForceInit) NOEXCEPT
:   allocator_type(Meta::MakeForceInit<allocator_type>()) {// used for non default-constructible allocators
#if USE_PPE_MEMORYDOMAINS
    RegisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(_Allocator&& ralloc) NOEXCEPT
:   _Allocator(TAllocatorTraits<_Allocator>::SelectOnMove(std::move(ralloc)))
,   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
TSlabHeap<_Allocator>::TSlabHeap(const _Allocator& alloc) NOEXCEPT
:   _Allocator(TAllocatorTraits<_Allocator>::SelectOnCopy(alloc))
,   _trackingData(Meta::type_info<TSlabHeap>.name, allocator_traits::TrackingData(*this)) {
    RegisterTrackingData(&_trackingData);
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
TSlabHeap<_Allocator>::~TSlabHeap() {
    Assert_NoAssume(CheckCanary_());
    ONLY_IF_MEMORYDOMAINS(AssertRelease_NoAssume(_trackingData.User().NumAllocs == 0));
    ReleaseAll();
    ONLY_IF_MEMORYDOMAINS(UnregisterTrackingData(&_trackingData));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::SetSlabSize(size_t value) NOEXCEPT {
    Assert_NoAssume(CheckCanary_());
    _slabSize = checked_cast<u32>(allocator_traits::SnapSize(*this, value));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Reallocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize) {
    Assert(oldSize | newSize);
    Assert_NoAssume(CheckCanary_());

    FSlabPtr* pSlab = nullptr;

    if (Likely(oldSize > 0)) {
        oldSize = SnapSize(oldSize);
        pSlab = _slabs.MakeView().Any([ptr](FSlabPtr& slab) -> bool {
            return FPlatformMemory::Memaliases(slab.Ptr, slab.Size, ptr);
        });

        AssertRelease(pSlab);
        Assert_NoAssume(FPlatformMemory::Memaliases(pSlab->Ptr, pSlab->Size, static_cast<u8*>(ptr) + oldSize - 1));
        Assert(static_cast<u8*>(pSlab->Ptr) + pSlab->Offset - oldSize == ptr);
        Assert(pSlab->Offset >= oldSize);

        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(oldSize));
    }

    if (Likely(newSize > 0)) {
        ONLY_IF_ASSERT(++_numLiveBlocks);

        newSize = SnapSize(newSize);
        if (pSlab && (pSlab->Offset - oldSize) + newSize <= pSlab->Size) {
            ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(newSize));
            pSlab->Offset = checked_cast<u32>((pSlab->Offset - oldSize) + newSize);
            return ptr;
        }

        void* const newP = Allocate(newSize);
        FPlatformMemory::Memcpy(newP, ptr, Min(newSize, oldSize));

        ONLY_IF_ASSERT(if (ptr) FPlatformMemory::Memdeadbeef(ptr, oldSize));
        ptr = newP;
    }

    if (pSlab) {
        pSlab->Offset -= checked_cast<u32>(oldSize);
    }
    return ptr;
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
    Assert_NoAssume(CheckCanary_());
    size = SnapSize(size);

    Assert(not _slabs.empty());

    foreachitem(it, _slabs) {
        FSlabPtr& slab = *it;
        if (FPlatformMemory::Memaliases(slab.Ptr, slab.Size, ptr)) {
            Assert_NoAssume(FPlatformMemory::Memaliases(slab.Ptr, slab.Size, static_cast<u8*>(ptr) + size - 1));
            if (Likely(static_cast<u8*>(slab.Ptr) + slab.Offset - size == ptr)) {
                Assert(slab.Offset >= size);
                slab.Offset -= checked_cast<u32>(size);

                ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(ptr, size));
                ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(size));

                // fold non-standalone empty slabs if possible
                if (Unlikely(!slab.Standalone & !slab.Offset)) {
                    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(slab.Size));
                    if (Deallocate_ReturnIfLast(slab.Ptr, slab.Size)) {
                        _slabs.erase_DontPreserveOrder(it);
                    }
                    else {
                        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(slab.Size));
                    }
                }

                return true;
            }
        }
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::DiscardAll() NOEXCEPT {
    Assert_NoAssume(CheckCanary_());
    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());

    Remove_If(_slabs, [](FSlabPtr& slab) -> bool {
        if (Likely(slab.Standalone)) {
            ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(slab.Ptr, slab.Offset));

            slab.Offset = 0;
            return false;
        }
        return true; // remove non-standalone slabs
    });
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::ReleaseAll() {
    Assert_NoAssume(CheckCanary_());
    ONLY_IF_MEMORYDOMAINS(_trackingData.ReleaseAllUser());

    auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
    using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

    for (FSlabPtr& slab : _slabs) {
        if (Likely(slab.Standalone)) {
            ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(slab.Size));

            const FAllocatorBlock blk{ slab.Ptr, slab.Size };
            allocator_traits_without_tracking::Deallocate(allocator, blk);
        }
    }

    _slabs.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::TrimMemory() {
    Assert_NoAssume(CheckCanary_());
    if (_slabs.size() < 2)
        return; // keep one slab here for hysteresis

    Remove_If(_slabs, [this](FSlabPtr& slab) -> bool {
        if (slab.Offset == 0) {
            if (Likely(slab.Standalone)) {
                ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(slab.Size));

                auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
                using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

                const FAllocatorBlock blk{ slab.Ptr, slab.Size };
                allocator_traits_without_tracking::Deallocate(allocator, blk);
                return true;
            }
            else { // this slab was reclaimed from a user allocation:
                ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(slab.Size));
                if (Deallocate_ReturnIfLast(slab.Ptr, slab.Size)) {
                    return true;
                }
                ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(slab.Size));
                return false;
            }
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
    Assert_NoAssume(CheckCanary_());

    for (const FSlabPtr& slab : _slabs) {
        if (FPlatformMemory::Memaliases(slab.Ptr, slab.Size, ptr))
            return true;
    }

    return false;
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TSlabHeap<_Allocator>::Allocate_FromNewSlab_(size_t size) {
    Assert(size);
    Assert_NoAssume(SnapSize(size) == size);

    auto& allocator = allocator_traits::AllocatorWithoutTracking(*this);
    using allocator_traits_without_tracking = TAllocatorTraits<Meta::TDecay<decltype(allocator)>>;

    size_t thisSlabSize = allocator_traits::SnapSize(*this, _slabSize * (1 + _slabs.size() / 2));
    if (size * 2 > thisSlabSize)
        thisSlabSize = allocator_traits::SnapSize(*this, size * 2);

    const FAllocatorBlock blk = allocator_traits_without_tracking::Allocate(allocator, thisSlabSize);

    AssertRelease(blk.Data);
    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateSystem(blk.SizeInBytes));

    FSlabPtr slab{ blk.Data, 0u, checked_cast<u32>(blk.SizeInBytes), true/* need to deallocate */ };
    AssertRelease(slab.Offset + size <= slab.Size);

    void* const result = static_cast<u8*>(slab.Ptr) + slab.Offset;
    slab.Offset += checked_cast<u32>(size);
    ONLY_IF_MEMORYDOMAINS(_trackingData.AllocateUser(size));
    ONLY_IF_ASSERT(++_numLiveBlocks);

    _slabs.push_back(std::move(slab));
    return result;
}
//---------------------------------------------------------------------------
template <typename _Allocator>
void TSlabHeap<_Allocator>::ReclaimUserBlock_AssumeTracked_(void* ptr, size_t size) {
    Assert(ptr);
    Assert(size > 2 * sizeof(FSlabPtr)); // sanity check
    Assert_NoAssume(AliasesToHeap(ptr));

    ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateUser(size));
    _slabs.push_back(FSlabPtr{ ptr, 0, checked_cast<u32>(size), false/* non-standalone */ });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
void* TPoolingSlabHeap<_Allocator>::Reallocate(void* ptr, size_t newSize, size_t oldSize) {
    Assert_NoAssume(_heap.CheckCanary_());
    if (!!oldSize && !!newSize) {
        newSize = SnapSize(newSize);
        oldSize = SnapSize(oldSize);

        if (newSize != oldSize) {
            // don't use PoolXXX() functions here: not guaranteed to fit in pool
            void* const newp = Allocate(newSize);
            FPlatformMemory::Memcpy(newp, ptr, Min(newSize, oldSize));
            Deallocate(ptr, oldSize);
            ptr = newp;
        }

        return ptr;
    }
    else if (oldSize) {
        Assert_NoAssume(0 == newSize);
        Deallocate(ptr, oldSize);
        return nullptr;
    }
    else {
        Assert(newSize);
        Assert(nullptr == ptr);
        return Allocate(newSize);
    }
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::DiscardAll() NOEXCEPT {
    _heap.DiscardAll();

    Broadcast(_pools.MakeView(), nullptr);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::ReleaseAll() {
    _heap.ReleaseAll();

    Broadcast(_pools.MakeView(), nullptr);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TPoolingSlabHeap<_Allocator>::TrimMemory() {
    Assert_NoAssume(_heap.CheckCanary_());
    // this is gonna be slow :/
    for (bool reclaim = true; reclaim; ) {
        reclaim = false;

        // trim the pools
        forrange(pool, 0, lengthof(_pools)) {
            for (void* it = _pools[pool], *prev = nullptr; it; ) {
                void* const pNext = *static_cast<void**>(it);

                ONLY_IF_MEMORYDOMAINS(_heap.TrackingData().AllocateUser(FAllocatorBinning::BinSizes[pool]));

                if (_heap.Deallocate_ReturnIfLast(it, FAllocatorBinning::BinSizes[pool])) {
                    reclaim = true;
                    if (prev)
                        *static_cast<void**>(prev) = pNext;
                    else
                        _pools[pool] = pNext;
                }
                else {
                    ONLY_IF_MEMORYDOMAINS(_heap.TrackingData().DeallocateUser(FAllocatorBinning::BinSizes[pool]));
                    prev = it;
                }

                it = pNext;
            }
        }
    }

    _heap.TrimMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
