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
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;
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

    void construct(pointer p, T&& rvalue) { ::new ((void**)p) T(std::forward<T>(rvalue)); }
    template<typename U, typename... _Args>
    void construct(U* p, _Args&&... args) { ::new((void*)p) U(std::forward<_Args>(args)...); }

    void destroy(pointer p) {
        Assert(p);
        __assume(p);
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~T();
    }

    template<typename U>
    void destroy(U* p) {
        Assert(p);
        __assume(p);
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

    // AllocatorRealloc()
    //void* rellocate(void* p, size_type newSize, size_type oldSize) {}
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
// Realloc semantic for allocators
//----------------------------------------------------------------------------
namespace details {
// Uses SFINAE to determine if an allocator implements rellocate()
template<
    class _Allocator,
    class = decltype(std::declval<_Allocator>().rellocate( std::declval<void*>(), std::declval<size_t>(), std::declval<size_t>() ))
>   std::true_type  _allocator_has_realloc(_Allocator&& );
    std::false_type _allocator_has_realloc(...);
} //!details
//----------------------------------------------------------------------------
template <typename _Allocator>
struct allocator_has_realloc : decltype(details::_allocator_has_realloc( std::declval<_Allocator>() )) {};
//----------------------------------------------------------------------------
// Best case : T is a pod and _Allocator supports reallocate()
template <typename _Allocator>
typename std::enable_if<
    true  == allocator_has_realloc<_Allocator>::value &&
    true  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type AllocatorRealloc(_Allocator& allocator, typename _Allocator::pointer p, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.rellocate(p, newSize, oldSize));
}
//----------------------------------------------------------------------------
// Worst case : T is a pod but _Allocator does not support rellocate()
template <typename _Allocator>
typename std::enable_if<
    false == allocator_has_realloc<_Allocator>::value &&
    true  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type AllocatorRealloc(_Allocator& allocator, typename _Allocator::pointer p, size_t newSize, size_t oldSize) {
    STATIC_ASSERT(false); // should be handled by all allocators, this is a warning
    typename _Allocator::pointer const newp = newSize ? allocator.allocate(newSize) : nullptr;
    const size_t copyRange = (newSize < oldSize) ? newSize : oldSize;
    if (copyRange) {
        Assert(p);
        Assert(newp);
        std::copy(p, p + copyRange, newp);
    }
    if (oldSize) {
        Assert(p);
        allocator.deallocate(p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
// Common case : T is not a pod, wheter _Allocator supports rellocate() or not
template <typename _Allocator>
typename std::enable_if<
    false == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type AllocatorRealloc(_Allocator& allocator, typename _Allocator::pointer p, size_t newSize, size_t oldSize) {
    STATIC_ASSERT(std::is_default_constructible<typename _Allocator::value_type>::value);
    STATIC_ASSERT(std::is_move_constructible<typename _Allocator::value_type>::value);
    typename _Allocator::pointer const newp = newSize ? allocator.allocate(newSize) : nullptr;
    const size_t moveRange = (newSize < oldSize) ? newSize : oldSize;
    Assert((newp && p) || 0 == moveRange);
    forrange(i, 0, moveRange) {
        allocator.construct(newp + i, std::move(p[i]));
        allocator.destroy(p + i);
    }
    forrange(i, moveRange, newSize) {
        allocator.construct(newp + i);
    }
    if (oldSize) {
        Assert(p);
        allocator.deallocate(p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use these when T is not a standard POD, but you know it call be treated as one
//----------------------------------------------------------------------------
template <typename _Allocator>
typename std::enable_if<
    true  == allocator_has_realloc<_Allocator>::value,
    typename _Allocator::pointer
>::type AllocatorRealloc_AssumePod(_Allocator& allocator, typename _Allocator::pointer p, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.rellocate(p, newSize, oldSize));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
typename std::enable_if<
    false == allocator_has_realloc<_Allocator>::value,
    typename _Allocator::pointer
>::type AllocatorRealloc_AssumePod(_Allocator& allocator, typename _Allocator::pointer p, size_t newSize, size_t oldSize) {
    return AllocatorRealloc(allocator, p, newSize, oldSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
