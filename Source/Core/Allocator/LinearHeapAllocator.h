#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/LinearHeap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define VECTOR_LINEARHEAP(T) ::Core::TVector<T, ::Core::TLinearHeapAllocator<T>>
//----------------------------------------------------------------------------
#define ASSOCIATIVE_VECTOR_LINEARHEAP(_KEY, _VALUE) ::Core::TAssociativeVector<_KEY, _VALUE, ::Core::Meta::TEqualTo<_KEY>, VECTOR_LINEARHEAP(::Core::TPair<_KEY COMMA _VALUE>)>
//----------------------------------------------------------------------------
#define HASHSET_LINEARHEAP(T) ::Core::THashSet<T, ::Core::Meta::THash<T>, ::Core::Meta::TEqualTo<T>, ::Core::TLinearHeapAllocator<T>>
//----------------------------------------------------------------------------
#define HASHMAP_LINEARHEAP(_KEY, _VALUE) ::Core::THashMap<_KEY, _VALUE, ::Core::Meta::THash<_KEY>, ::Core::Meta::TEqualTo<_KEY>, ::Core::TLinearHeapAllocator<::Core::TPair<_KEY, _VALUE>>>
//----------------------------------------------------------------------------
#define MEMORYSTREAM_LINEARHEAP() ::Core::TMemoryStream<::Core::TLinearHeapAllocator<u8>>
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
        : _heap(&heap) {
        Assert(_heap);
    }

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

    // overload destroy() to disable destructor calls with heap !
    void destroy(pointer p) { NOOP(p); }

private:
    FLinearHeap* _heap;
};
//----------------------------------------------------------------------------
template <typename T>
auto TLinearHeapAllocator<T>::allocate(size_type n) -> pointer {
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
        CORE_THROW_IT(std::length_error("TLinearHeapAllocator<T>::allocate() - Integer overflow."));

    // TLinearHeapAllocator wraps FLinearHeap
    Assert(_heap);
    void * const pv = _heap->Allocate(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T>
void TLinearHeapAllocator<T>::deallocate(void* p, size_type n) {
    Assert(_heap);

    if (p) {
        Assert(n);
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
    // TLinearHeapAllocator wraps FLinearHeap
    Assert(_heap);
    void* const newp = _heap->Relocate(p, newSize, oldSize, _Alignment);
    if (nullptr == newp && newSize)
        CORE_THROW_IT(std::bad_alloc());

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
    return (FLinearHeap::SnapSize(size * sizeof(T)) / sizeof(T)); // align on 16 bytes boundary
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
} //!namespace Core