#pragma once

#include "Core.h"

#include "Allocator/AllocatorBase.h"
#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"

#include <type_traits>

#define USE_PPE_INSITU_ALLOCATOR (!USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
struct ALIGN(ALLOCATION_BOUNDARY) TInSituStorage {
    STATIC_CONST_INTEGRAL(size_t, Capacity, _Capacity);

    using storage_type = Meta::TAlignedStorage<
        // keep the storage aligned to avoid any warning about padding
#if 0
        Capacity * sizeof(T),
#else
        Meta::RoundToNext(Capacity * sizeof(T), ALLOCATION_BOUNDARY),
#endif
        ALLOCATION_BOUNDARY
    >;

    storage_type InSitu;

    void* data() { return (&InSitu); }
    const void* data() const { return (&InSitu); }

#ifdef WITH_PPE_ASSERT
    // Verify that there's only one insitu allocation in flight
    // This is an important assumption which allows to avoid state tracking in release builds
    STATIC_CONST_INTEGRAL(size_t, GStateBusy, CODE3264(0x0CC491EDul, 0x0CC491ED0CC491EDul));
    STATIC_CONST_INTEGRAL(size_t, GStateEmpty, CODE3264(0xFA11BAC4ul, 0xFA11BAC4FA11BAC4ul));

    size_t State; // also acts as a canary

    TInSituStorage()
    :   State(GStateEmpty) {
        FPlatformMemory::Memset(&InSitu, 0xCC, sizeof(InSitu));
    }

    ~TInSituStorage() {
        STATIC_ASSERT(_Capacity > 0);
        STATIC_ASSERT(sizeof(InSitu) >= sizeof(T) * _Capacity);
        Assert(GStateEmpty == State); // or we are "leaking" an allocation
    }

    bool UsesInSitu() const { return (GStateBusy == State); }

    void* Allocate() {
        Assert(GStateEmpty == State);

        State = GStateBusy;
        Assert_NoAssume(
            FPlatformMemory::Memtest(&InSitu, 0xCC, sizeof(InSitu)) ||
            FPlatformMemory::Memtest(&InSitu, 0xDD, sizeof(InSitu)) );

        return data();
    }

    void Deallocate(void* p) {
        Assert(data() == p);
        Assert(GStateBusy == State);

        State = GStateEmpty;
        FPlatformMemory::Memset(&InSitu, 0xDD, sizeof(InSitu));
    }

#else
    void* Allocate() { return data(); }
    void Deallocate(void*) {}

#endif
};
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
using TAlignedInSituStorage = TInSituStorage<
    T, // avoid wasting alignment memory :
    ROUND_TO_NEXT_16(sizeof(T) * _Capacity) / sizeof(T)
>;
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
class TInSituAllocator : public _Allocator {
public:
    typedef _Allocator fallback_type;

    using typename fallback_type::value_type;
    using typename fallback_type::pointer;
    using typename fallback_type::size_type;

    typedef std::false_type propagate_on_container_copy_assignment;
    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;

    typedef _Storage storage_type;

#if USE_PPE_INSITU_ALLOCATOR
    STATIC_CONST_INTEGRAL(size_t, Capacity, storage_type::Capacity);
#else
    STATIC_CONST_INTEGRAL(size_t, Capacity, 0); // force fall-back allocation
#endif

    template<typename U>
    struct rebind {
        STATIC_ASSERT(Meta::IsAligned(sizeof(U), sizeof(storage_type)));
        typedef TInSituAllocator<
            TInSituStorage<U, (sizeof(value_type) * Capacity) / sizeof(U)>,
            typename _Allocator::template rebind<U>::other
        >   other;
    };

    TInSituAllocator() EXPORT_DELETED_FUNCTION;
    TInSituAllocator(storage_type& insitu) NOEXCEPT : _insituRef(&insitu) {}

    TInSituAllocator(const TInSituAllocator&) EXPORT_DELETED_FUNCTION;
    TInSituAllocator& operator=(const TInSituAllocator&) EXPORT_DELETED_FUNCTION;

    TInSituAllocator(TInSituAllocator&& rvalue) NOEXCEPT
    :   _insituRef(rvalue._insituRef) {
        rvalue._insituRef = nullptr;
    }

    TInSituAllocator& operator=(TInSituAllocator&&) EXPORT_DELETED_FUNCTION;

    size_t InSituCapacity() const { return Capacity; }
    const storage_type& InSituData() const { return (*_insituRef); }

    bool AliasesToInSitu(const void* p, size_t sz) const {
        return FPlatformMemory::Memoverlap(p, sz, _insituRef, sizeof(storage_type));
    }

    fallback_type& FallbackAllocator() { return static_cast<fallback_type&>(*this); }
    const fallback_type& FallbackAllocator() const { return static_cast<const fallback_type&>(*this); }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(pointer p, size_type n) NOEXCEPT;

    // see Relocate()
    void* relocate(void* p, size_type newSize, size_type oldSize);

    friend bool operator ==(const TInSituAllocator& lhs, const TInSituAllocator& rhs) NOEXCEPT {
        return (lhs._insituRef == rhs._insituRef);
    }
    friend bool operator !=(const TInSituAllocator& lhs, const TInSituAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

private:
    storage_type* _insituRef = nullptr;
};
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
auto TInSituAllocator<_Storage, _Allocator>::allocate(size_type n) -> pointer {
    Assert(n > 0);
    Assert(n < fallback_type::max_size());
    Assert(_insituRef);

    pointer const p = (n <= Capacity
        ? reinterpret_cast<pointer>(_insituRef->Allocate())
        : fallback_type::allocate(n) );

    Assert(p); // fallback_type should have thrown a std::bad_alloc() exception
    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, p));
    return p;
}
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
void TInSituAllocator<_Storage, _Allocator>::deallocate(pointer p, size_type n) NOEXCEPT {
    Assert(p);
    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, p));
    Assert(n > 0);
    Assert(n < fallback_type::max_size());
    Assert(_insituRef);

    if (_insituRef->data() == p) {
        Assert(n <= Capacity);
        _insituRef->Deallocate(p);
    }
    else {
        Assert(n > Capacity);
        fallback_type::deallocate(p, n);
    }
}
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
void* TInSituAllocator<_Storage, _Allocator>::relocate(void* p, size_type newSize, size_type oldSize) {
    STATIC_ASSERT(Meta::TIsPod<value_type>::value);
    Assert(nullptr == p || 0 < oldSize);
    Assert(_insituRef);

    if (Likely(nullptr == p)) {
        Assert(newSize); // no realloc(nullptr, 0, 0)
        Assert(not oldSize);

        return allocate(newSize);
    }

    void* newp;
    if (_insituRef->data() == p) {
        Assert(oldSize);
        Assert(oldSize <= Capacity);
        Assert_NoAssume(_insituRef->UsesInSitu());

        if (0 == newSize)
            return nullptr; // nothing todo
        if (newSize <= Capacity)
            return p;

        newp = fallback_type::allocate(newSize);
        FPlatformMemory::MemcpyLarge(newp, p, Min(newSize, oldSize) * sizeof(value_type));

        _insituRef->Deallocate(p);
    }
    else {
        Assert(p);
        Assert(oldSize);
        Assert(oldSize > Capacity);

        if (Likely(newSize)) {
            // assuming fall-back allocator has implemented relocate
            newp = fallback_type::relocate(p, newSize, oldSize);
        }
        else {
            fallback_type::deallocate(p, oldSize);
            return nullptr;
        }
    }

    Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, newp));
    return newp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
size_t AllocatorSnapSize(const TInSituAllocator<_Storage, _Allocator>& allocator, size_t size) {
#if USE_PPE_INSITU_ALLOCATOR
    Assert(size);
    return (size > TInSituAllocator<_Storage, _Allocator>::Capacity
        ? AllocatorSnapSize(allocator.FallbackAllocator(), size)
        : TInSituAllocator<_Storage, _Allocator>::Capacity );
#else
    return AllocatorSnapSize(allocator.FallbackAllocator(), size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Can't steal from insitu, but it may be possible with fall-back allocator :
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator, typename _Storage2, typename _Allocator2>
struct allocator_can_steal_from<
    TInSituAllocator<_Storage, _Allocator>,
    TInSituAllocator<_Storage2, _Allocator2>
>   : std::bool_constant<
    sizeof(typename _Storage::storage_type) ==
    sizeof(typename _Storage2::storage_type) &&
    allocator_can_steal_from<_Allocator, _Allocator2>::value
> {};
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator, typename _Allocator2>
struct allocator_can_steal_from<
    TInSituAllocator<_Storage, _Allocator>,
    _Allocator2
>   : allocator_can_steal_from<_Allocator, _Allocator2> {};
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator, typename _Allocator2>
struct allocator_can_steal_from<
    _Allocator,
    TInSituAllocator<_Storage, _Allocator2>
>   : allocator_can_steal_from<_Allocator, _Allocator2> {};
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
auto/* inherited */AllocatorStealFrom(
    TInSituAllocator<_Storage, _Allocator>& alloc,
    typename TInSituAllocator<_Storage, _Allocator>::pointer ptr, size_t size ) {
    // checks that we not stealing from insitu storage, which can't be moved
    Assert(TInSituAllocator<_Storage, _Allocator>::Capacity < size);
    Assert_NoAssume(not alloc.AliasesToInSitu(ptr, size));
    return AllocatorStealFrom(alloc.FallbackAllocator(), ptr, size);
}
//----------------------------------------------------------------------------
template <typename _Storage, typename _Allocator>
auto/* inherited */AllocatorAcquireStolen(
    TInSituAllocator<_Storage, _Allocator>& alloc,
    typename TInSituAllocator<_Storage, _Allocator>::pointer ptr, size_t size ) {
    // checks that the stolen block is larger than insitu storage, we can't break this predicate
    Assert(TInSituAllocator<_Storage, _Allocator>::Capacity < size);
    Assert_NoAssume(not alloc.AliasesToInSitu(ptr, size));
    return AllocatorAcquireStolen(alloc.FallbackAllocator(), ptr, size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
