#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/Heap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton = Heaps::Process >
class THeapAllocator : public TAllocatorBase<T> {
public:
    typedef TAllocatorBase<T> base_type;
    typedef _HeapSingleton heapsingleton_type;

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
    struct rebind {
        typedef THeapAllocator<U, heapsingleton_type> other;
    };

    static FHeap& HeapInstance() { return heapsingleton_type::Instance(); }

    THeapAllocator() throw() {}

    THeapAllocator(const THeapAllocator&) throw() {}
    template <typename U>
    THeapAllocator(const THeapAllocator<U, _HeapSingleton>&) throw() {}

    THeapAllocator& operator =(const THeapAllocator&) { return *this; }
    template <typename U>
    THeapAllocator& operator =(const THeapAllocator<U, _HeapSingleton>&) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type );

    // see AllocatorRealloc()
    void* relocate(void* p, size_type newSize, size_type oldSize);

    template <typename U>
    friend bool operator ==(const THeapAllocator&/* lhs */, const THeapAllocator<U, _HeapSingleton>&/* rhs */) {
        return true;
    }

    template <typename U>
    friend bool operator !=(const THeapAllocator& lhs, const THeapAllocator<U, _HeapSingleton>& rhs) {
        return !operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
auto THeapAllocator<T, _HeapSingleton>::allocate(size_type n) -> pointer {
    // The return value of allocate(0) is unspecified.
    // TMallocator returns NULL in order to avoid depending
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
        CORE_THROW_IT(std::length_error("THeapAllocator<T, _HeapSingleton>::allocate() - Integer overflow."));

    // THeapAllocator wraps FHeap.
    void * const pv = HeapInstance().Malloc<std::alignment_of<T>::value>(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
void THeapAllocator<T, _HeapSingleton>::deallocate(void* p, size_type n) {
    UNUSED(n);

    // THeapAllocator wraps FHeap.
    HeapInstance().Free<std::alignment_of<T>::value>(p);
}
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
void* THeapAllocator<T, _HeapSingleton>::relocate(void* p, size_type newSize, size_type oldSize) {
    UNUSED(oldSize);

    // THeapAllocator wraps FHeap.
    void* const newp = HeapInstance().Realloc<std::alignment_of<T>::value>(p, newSize * sizeof(T));
    if (nullptr == newp && newSize)
        CORE_THROW_IT(std::bad_alloc());

    return static_cast<T*>(newp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
size_t AllocatorSnapSize(const THeapAllocator<T, _HeapSingleton>&, size_t size) {
    const auto& heap = THeapAllocator<T, _HeapSingleton>::HeapInstance();
    return (heap.SnapSize(size * sizeof(T)) / sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
