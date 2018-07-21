#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"
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

    template <typename... _Args>
    void construct(pointer p, _Args&&... args) { Meta::Construct(p, std::forward<_Args>(args)...); }
    void destroy(pointer p) { Meta::Destroy(p); }

    size_type max_size() const
    {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
    }

    /*
    ** Implemented in derived classes :
    */

    //pointer allocate(size_type n, const void* hint = 0) {}
    //void deallocate(void* p, size_type n) {}

    // AllocatorRealloc()
    //void* relocate(void* p, size_type newSize, size_type oldSize) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Construct/Destroy ranges
//----------------------------------------------------------------------------
namespace details {
template <typename _Allocator, typename T>
void Construct_(_Allocator&, const TMemoryView<T>&, std::true_type) {}
template <typename _Allocator, typename T, typename _Arg0, typename... _Args>
void Construct_(_Allocator&, const TMemoryView<T>&, std::true_type, _Arg0&& arg0, _Args&&... args) {
    for (T& pod : items)
        alloc.construct(&pod, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
template <typename _Allocator, typename T, typename... _Args>
void Construct_(_Allocator& alloc, const TMemoryView<T>& items, std::false_type, _Args&&... args) {
    for (T& non_pod : items)
        alloc.construct(&non_pod, std::forward<_Args>(args)...);
}
} //!details
template <typename _Allocator, typename T, typename... _Args>
void Construct(_Allocator& alloc, const TMemoryView<T>& items, _Args&&... args) {
    details::Construct_(alloc, items, typename Meta::TIsPod<T>{}, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
namespace details {
template <typename _Allocator, typename T>
void Destroy_(_Allocator&, const TMemoryView<T>&, std::true_type) {}
template <typename _Allocator, typename T>
void Destroy_(_Allocator& alloc, const TMemoryView<T>& items, std::false_type) {
    for (T& non_pod : items)
        alloc.destroy(&non_pod);
}
} //!details
template <typename _Allocator, typename T>
void Destroy(_Allocator& alloc, const TMemoryView<T>& items) {
    details::Destroy_(alloc, items, typename Meta::TIsPod<T>{});
}
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
Meta::TEnableIf<
    true  == Meta::TIsPod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>   Relocate_AssumeNoRealloc(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    typedef std::allocator_traits<_Allocator> allocator_traits;
    typedef typename allocator_traits::pointer pointer;
    Assert(0 == oldSize || nullptr != data.Pointer());
    pointer const p = data.Pointer();
    pointer const newp = newSize ? allocator.allocate(newSize) : nullptr;
    const size_t copyRange = (newSize < data.size()) ? newSize : data.size();
    if (copyRange) {
        Assert(p);
        Assert(newp);
#if 0
        std::copy(p, p + copyRange, MakeCheckedIterator(newp, newSize, 0));
#else
        FPlatformMemory::MemcpyLarge(newp, p, copyRange * sizeof(*p));
#endif
    }
    if (data.Pointer()) {
        Assert(0 < oldSize);
        allocator_traits::deallocate(allocator, p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TEnableIf<
    false  == Meta::TIsPod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>   Relocate_AssumeNoRealloc(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
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

    if (data.Pointer()) {
        Assert(p);

        Destroy(allocator, data);

        allocator_traits::deallocate(allocator, p, oldSize);
    }
    return newp;
}
//----------------------------------------------------------------------------
// Best case : T is a pod and _Allocator supports reallocate()
template <typename _Allocator>
Meta::TEnableIf<
    true  == allocator_has_realloc<_Allocator>::value &&
    true  == Meta::TIsPod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>   Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.relocate(data.Pointer(), newSize, oldSize));
}
//----------------------------------------------------------------------------
// Worst case : T is a pod but _Allocator does not support relocate()
template <typename _Allocator>
Meta::TEnableIf<
    false == allocator_has_realloc<_Allocator>::value &&
    true  == Meta::TIsPod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>   Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate_AssumeNoRealloc(allocator, data, newSize, oldSize);
}
//----------------------------------------------------------------------------
// Common case : T is not a pod, whether _Allocator supports relocate() or not
template <typename _Allocator>
Meta::TEnableIf<
    false == Meta::TIsPod<typename _Allocator::value_type>::value,
    typename _Allocator::pointer
>   Relocate(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate_AssumeNoRealloc(allocator, data, newSize, oldSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use these when T is not a standard POD, but you know it call be treated as one
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TEnableIf<
    true  == allocator_has_realloc<_Allocator>::value,
    typename _Allocator::pointer
>   Relocate_AssumePod(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return static_cast<typename _Allocator::pointer>(allocator.relocate(data.Pointer(), newSize, oldSize));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TEnableIf<
    false == allocator_has_realloc<_Allocator>::value,
    typename _Allocator::pointer
>   Relocate_AssumePod(_Allocator& allocator, const TMemoryView<typename _Allocator::value_type>& data, size_t newSize, size_t oldSize) {
    return Relocate(allocator, data, newSize, oldSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Must be overloaded by each allocator,
//  - Correctly handle insitu allocations
//  - Minimize wasted size for heap allocations
//----------------------------------------------------------------------------
// never defined, it should be specialized for each allocator
template <typename _Allocator>
size_t AllocatorSnapSize(const _Allocator&, size_t size);
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t SafeAllocatorSnapSize(const _Allocator& alloc, size_t size) {
#ifdef WITH_CORE_ASSERT
    const size_t snapped = AllocatorSnapSize(alloc, size);
    Assert(snapped >= size);
    Assert(AllocatorSnapSize(alloc, snapped) == snapped);
    return snapped;
#else
    return AllocatorSnapSize(alloc, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Can be overloaded by each allocator,
//  - Handles stealing a block from an allocator to another
//  - Used to disable memory stealing when not available and keep track of stolen blocks
//----------------------------------------------------------------------------
template <typename _Allocator>
std::false_type/* disabled */ AllocatorStealFrom(_Allocator&, typename _Allocator::pointer, size_t) {}
//----------------------------------------------------------------------------
template <typename _Allocator>
std::false_type/* disabled */ AllocatorAcquireStolen(_Allocator&, typename _Allocator::pointer, size_t) {}
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
struct allocator_can_steal_from : std::is_same<_AllocatorSrc, _AllocatorDst> {};
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
bool AllocatorCheckStealing(_AllocatorDst&, _AllocatorSrc&) { return true; }
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
struct allocator_can_steal_block {
    using stealfrom_type = decltype(AllocatorStealFrom(
        std::declval<_AllocatorSrc&>(),
        std::declval<typename _AllocatorSrc::pointer>(), 0 ));
    using acquirestolen_type = decltype(AllocatorAcquireStolen(
        std::declval<_AllocatorDst&>(),
        std::declval<typename _AllocatorDst::pointer>(), 0 ));
    static constexpr bool implements_stealing =
        stealfrom_type::value &&
        acquirestolen_type::value &&
        allocator_can_steal_from<_AllocatorDst, _AllocatorSrc>::value;

    static constexpr bool value = (std::is_same_v<_AllocatorSrc, _AllocatorDst> || implements_stealing);
};
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
Meta::TEnableIf<
    allocator_can_steal_block<_AllocatorDst, _AllocatorSrc>::value,
    TMemoryView<typename _AllocatorDst::value_type>
>   AllocatorStealBlock(_AllocatorDst& dst, const TMemoryView<typename _AllocatorSrc::value_type>& block, _AllocatorSrc& src) {
    Assert(AllocatorCheckStealing(dst, src));
    const TMemoryView<typename _AllocatorDst::value_type> stolen = block.template Cast<typename _AllocatorDst::value_type>();
    AllocatorStealFrom(src, block.data(), block.size());
    AllocatorAcquireStolen(dst, stolen.data(), stolen.size());
    return stolen;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Prettier syntax for allocator rebinding
//----------------------------------------------------------------------------
template <typename _Allocator, typename T>
using TRebindAlloc = typename std::allocator_traits<_Allocator>::template rebind_alloc<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
