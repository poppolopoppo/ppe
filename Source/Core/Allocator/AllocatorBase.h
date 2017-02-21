#pragma once

#include "Core/Core.h"

#include "Core/Memory/UniquePtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TAllocatorBase {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;
    typedef T value_type;

    template<typename U>
    struct rebind
    {
        typedef TAllocatorBase<U> other;
    };

    TAllocatorBase() throw() {}

    TAllocatorBase(const TAllocatorBase& ) throw() {}
    template<typename U>
    TAllocatorBase(const TAllocatorBase<U>& ) throw() {}

    TAllocatorBase& operator=(const TAllocatorBase& ) { return *this; }
    template<typename U>
    TAllocatorBase& operator=(const TAllocatorBase<U>&) { return *this; }

    pointer address(reference x) const { return std::addressof(x); }
    const_pointer address(const_reference x) const { return std::addressof(x); }

    void construct(pointer p, T&& rvalue) { ::new ((void*)p) T(std::forward<T>(rvalue)); }

    template<typename U, typename... _Args>
    typename std::enable_if< std::is_trivially_constructible<U>::value >::type
        construct(U* p, _Args&&... args) { ::new((void*)p) U{std::forward<_Args>(args)...}; }

    template<typename U, typename... _Args>
    typename std::enable_if< not std::is_trivially_constructible<U>::value >::type
        construct(U* p, _Args&&... args) { ::new((void*)p) U(std::forward<_Args>(args)...); }

    void destroy(pointer p) {
        Assert(p);
        Likely(p);
        typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complete);
        p->~T();
    }

    template<typename U>
    void destroy(U* p) {
        Assert(p);
        Likely(p);
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
    //void* relocate(void* p, size_type newSize, size_type oldSize) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TAllocatorDeleter : _Allocator {
public:
    void operator ()(T* ptr) {
        _Allocator::destroy(ptr);
        _Allocator::deallocate(ptr, 1);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
using TAllocatorPtr = TUniquePtr<T, TAllocatorDeleter<T, _Allocator> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Realloc semantic for allocators
//----------------------------------------------------------------------------
namespace details {
// Uses SFINAE to determine if an allocator implements relocate()
template<
    class _Allocator,
    class = decltype(std::declval<_Allocator>().relocate( std::declval<void*>(), std::declval<size_t>(), std::declval<size_t>() ))
>   std::true_type  _allocator_has_realloc(_Allocator&& );
    std::false_type _allocator_has_realloc(...);
} //!details
//----------------------------------------------------------------------------
template <typename _Allocator>
struct allocator_has_realloc : decltype(details::_allocator_has_realloc( std::declval<_Allocator>() )) {};
//----------------------------------------------------------------------------
template <typename _Allocator>
typename std::enable_if<
    true  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type Relocate_AssumeNoRealloc(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    typedef std::allocator_traits<_Allocator> allocator_traits;
    typedef typename allocator_traits::pointer pointer;
    Assert(0 == oldSize || nullptr != data.Pointer());
    pointer const p = data.Pointer();
    pointer const newp = newSize ? allocator.allocate(newSize) : nullptr;
    const size_t copyRange = (newSize < data.size()) ? newSize : data.size();
    if (copyRange) {
        Assert(p);
        Assert(newp);
        std::copy(p, p + copyRange, MakeCheckedIterator(newp, newSize, 0));
    }
    if (data.Pointer()) {
        Assert(0 < oldSize);
        allocator_traits::deallocate(allocator, p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
typename std::enable_if<
    false  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type Relocate_AssumeNoRealloc(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    STATIC_ASSERT(std::is_default_constructible<typename _Allocator::value_type>::value);
    STATIC_ASSERT(std::is_move_constructible<typename _Allocator::value_type>::value);
    typedef std::allocator_traits<_Allocator> allocator_traits;
    typedef typename allocator_traits::pointer pointer;
    pointer const p = data.Pointer();
    Assert(0 == oldSize || nullptr != p);
    pointer const newp = newSize ? allocator.allocate(newSize) : nullptr;
    const size_t moveRange = (newSize < data.size()) ? newSize : data.size();
    Assert((newp && p) || 0 == moveRange);
    forrange(i, 0, moveRange)
        allocator_traits::construct(allocator, newp + i, std::move(p[i]));
    forrange(i, 0, data.size())
        allocator_traits::destroy(allocator, p + i);
    if (data.Pointer()) {
        Assert(p);
        allocator_traits::deallocate(allocator, p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
// Best case : T is a pod and _Allocator supports reallocate()
template <typename _Allocator>
typename std::enable_if<
    true  == allocator_has_realloc<_Allocator>::value &&
    true  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.relocate(data.Pointer(), newSize, oldSize));
}
//----------------------------------------------------------------------------
// Worst case : T is a pod but _Allocator does not support relocate()
template <typename _Allocator>
typename std::enable_if<
    false == allocator_has_realloc<_Allocator>::value &&
    true  == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate_AssumeNoRealloc(allocator, data, newSize, oldSize);
}
//----------------------------------------------------------------------------
// Common case : T is not a pod, wheter _Allocator supports relocate() or not
template <typename _Allocator>
typename std::enable_if<
    false == std::is_pod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>::type Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate_AssumeNoRealloc(allocator, data, newSize, oldSize);
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
>::type Relocate_AssumePod(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.relocate(data.Pointer(), newSize, oldSize));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
typename std::enable_if<
    false == allocator_has_realloc<_Allocator>::value,
    typename _Allocator::pointer
>::type Relocate_AssumePod(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate(allocator, data, newSize, oldSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Must be overloaded by each allocator, used to correctly handle insitu allocations
//----------------------------------------------------------------------------
template <typename T>
size_t AllocationMinSize(const std::allocator<T>& ) {
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
