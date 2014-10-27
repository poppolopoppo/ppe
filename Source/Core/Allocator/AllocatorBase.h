#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniquePtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class AllocatorBase {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_reference<T>::type reference;
    typedef typename std::add_reference<const T>::type const_reference;
    typedef T value_type;

    template<typename U>
    struct rebind
    {
        typedef AllocatorBase<U> other;
    };

    AllocatorBase() throw() {}

    AllocatorBase(const AllocatorBase& ) throw() {}
    template<typename U>
    AllocatorBase(const AllocatorBase<U>& ) throw() {}

    AllocatorBase& operator=(const AllocatorBase& ) { return *this; }
    template<typename U>
    AllocatorBase& operator=(const AllocatorBase<U>&) { return *this; }

    pointer address(reference x) const { return std::addressof(x); }
    const_pointer address(const_reference x) const { return std::addressof(x); }

    void construct(pointer p, const T& val) { ::new ((void**)p) T(val); }
    template<typename U, typename... _Args>
    void construct(U* p, _Args&&... args) { ::new((void*)p) U(std::forward<_Args>(args)...); }

    void destroy(pointer p) {
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~T();
    }

    template<typename U>
    void destroy(U* p) {
        typedef char type_must_be_complete[sizeof(U) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~U();
    }

    size_type max_size() const
    {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
    }

    //pointer allocate(size_type n, const void* hint = 0) {}
    //void deallocate(void* p, size_type n) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class AllocatorDeleter : _Allocator {
public:
    void operator ()(T* ptr) {
        _Allocator::destroy(ptr);
        _Allocator::deallocate(ptr, 1);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
using AllocatorPtr = UniquePtr<T, AllocatorDeleter<T, _Allocator> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
