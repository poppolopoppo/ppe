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

    using allocator_traits::MaxSize;
    using allocator_traits::SnapSize;
    using allocator_traits::SnapSizeT;

    static bool Owns(FAllocatorBlock b) NOEXCEPT {
        allocator_type alloc;
        return allocator_traits::Owns(alloc, b);
    }

    static FAllocatorBlock Allocate(size_t s) {
        allocator_type alloc;
        return allocator_traits::Allocate(alloc, s);
    }

    template <typename T>
    static TMemoryView<T> AllocateT(size_t n) {
        allocator_type alloc;
        return allocator_traits::template AllocateT<T>(alloc, n);
    }

    template <typename T>
    static T* AllocateOneT() {
        allocator_type alloc;
        return allocator_traits::template AllocateOneT<T>(alloc);
    }

    static void Deallocate(FAllocatorBlock b) {
        allocator_type alloc;
        return allocator_traits::Deallocate(alloc, b);
    }

    template <typename T>
    static void DeallocateT(TMemoryView<T> v) {
        allocator_type alloc;
        return allocator_traits::template DeallocateT<T>(alloc, v);
    }

    template <typename T>
    static void DeallocateT(T* p, size_t n) {
        allocator_type alloc;
        return allocator_traits::template DeallocateT<T>(alloc, p, n);
    }

    template <typename T>
    static void DeallocateOneT(T* p) {
        allocator_type alloc;
        return allocator_traits::template DeallocateOneT<T>(alloc, p);
    }

    static auto Reallocate(FAllocatorBlock& b, size_t s) {
        allocator_type alloc;
        return allocator_traits::Reallocate(alloc, b, s);
    }

    // specialized this method to avoid over-copying when !has_reallocate
    template <typename T>
    static auto ReallocateT_AssumePOD(TMemoryView<T>& items, size_t oldSize, size_t newSize) {
        allocator_type alloc;
        return allocator_traits::template ReallocateT_AssumePOD<T>(alloc, items, oldSize, newSize);
    }

    static bool Acquire(FAllocatorBlock b) NOEXCEPT {
        allocator_type alloc;
        return allocator_traits::Acquire(alloc, b);
    }

    static bool Steal(FAllocatorBlock b) NOEXCEPT {
        allocator_type alloc;
        return allocator_traits::Steal(alloc, b);
    }

#if USE_PPE_MEMORYDOMAINS
    static FMemoryTracking& TrackingData() NOEXCEPT {
        allocator_type alloc;
        return allocator_traits::TrackingData(alloc);
    }
#endif

    using allocator_traits::StealAndAcquire;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
