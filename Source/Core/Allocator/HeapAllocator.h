#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/Heap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton = Heaps::Process >
class HeapAllocator : public AllocatorBase<T> {
public:
    typedef AllocatorBase<T> base_type;
    typedef _HeapSingleton heapsingleton_type;

    typedef typename base_type::pointer pointer;
    typedef typename base_type::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef HeapAllocator<U, heapsingleton_type> other;
    };

    static Heap& HeapInstance() { return heapsingleton_type::Instance(); }

    HeapAllocator() throw() {}

    HeapAllocator(const HeapAllocator&) throw() {}
    template <typename U>
    HeapAllocator(const HeapAllocator<U, _HeapSingleton>&) throw() {}

    HeapAllocator& operator =(const HeapAllocator&) { return *this; }
    template <typename U>
    HeapAllocator& operator =(const HeapAllocator<U, _HeapSingleton>&) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type );

    template <typename U>
    friend bool operator ==(const HeapAllocator&/* lhs */, const HeapAllocator<U, _HeapSingleton>&/* rhs */) {
        return true;
    }
    
    template <typename U>
    friend bool operator !=(const HeapAllocator& lhs, const HeapAllocator<U, _HeapSingleton>& rhs) {
        return !operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
auto HeapAllocator<T, _HeapSingleton>::allocate(size_type n) -> pointer {
    enum { Alignment = std::alignment_of<T>::value };

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
        throw std::length_error("HeapAllocator<T, _HeapSingleton>::allocate() - Integer overflow.");

    // HeapAllocator wraps Heap.
    void * const pv = HeapInstance().malloc<Alignment>(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        throw std::bad_alloc();

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, typename _HeapSingleton >
void HeapAllocator<T, _HeapSingleton>::deallocate(void* p, size_type) {
    enum { Alignment = std::alignment_of<T>::value };

    // HeapAllocator wraps Heap.
    HeapInstance().free<Alignment>(p);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
