#pragma once

// simple wrapper for state-less allocator + allocator traits

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
struct TStaticAllocator : TAllocatorTraits<_Allocator> {
    STATIC_ASSERT(TAllocatorTraits<_Allocator>::is_always_equal::value); // must be state-less

    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using typename allocator_traits::propagate_on_container_copy_assignment;
    using typename allocator_traits::propagate_on_container_move_assignment;
    using typename allocator_traits::propagate_on_container_swap;

    using typename allocator_traits::is_always_equal;

    using typename allocator_traits::has_maxsize;
    using typename allocator_traits::has_owns;
    using typename allocator_traits::has_reallocate;
    using typename allocator_traits::has_acquire;
    using typename allocator_traits::has_steal;
#if USE_PPE_MEMORYDOMAINS
    using typename allocator_traits::has_memory_tracking;
#endif

    using allocator_traits::Alignment;

    using typename allocator_traits::reallocate_can_fail;

    NODISCARD static size_t MaxSize() NOEXCEPT {
        return allocator_traits::MaxSize(Default);
    }

    NODISCARD static size_t SnapSize(size_t s) NOEXCEPT {
        return allocator_traits::SnapSize(Default, s);
    }

    template <typename T>
    NODISCARD static size_t SnapSizeT(size_t s) NOEXCEPT {
        return allocator_traits::template SnapSizeT<T>(Default, s);
    }

    NODISCARD static bool Owns(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Owns(Default, b);
    }

    NODISCARD static FAllocatorBlock Allocate(size_t s) {
        return allocator_traits::Allocate(Default, s);
    }

    template <typename T>
    NODISCARD static TMemoryView<T> AllocateT(size_t n) {
        return allocator_traits::template AllocateT<T>(Default, n);
    }

    template <typename T>
    NODISCARD static T* AllocateOneT() {
        return allocator_traits::template AllocateOneT<T>(Default);
    }

    static void Deallocate(FAllocatorBlock b) {
        return allocator_traits::Deallocate(Default, b);
    }

    template <typename T>
    static void DeallocateT(TMemoryView<T> v) {
        return allocator_traits::template DeallocateT<T>(Default, v);
    }

    template <typename T>
    static void DeallocateT(T* p, size_t n) {
        return allocator_traits::template DeallocateT<T>(Default, p, n);
    }

    template <typename T>
    static void DeallocateOneT(T* p) {
        return allocator_traits::template DeallocateOneT<T>(Default, p);
    }

    NODISCARD static auto Reallocate(FAllocatorBlock& b, size_t s) {
        return allocator_traits::Reallocate(Default, b, s);
    }

    // specialized this method to avoid over-copying when !has_reallocate
    template <typename T>
    NODISCARD static auto ReallocateT_AssumePOD(TMemoryView<T>& items, size_t oldSize, size_t newSize) {
        return allocator_traits::template ReallocateT_AssumePOD<T>(Default, items, oldSize, newSize);
    }

    NODISCARD static bool Acquire(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Acquire(Default, b);
    }

    NODISCARD static bool Steal(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Steal(Default, b);
    }

#if USE_PPE_MEMORYDOMAINS
    NODISCARD static FMemoryTracking& TrackingData() NOEXCEPT {
        return allocator_traits::TrackingData(Default);
    }

    NODISCARD static auto& AllocatorWithoutTracking() NOEXCEPT {
        return allocator_traits:: AllocatorWithoutTracking(Default);
    }
#endif

    using allocator_traits::StealAndAcquire;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
