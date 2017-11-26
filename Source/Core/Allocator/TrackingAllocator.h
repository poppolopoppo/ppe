#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

#include <memory>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Domain, typename _Allocator>
class TTrackingAllocator;
//----------------------------------------------------------------------------
namespace Meta {
//----------------------------------------------------------------------------
template <typename T>
struct IsATrackingAllocator {
    enum : bool { value = false };
};
template <typename _Domain, typename _Allocator>
struct IsATrackingAllocator< TTrackingAllocator<_Domain, _Allocator> > {
    enum : bool { value = true };
};
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
// See AllocatorRealloc()
template <typename _Domain, typename _Allocator, bool = allocator_has_realloc<_Allocator>::value >
class fwd_realloc_semantic_for_tracking : public _Allocator {
public:
    using _Allocator::_Allocator;
};
template <typename _Domain, typename _Allocator>
class fwd_realloc_semantic_for_tracking<_Domain, _Allocator, true> : public _Allocator {
public:
    using _Allocator::_Allocator;
    using typename _Allocator::size_type;
    using typename _Allocator::value_type;
    void* relocate(void* p, size_type newSize, size_type oldSize) {
        auto pself = static_cast<TTrackingAllocator<_Domain, _Allocator>* >(this);

        if (p && pself->TrackingData())
            pself->TrackingData()->Deallocate(oldSize, sizeof(value_type));

        void* const newp = _Allocator::relocate(p, newSize, oldSize);

        if (newp && pself->TrackingData())
            pself->TrackingData()->Allocate(newSize, sizeof(value_type));

        return newp;
    }
};
} //!details
//----------------------------------------------------------------------------
template <typename _Domain, typename _Allocator>
class TTrackingAllocator : public details::fwd_realloc_semantic_for_tracking<_Domain, _Allocator> {
public:
    STATIC_ASSERT(!Meta::IsATrackingAllocator< _Allocator >::value);

    friend class details::fwd_realloc_semantic_for_tracking<_Domain, _Allocator>;
    typedef details::fwd_realloc_semantic_for_tracking<_Domain, _Allocator> base_type;

    typedef _Domain domain_type;

    using typename base_type::value_type;
    using typename base_type::reference;
    using typename base_type::const_reference;

    typedef std::allocator_traits<_Allocator> traits_type;

    typedef typename traits_type::difference_type difference_type;
    typedef typename traits_type::size_type size_type;

    typedef typename traits_type::pointer pointer;
    typedef typename traits_type::const_pointer const_pointer;

    typedef typename traits_type::propagate_on_container_copy_assignment propagate_on_container_copy_assignment;
    typedef typename traits_type::propagate_on_container_move_assignment propagate_on_container_move_assignment;
    typedef typename traits_type::propagate_on_container_swap propagate_on_container_swap;
    typedef typename traits_type::is_always_equal is_always_equal;

    template<typename U>
    struct rebind {
        typedef TTrackingAllocator< _Domain, typename traits_type::template rebind_alloc<U> > other;
    };

    TTrackingAllocator() throw() {
        STATIC_ASSERT(  allocator_has_realloc<TTrackingAllocator>::value ==
                        allocator_has_realloc<base_type>::value );
    }
    explicit TTrackingAllocator(const base_type& allocator) throw() : base_type(allocator) {}

    TTrackingAllocator(const TTrackingAllocator& other) throw() = default;
    template<typename _D, typename _A>
    TTrackingAllocator(const TTrackingAllocator<_D, _A>& other) throw() : base_type(other) {}

    TTrackingAllocator& operator =(const TTrackingAllocator& other) = default;
    template<typename _D, typename _A>
    TTrackingAllocator& operator =(const TTrackingAllocator<_D, _A>& other) {
        base_type::operator =(other);
        return *this;
    }

    FMemoryTracking* TrackingData() const { return &(domain_type::TrackingData); }
    const _Allocator& WrappedAllocator() const { return static_cast<const _Allocator&>(*this); }

    size_type max_size() const { return base_type::max_size(); }

    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    pointer allocate(size_type n) {
        if (FMemoryTracking* const trackingData = TrackingData())
            trackingData->Allocate(n, sizeof(value_type));

        return traits_type::allocate(*this, n);
    }

    void deallocate(void* p, size_type n) {
        traits_type::deallocate(*this, pointer(p), n);

        if (FMemoryTracking* const trackingData = TrackingData())
            trackingData->Deallocate(n, sizeof(value_type));
    }

    template<typename _D, typename _A>
    friend bool operator ==(const TTrackingAllocator& lhs, const TTrackingAllocator<_D, _A>& rhs) {
        return  operator ==(static_cast<const base_type&>(lhs), static_cast<const typename TTrackingAllocator<_D, _A>::base_type&>(rhs));
    }

    template<typename _D, typename _A>
    friend bool operator !=(const TTrackingAllocator& lhs, const TTrackingAllocator<_D, _A>& rhs) {
        return !operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Domain, typename _Allocator>
size_t AllocationSnapSize(const TTrackingAllocator<_Domain, _Allocator>& allocator, size_t size) {
    return AllocationSnapSize(allocator.WrappedAllocator(), size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
