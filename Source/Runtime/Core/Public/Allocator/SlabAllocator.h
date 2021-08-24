#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Allocator/SlabHeap.h"
#include "Memory/PtrRef.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define SLAB_ALLOCATOR(_DOMAIN) ::PPE::TSlabAllocator< ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define ARRAY_SLAB(_DOMAIN, T) ::PPE::TArray<T, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define VECTOR_SLAB(_DOMAIN, T) ::PPE::TVector<T, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define SPARSEARRAY_SLAB(_DOMAIN, T) ::PPE::TSparseArray<T, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_SLAB(_DOMAIN, _KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR_SLAB(_DOMAIN, ::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_SLAB(_DOMAIN, T) ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define HASHMAP_SLAB(_DOMAIN, _KEY, _VALUE) ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
#define MEMORYSTREAM_SLAB(_DOMAIN) ::PPE::TMemoryStream< SLAB_ALLOCATOR(_DOMAIN) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Slab allocator have an handle to a FPoolingSlabHeap
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Unknown) >
class TSlabAllocator : private FGenericAllocator {
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

    using heap_type = TPoolingSlabHeap<_Allocator>;

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
namespace details {
template <typename T> struct TPoolingSlabAllocator_{};
template <typename _Allocator> struct TPoolingSlabAllocator_<TPoolingSlabHeap<_Allocator>> {
    using type = _Allocator;
};
}
template <typename _PoolingSlabAllocator>
using TPoolingSlabAllocator = TSlabAllocator<typename details::TPoolingSlabAllocator_<_PoolingSlabAllocator>::type>;
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
template <typename _Allocator>
TSlabAllocator(TPoolingSlabHeap<_Allocator>&) -> TSlabAllocator< Meta::TDecay<_Allocator> >;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
