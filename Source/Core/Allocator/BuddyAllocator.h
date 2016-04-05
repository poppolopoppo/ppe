#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/BuddyHeap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _BuddyHeap = BuddyHeap >
class BuddyAllocator : public AllocatorBase<T> {
public:
    typedef AllocatorBase<T> base_type;

    using typename base_type::pointer;
    using typename base_type::size_type;

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;
    typedef std::true_type is_always_equal;

    using base_type::address;
    using base_type::construct;
    using base_type::destroy;
    using base_type::max_size;

    template<typename U>
    struct rebind
    {
        typedef BuddyAllocator<U> other;
    };

    explicit BuddyAllocator(_BuddyHeap *heap) throw() : _heap(heap) { Assert(_heap); }

    BuddyAllocator(const BuddyAllocator& other) throw() : _heap(other._heap) {}
    template <typename U>
    BuddyAllocator(const BuddyAllocator<U>& other) throw() : _heap(other._heap) {}

    BuddyAllocator& operator=(const BuddyAllocator& other) { _heap = other._heap; return *this; }
    template <typename U>
    BuddyAllocator& operator=(const BuddyAllocator<U>& other) { _heap = other._heap; return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type );

    template <typename U>
    friend bool operator ==(const BuddyAllocator& lhs, const BuddyAllocator<U, _BuddyHeap>& rhs) {
        return lhs._heap == rhs._heap;
    }

    template <typename U>
    friend bool operator !=(const BuddyAllocator& lhs, const BuddyAllocator<U, _BuddyHeap>& rhs) {
        return !operator ==(lhs, rhs);
    }

private:
    _BuddyHeap *_heap;
};
//----------------------------------------------------------------------------
template <typename T, typename _BuddyHeap>
auto BuddyAllocator<T, _BuddyHeap>::allocate(size_type n) -> pointer {
    Assert(_heap);

    // The return value of allocate(0) is unspecified.
    // Mallocator returns NULL in order to avoid depending
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
        throw std::length_error("BuddyAllocator<T, _HeapSingleton>::allocate() - Integer overflow.");

    // BuddyAllocator wraps BuddyHeap.
    void *const pv = _heap->Allocate(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        throw std::bad_alloc();

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, typename _BuddyHeap>
void BuddyAllocator<T, _BuddyHeap>::deallocate(void* p, size_type) {
    Assert(_heap);

    // BuddyAllocator wraps BuddyHeap.
    _heap->Deallocate(p);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
