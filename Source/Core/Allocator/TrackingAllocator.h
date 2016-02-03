#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
class TrackingAllocator;
//----------------------------------------------------------------------------
namespace Meta {
//----------------------------------------------------------------------------
template <typename T>
struct IsATrackingAllocator {
    enum : bool { value = false };
};
template <typename _Allocator>
struct IsATrackingAllocator< TrackingAllocator<_Allocator> > {
    enum : bool { value = true };
};
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
// See AllocatorRealloc()
template <typename _Allocator, bool = allocator_has_realloc<_Allocator>::value >
class fwd_realloc_tracking_semantic : public _Allocator {
public:
    using _Allocator::_Allocator;
};
template <typename _Allocator>
class fwd_realloc_tracking_semantic <_Allocator, true> : public _Allocator {
public:
    using _Allocator::_Allocator;
    using typename _Allocator::size_type;
    using typename _Allocator::value_type;
    void* relocate(void* p, size_type newSize, size_type oldSize) {
        auto pself = static_cast<TrackingAllocator<_Allocator>* >(this);

        if (p && pself->_trackingData)
            pself->_trackingData->Deallocate(oldSize, sizeof(value_type));

        void* const newp = _Allocator::relocate(p, newSize, oldSize);

        if (newp && pself->_trackingData)
            pself->_trackingData->Allocate(newSize, sizeof(value_type));

        return newp;
    }
};
} //!details
//----------------------------------------------------------------------------
template <typename _Allocator>
class TrackingAllocator : public details::fwd_realloc_tracking_semantic<_Allocator> {
public:
    STATIC_ASSERT(!Meta::IsATrackingAllocator< _Allocator >::value);

    friend class details::fwd_realloc_tracking_semantic<_Allocator>;
    typedef details::fwd_realloc_tracking_semantic<_Allocator> base_type;

    using typename base_type::size_type;
    using typename base_type::difference_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;
    using typename base_type::value_type;

    template<typename U>
    struct rebind {
        typedef TrackingAllocator<typename base_type::template rebind<U>::other > other;
    };

    TrackingAllocator() throw()
        : _trackingData(&MemoryTrackingData::Global()) {}
    TrackingAllocator(MemoryTrackingData& trackingData) throw()
        : _trackingData(&trackingData) {}
    TrackingAllocator(const base_type& allocator) throw()
        : base_type(allocator), _trackingData(MemoryTrackingData::Global()) {}
    TrackingAllocator(const base_type& allocator, MemoryTrackingData& trackingData) throw()
        : base_type(allocator), _trackingData(&trackingData) {}

    TrackingAllocator(const TrackingAllocator& other) throw()
        : base_type(other), _trackingData(other.TrackingData()) {}
    template<typename U>
    TrackingAllocator(const TrackingAllocator<U>& other) throw()
        : base_type(other), _trackingData(other.TrackingData()) {}

    TrackingAllocator& operator =(const TrackingAllocator& other);
    template<typename U>
    TrackingAllocator& operator =(const TrackingAllocator<U>& other);

    size_type max_size() const { return base_type::max_size(); }

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    MemoryTrackingData* TrackingData() const { return _trackingData; }

    template <typename U>
    friend bool operator ==(const TrackingAllocator& lhs, const TrackingAllocator<U>& rhs) {
        return  operator ==(static_cast<const base_type&>(lhs), static_cast<const typename TrackingAllocator<U>::base_type&>(rhs)) &&
                lhs._trackingData == rhs._trackingData;
    }

    template <typename U>
    friend bool operator !=(const TrackingAllocator& lhs, const TrackingAllocator<U>& rhs) {
        return !operator ==(lhs, rhs);
    }

private:
    MemoryTrackingData* _trackingData;
};
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TrackingAllocator<_Allocator>::operator = (const TrackingAllocator& other) -> TrackingAllocator& {
    base_type::operator =(other);
    _trackingData = other.TrackingData();
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
template<typename U>
auto TrackingAllocator<_Allocator>::operator=(const TrackingAllocator<U>& other) -> TrackingAllocator& {
    base_type::operator =(other);
    _trackingData = other.TrackingData();
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TrackingAllocator<_Allocator>::allocate(size_type n) -> pointer {
    if (_trackingData)
        _trackingData->Allocate(n, sizeof(value_type));

    return base_type::allocate(n);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TrackingAllocator<_Allocator>::deallocate(void* p, size_type n) {
    base_type::deallocate(p, n);

    if (_trackingData)
        _trackingData->Deallocate(n, sizeof(value_type));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Tag >
class TagTrackingAllocator : public TrackingAllocator<_Allocator> {
public:
    typedef TrackingAllocator<_Allocator> base_type;
    typedef _Tag tag_type;

    using typename base_type::size_type;
    using typename base_type::difference_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;
    using typename base_type::value_type;

    template<typename U>
    struct rebind
    {
        typedef TagTrackingAllocator<typename _Allocator::template rebind<U>::other, tag_type > other;
    };

    TagTrackingAllocator() throw()
        : base_type(tag_type::TrackingData) {}
    TagTrackingAllocator(const base_type& allocator) throw()
        : base_type(allocator, tag_type::TrackingData) {}
    TagTrackingAllocator(const _Allocator& allocator) throw()
        : base_type(allocator, tag_type::TrackingData) {}

    TagTrackingAllocator(const TagTrackingAllocator& other) throw()
        : base_type(other) {}
    template<typename U>
    TagTrackingAllocator(const TagTrackingAllocator<U, tag_type>& other) throw()
        : base_type(other, tag_type::TrackingData) {}

    TagTrackingAllocator& operator =(const TagTrackingAllocator& other) {
        base_type::operator =(other);
        return *this;
    }

    template<typename U>
    TagTrackingAllocator& operator =(const TagTrackingAllocator<U, tag_type>& other) {
        base_type::operator =(other);
        return *this;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
