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
        allocator_type a = Default;
        return allocator_traits::MaxSize(a);
    }

    NODISCARD static size_t SnapSize(size_t s) NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits::SnapSize(a, s);
    }

    template <typename T>
    NODISCARD static size_t SnapSizeT(size_t s) NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits::template SnapSizeT<T>(a, s);
    }

    NODISCARD static bool Owns(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Owns(Default, b);
    }

    NODISCARD static FAllocatorBlock Allocate(size_t s) {
        allocator_type a = Default;
        return allocator_traits::Allocate(a, s);
    }

    template <typename T>
    NODISCARD static TMemoryView<T> AllocateT(size_t n) {
        allocator_type a = Default;
        return allocator_traits::template AllocateT<T>(a, n);
    }

    template <typename T>
    NODISCARD static T* AllocateOneT() {
        allocator_type a = Default;
        return allocator_traits::template AllocateOneT<T>(a);
    }

    static void Deallocate(FAllocatorBlock b) {
        allocator_type a = Default;
        return allocator_traits::Deallocate(a, b);
    }

    template <typename T>
    static void DeallocateT(TMemoryView<T> v) {
        allocator_type a = Default;
        return allocator_traits::template DeallocateT<T>(a, v);
    }

    template <typename T>
    static void DeallocateT(T* p, size_t n) {
        allocator_type a = Default;
        return allocator_traits::template DeallocateT<T>(a, p, n);
    }

    template <typename T>
    static void DeallocateOneT(T* p) {
        allocator_type a = Default;
        return allocator_traits::template DeallocateOneT<T>(a, p);
    }

    NODISCARD static auto Reallocate(FAllocatorBlock& b, size_t s) {
        allocator_type a = Default;
        return allocator_traits::Reallocate(a, b, s);
    }

    // specialized this method to avoid over-copying when !has_reallocate
    template <typename T>
    NODISCARD static auto ReallocateT_AssumePOD(TMemoryView<T>& items, size_t oldSize, size_t newSize) {
        allocator_type a = Default;
        return allocator_traits::template ReallocateT_AssumePOD<T>(
            a, items, oldSize, newSize );
    }

    NODISCARD static bool Acquire(FAllocatorBlock b) NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits::Acquire(a, b);
    }

    NODISCARD static bool Steal(FAllocatorBlock b) NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits::Steal(a, b);
    }

#if USE_PPE_MEMORYDOMAINS
    NODISCARD static FMemoryTracking& TrackingData() NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits::TrackingData(a);
    }

    NODISCARD static auto& AllocatorWithoutTracking() NOEXCEPT {
        allocator_type a = Default;
        return allocator_traits:: AllocatorWithoutTracking(a);
    }
#endif

    using allocator_traits::StealAndAcquire;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
