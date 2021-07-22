#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/SlabHeap.h"
#include "Memory/PtrRef.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ARRAY_SLAB(T) ::PPE::TArray<T, ::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
#define VECTOR_SLAB(T) ::PPE::TVector<T, ::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
#define SPARSEARRAY_SLAB(T) ::PPE::TSparseArray<T, ::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_SLAB(_KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR_SLAB(::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_SLAB(T) ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, ::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
#define HASHMAP_SLAB(_KEY, _VALUE) ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
#define MEMORYSTREAM_SLAB() ::PPE::TMemoryStream<::PPE::FSlabAllocator>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Slab allocator have an handle to a FPoolingSlabHeap
//----------------------------------------------------------------------------
class PPE_CORE_API FSlabAllocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;
#endif

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    TPtrRef<FPoolingSlabHeap> Heap;

    FSlabAllocator(FPoolingSlabHeap& heap) NOEXCEPT
    :   Heap(heap)
    {}
    explicit FSlabAllocator(Meta::FForceInit forceInit) NOEXCEPT
    :   Heap(forceInit)
    {}

    static size_t SnapSize(size_t s) NOEXCEPT {
        return FPoolingSlabHeap::SnapSize(s);
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
        UNUSED(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(Heap->AliasesToHeap(b.Data));
        UNUSED(b); // nothing to do
        return true;
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT {
        return Heap->TrackingData();
    }
#endif

    friend bool operator ==(const FSlabAllocator& lhs, const FSlabAllocator& rhs) {
        return (lhs.Heap == rhs.Heap);
    }

    friend bool operator !=(const FSlabAllocator& lhs, const FSlabAllocator& rhs) {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
