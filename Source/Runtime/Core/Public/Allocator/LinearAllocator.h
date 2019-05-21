#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/LinearHeap.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define VECTOR_LINEARHEAP(T) ::PPE::TVector<T, ::PPE::FLinearAllocator>
//----------------------------------------------------------------------------
#define SPARSEARRAY_LINEARHEAP(_DOMAIN, T, _ChunkSize) ::PPE::TAlignedSparseArray<T, _ChunkSize, ::PPE::FLinearAllocator>
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_LINEARHEAP(_KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR_LINEARHEAP(::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_LINEARHEAP(T) ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, ::PPE::FLinearAllocator>
//----------------------------------------------------------------------------
#define HASHMAP_LINEARHEAP(_KEY, _VALUE) ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::FLinearAllocator>
//----------------------------------------------------------------------------
#define MEMORYSTREAM_LINEARHEAP() ::PPE::TMemoryStream<::PPE::FLinearAllocator>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Linear allocator have an handle to a FLinearHeap
//----------------------------------------------------------------------------
class PPE_CORE_API FLinearAllocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::true_type;
    using has_owns = std::true_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FLinearHeap* Heap;

    explicit FLinearAllocator(FLinearHeap& heap) NOEXCEPT
    :   Heap(&heap)
    {}
    explicit FLinearAllocator(Meta::FForceInit) NOEXCEPT
    :   Heap(nullptr)
    {}

    static size_t MaxSize() NOEXCEPT {
        return FLinearHeap::MaxBlockSize;
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return FLinearHeap::SnapSizeForRecycling(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return Heap->AliasesToHeap(b.Data);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ Heap->Allocate(s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        Heap->Release(b.Data, b.SizeInBytes);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = Heap->Relocate(b.Data, s, b.SizeInBytes);;
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

    friend bool operator ==(const FLinearAllocator& lhs, const FLinearAllocator& rhs) {
        return (lhs.Heap == rhs.Heap);
    }

    friend bool operator !=(const FLinearAllocator& lhs, const FLinearAllocator& rhs) {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
