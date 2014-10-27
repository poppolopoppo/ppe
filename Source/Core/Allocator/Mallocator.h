#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Allocator/Malloc.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment = 0>
class Mallocator : public AllocatorBase<T> {
public:
    typedef AllocatorBase<T> base_type;

    typedef typename base_type::pointer pointer;
    typedef typename base_type::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef Mallocator<U, _Alignment> other;
    };

    Mallocator() throw() {}

    Mallocator(const Mallocator& ) throw() {}
    template <typename U>
    Mallocator(const Mallocator<U>&) throw() {}

    Mallocator& operator=(const Mallocator& ) { return *this; }
    template <typename U>
    Mallocator& operator=(const Mallocator<U>&) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);
};
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
auto Mallocator<T, _Alignment>::allocate(size_type n) -> pointer {
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
        throw std::length_error("Mallocator<T>::allocate() - Integer overflow.");

    // Mallocator wraps malloc().
    void * const pv = malloc<_Alignment>(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        throw std::bad_alloc();

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
void Mallocator<T, _Alignment>::deallocate(void* p, size_type ) {
    // Mallocator wraps malloc().
    free<_Alignment>(p);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class Mallocator<T, 0> : public AllocatorBase<T> {
public:
    typedef AllocatorBase<T> base_type;

    typedef typename base_type::pointer pointer;
    typedef typename base_type::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef Mallocator<U, 0> other;
    };

    Mallocator() throw() {}

    Mallocator(const Mallocator& ) throw() {}
    template <typename U>
    Mallocator(const Mallocator<U>&) throw() {}

    Mallocator& operator=(const Mallocator& ) { return *this; }
    template <typename U>
    Mallocator& operator=(const Mallocator<U>&) { return *this; }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);
};
//----------------------------------------------------------------------------
template <typename T>
auto Mallocator<T, 0>::allocate(size_type n) -> pointer {
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
    if (n > base_type::max_size())
        throw std::length_error("Mallocator<T>::allocate() - Integer overflow.");

    // Mallocator wraps malloc().
    void * const pv = malloc<Alignment>(n * sizeof(T));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        throw std::bad_alloc();

    return static_cast<T *>(pv);
}
//----------------------------------------------------------------------------
template <typename T>
void Mallocator<T, 0>::deallocate(void* p, size_type ) {
    enum { Alignment = std::alignment_of<T>::value };

    // Mallocator wraps malloc().
    free<Alignment>(p);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
bool operator ==(const Mallocator<T, _Alignment>& lhs, const Mallocator<T, _Alignment>& rhs) {
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment>
bool operator !=(const Mallocator<T, _Alignment>& lhs, const Mallocator<T, _Alignment>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment, typename _Other >
bool operator ==(const Mallocator<T, _Alignment>& lhs, const _Other& rhs) {
    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Alignment, typename _Other >
bool operator !=(const Mallocator<T, _Alignment>& lhs, const _Other& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
