﻿#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Allocator/SlabHeap.h"
#include "Memory/PtrRef.h"
#include "Thread/ThreadSafe_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define SLAB_ALLOCATOR(_DOMAIN) ::PPE::TSlabAllocator< ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define VECTOR_SLAB(_DOMAIN, T) ::PPE::TVector<T, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define SPARSEARRAY_SLAB(_DOMAIN, T) ::PPE::TSparseArray<T, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_SLAB(_DOMAIN, _KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR_SLAB(_DOMAIN, ::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define ASSOCIATIVE_SPARSEARRAY_SLAB(_DOMAIN, _KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, SPARSEARRAY_SLAB(_DOMAIN, ::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_SLAB(_DOMAIN, T) ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define HASHMAP_SLAB(_DOMAIN, _KEY, _VALUE) ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_SLAB(_DOMAIN) ::PPE::TMemoryStream< SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define TEXT_SLAB(_CHAR, _DOMAIN) ::PPE::TBasicText<_CHAR, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Slab allocator has an handle to a TSlabHeap
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Unknown) >
class TSlabAllocator : private FAllocatorPolicy {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::false_type;
    using has_owns = std::true_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;
#endif

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    using heap_type = TSlabHeap<_Allocator>;

    TPtrRef<heap_type> Heap;

    TSlabAllocator(heap_type& heap) NOEXCEPT
    :   Heap(heap)
    {}
    explicit TSlabAllocator(Meta::FForceInit forceInit) NOEXCEPT
    :   Heap(forceInit)
    {}

    size_t SnapSize(size_t s) const NOEXCEPT {
        return heap_type::SnapSize(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return Heap->AliasesToHeap(b.Data);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ Heap->Allocate(s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        Heap->Deallocate(b.Data, b.SizeInBytes);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = Heap->Reallocate(b.Data, s, b.SizeInBytes);
        b.SizeInBytes = s;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Heap->AliasesToHeap(b.Data));
        Unused(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Heap->AliasesToHeap(b.Data));
        Unused(b); // nothing to do
        return true;
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* TrackingData() NOEXCEPT {
        return std::addressof(Heap->TrackingData());
    }
    auto& AllocatorWithoutTracking() NOEXCEPT {
        return Heap->Allocator(); // can't remove tracking
    }
#endif

    friend bool operator ==(const TSlabAllocator& lhs, const TSlabAllocator& rhs) {
        return (lhs.Heap == rhs.Heap);
    }

    friend bool operator !=(const TSlabAllocator& lhs, const TSlabAllocator& rhs) {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename _Allocator>
TSlabAllocator(TSlabHeap<_Allocator>&) -> TSlabAllocator< Meta::TDecay<_Allocator> >;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Accessor, typename _Allocator = ALLOCATOR(Unknown)>
struct TScopedSlabAllocator : _Accessor {
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;
#endif

    using heap_type = TSlabHeap<_Allocator>;
    using heap_ref = TPtrRef<heap_type>;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    TScopedSlabAllocator() NOEXCEPT = default;

    heap_ref HeapRef() const NOEXCEPT {
        return static_cast<const _Accessor&>(*this)();
    }

    size_t SnapSize(size_t s) const NOEXCEPT {
        return heap_type::SnapSize(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        if (const heap_ref heap = HeapRef(); heap)
            return FAllocatorBlock{ heap->Allocate(s), s };
        return FAllocatorBlock{};
    }

    void Deallocate(FAllocatorBlock b) const {
        if (const heap_ref heap = HeapRef(); heap)
            heap->Deallocate(b.Data, b.SizeInBytes);
    }

    bool Reallocate(FAllocatorBlock& b, size_t s) const {
        if (const heap_ref heap = HeapRef(); heap) {
            b.Data = heap->Reallocate(b.Data, s, b.SizeInBytes);
            b.SizeInBytes = s;
            return true;
        }
        return false;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        if (const heap_ref heap = HeapRef(); heap) {
            Assert_NoAssume(heap->AliasesToHeap(b.Data));
            Unused(b); // nothing to do
            return true;
        }
        return false;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        if (const heap_ref heap = HeapRef(); heap) {
            Assert_NoAssume(heap->AliasesToHeap(b.Data));
            Unused(b); // nothing to do
            return true;
        }
        return false;
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* TrackingData() NOEXCEPT {
        if (const heap_ref heap = HeapRef(); heap)
            return std::addressof(heap->TrackingData());
        return nullptr;
    }
    auto& AllocatorWithoutTracking() NOEXCEPT {
        return *this; // can't remove tracking
    }
#endif
};
//----------------------------------------------------------------------------
template <typename _Tag, typename _Allocator = ALLOCATOR(Unknown)>
struct TThreadLocalSlabAllocator {
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;
#endif

    using heap_type = TSlabHeap<_Allocator>;
    using heap_ref = TPtrRef<heap_type>;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    TThreadLocalSlabAllocator() NOEXCEPT = default;

    size_t SnapSize(size_t s) const NOEXCEPT {
        return heap_type::SnapSize(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        if (const heap_ref heap = Mutable(); heap)
            return FAllocatorBlock{ heap->Allocate(s), s };
        return FAllocatorBlock{};
    }

    void Deallocate(FAllocatorBlock b) const {
        if (const heap_ref heap = Mutable(); heap)
            heap->Deallocate(b.Data, b.SizeInBytes);
    }

    bool Reallocate(FAllocatorBlock& b, size_t s) const {
        if (const heap_ref heap = Mutable(); heap) {
            b.Data = heap->Reallocate(b.Data, s, b.SizeInBytes);
            b.SizeInBytes = s;
            return true;
        }
        return false;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        if (const heap_ref heap = Mutable(); heap) {
            Assert_NoAssume(heap->AliasesToHeap(b.Data));
            Unused(b); // nothing to do
            return true;
        }
        return false;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        if (const heap_ref heap = Mutable(); heap) {
            Assert_NoAssume(heap->AliasesToHeap(b.Data));
            Unused(b); // nothing to do
            return true;
        }
        return false;
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking* TrackingData() NOEXCEPT {
        if (const heap_ref heap = Mutable(); heap)
            return std::addressof(heap->TrackingData());
        return nullptr;
    }
    auto& AllocatorWithoutTracking() NOEXCEPT {
        return *this; // can't remove tracking
    }
#endif

    static heap_ref& Mutable() NOEXCEPT {
        ONE_TIME_INITIALIZE_THREAD_LOCAL(heap_ref, GInstanceTLS);
        return GInstanceTLS;
    }

    struct FScope {
        heap_ref Previous;

        explicit FScope(heap_ref heap) NOEXCEPT {
            heap_ref& mut = Mutable();
            Previous = mut;
            mut = heap;
        }

        ~FScope() NOEXCEPT {
            Mutable() = Previous;
        }
    };

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
