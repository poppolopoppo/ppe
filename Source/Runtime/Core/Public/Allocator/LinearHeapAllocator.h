#pragma once

#include "Core.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/LinearHeap.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define VECTOR_LINEARHEAP(T) ::PPE::TVector<T, ::PPE::TLinearHeapAllocator<T>>
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_LINEARHEAP(_KEY, _VALUE) ::PPE::TAssociativeVector<_KEY, _VALUE, ::PPE::Meta::TEqualTo<_KEY>, VECTOR_LINEARHEAP(::PPE::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_LINEARHEAP(T) ::PPE::THashSet<T, ::PPE::Meta::THash<T>, ::PPE::Meta::TEqualTo<T>, ::PPE::TLinearHeapAllocator<T>>
//----------------------------------------------------------------------------
#define HASHMAP_LINEARHEAP(_KEY, _VALUE) ::PPE::THashMap<_KEY, _VALUE, ::PPE::Meta::THash<_KEY>, ::PPE::Meta::TEqualTo<_KEY>, ::PPE::TLinearHeapAllocator<::PPE::TPair<_KEY, _VALUE>>>
//----------------------------------------------------------------------------
#define MEMORYSTREAM_LINEARHEAP() ::PPE::TMemoryStream<::PPE::TLinearHeapAllocator<u8>>
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TLinearHeapAllocator : public TAllocatorBase<T> {
public:
    template <typename U>
    friend class TLinearHeapAllocator;

    typedef TAllocatorBase<T> base_type;

    using typename base_type::pointer;
    using typename base_type::size_type;

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;

    using base_type::address;
    using base_type::construct;
    using base_type::destroy;
    using base_type::max_size;

    template<typename U>
    struct rebind {
        typedef TLinearHeapAllocator<U> other;
    };

    TLinearHeapAllocator(FLinearHeap& heap) noexcept
    :   _heap(&heap) {
        Assert(_heap);
    }

    TLinearHeapAllocator(Meta::FForceInit) noexcept
    :   _heap(nullptr) 
    {}

    TLinearHeapAllocator(const TLinearHeapAllocator& other) noexcept
        : TLinearHeapAllocator(*other._heap) {}

    template <typename U>
    TLinearHeapAllocator(const TLinearHeapAllocator<U>& other) noexcept
        : TLinearHeapAllocator(*other._heap) {}

    TLinearHeapAllocator& operator=(const TLinearHeapAllocator& other) noexcept {
        _heap = other._heap;
        return *this;
    }

    template <typename U>
    TLinearHeapAllocator& operator=(const TLinearHeapAllocator<U>& other) noexcept {
        _heap = other._heap;
        return *this;
    }

    FLinearHeap* Heap() const { return _heap; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    // see AllocatorRealloc()
    void* relocate(void* p, size_type newSize, size_type oldSize);

#if 0 // nooope this is leaking blocks !
    // overload destroy() to disable destructor calls with heap !
    void destroy(pointer p) { NOOP(p); }
#endif

private:
    FLinearHeap* _heap;
};
//----------------------------------------------------------------------------
template <typename T>
auto TLinearHeapAllocator<T>::allocate(size_type n) -> pointer {
    Assert(_heap);

    // The return value of allocate(0) is unspecified.
    // TLinearHeapAllocator returns NULL in order to avoid depending
    // on malloc(0)'s implementation-defined behavior
    // (the implementation can define malloc(0) to return NULL,
    // in which case the bad_alloc check below would fire).
    // All allocators can return NULL in this case.
    if (n == 0)
        return nullptr;

    // All allocators should contain an integer overflow check.
    // The Standardization Committee recommends that std::length_error
    // be thrown in the case of integer overflow.
    if (n > max_size())
        PPE_THROW_IT(std::length_error("TLinearHeapAllocator<T>::allocate() - Integer overflow."));

    // TLinearHeapAllocator wraps FLinearHeap
    void * const pv = _heap->Allocate(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        PPE_THROW_IT(std::bad_alloc());

    Assert(Meta::IsAligned(16, pv));
    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T>
void TLinearHeapAllocator<T>::deallocate(void* p, size_type n) {
    Assert(_heap);

    if (p) {
        Assert(n);
        Assert(Meta::IsAligned(16, p));

        // TLinearHeapAllocator wraps FLinearHeap
        _heap->Release(p, n * sizeof(T));
    }
    else {
        Assert(0 == n);
    }
}
//----------------------------------------------------------------------------
template <typename T>
void* TLinearHeapAllocator<T>::relocate(void* p, size_type newSize, size_type oldSize) {
    Assert(_heap);
    Assert(not p || Meta::IsAligned(16, p));

    // TLinearHeapAllocator wraps FLinearHeap
    void* const newp = _heap->Relocate(p, newSize * sizeof(T), oldSize * sizeof(T));
    if (nullptr == newp && newSize)
        PPE_THROW_IT(std::bad_alloc());

    Assert(Meta::IsAligned(16, newp));
    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
bool operator ==(const TLinearHeapAllocator<U>& lhs, const TLinearHeapAllocator<V>& rhs) {
    return (lhs.Heap() == rhs.Heap());
}
//----------------------------------------------------------------------------
template <typename U, typename V>
bool operator !=(const TLinearHeapAllocator<U>& lhs, const TLinearHeapAllocator<V>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
size_t AllocatorSnapSize(const TLinearHeapAllocator<T>&, size_t size) {
    return (FLinearHeap::SnapSizeForRecycling(size * sizeof(T)) / sizeof(T)); // align on 16 bytes boundary
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V>
struct allocator_can_steal_from<
    TLinearHeapAllocator<U>,
    TLinearHeapAllocator<V>
> : Meta::TIntegralConstant<bool, true> {};
//----------------------------------------------------------------------------
template <typename U, typename V>
bool AllocatorCheckStealing(TLinearHeapAllocator<U>& dst, TLinearHeapAllocator<V>& src) {
    return (dst.Heap() == src.Heap());
}
//----------------------------------------------------------------------------
template <typename T>
std::true_type/* enabled */AllocatorStealFrom(
    TLinearHeapAllocator<T>&,
    typename TLinearHeapAllocator<T>::pointer, size_t) {
    return std::true_type{}; // nothing to do, everything is already checked statically
}
//----------------------------------------------------------------------------
template <typename T>
std::true_type/* enabled */AllocatorAcquireStolen(
    TLinearHeapAllocator<T>&,
    typename TLinearHeapAllocator<T>::pointer, size_t) {
    return std::true_type{}; // nothing to do, everything is already checked statically
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
