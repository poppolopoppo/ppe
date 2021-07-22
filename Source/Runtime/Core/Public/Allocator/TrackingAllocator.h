#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Allocate a new _Allocator when allocation fails
//----------------------------------------------------------------------------
template <typename _Domain, typename _Allocator>
class TTrackingAllocator : private _Allocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;
    using domain_tag = _Domain;

#define TRACKING_USING_DEF(_NAME) \
    using _NAME = typename allocator_traits::_NAME

    TRACKING_USING_DEF(propagate_on_container_copy_assignment);
    TRACKING_USING_DEF(propagate_on_container_move_assignment);
    TRACKING_USING_DEF(propagate_on_container_swap);

    TRACKING_USING_DEF(is_always_equal);

    TRACKING_USING_DEF(has_maxsize);
    TRACKING_USING_DEF(has_owns);
    TRACKING_USING_DEF(has_reallocate);
    TRACKING_USING_DEF(has_acquire);
    TRACKING_USING_DEF(has_steal);

#undef TRACKING_USING_DEF

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    TTrackingAllocator() = default;

    explicit TTrackingAllocator(const _Allocator& alloc)
    :   _Allocator(alloc)
    {}
    explicit TTrackingAllocator(_Allocator&& ralloc)
    :   _Allocator(std::move(ralloc))
    {}

    TTrackingAllocator(const TTrackingAllocator& other)
    :   _Allocator(allocator_traits::SelectOnCopy(other))
    {}
    TTrackingAllocator& operator =(const TTrackingAllocator& other) {
        allocator_traits::Copy(this, other);
        return (*this);
    }

    TTrackingAllocator(TTrackingAllocator&& rvalue) NOEXCEPT
    :   _Allocator(allocator_traits::SelectOnMove(std::move(rvalue)))
    {}
    TTrackingAllocator& operator =(TTrackingAllocator&& rvalue) NOEXCEPT {
        allocator_traits::Move(this, std::move(rvalue));
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return allocator_traits::MaxSize();
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return allocator_traits::SnapSize(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return allocator_traits::Owns(*this, b);
    }

    FAllocatorBlock Allocate(size_t s) {
        STATIC_ASSERT(not allocator_traits::has_memory_tracking::value); // double-tracking !
        const FAllocatorBlock r = allocator_traits::Allocate(*this, s);
        Tracking().Allocate(r.SizeInBytes, SnapSize(r.SizeInBytes));
        return r;
    }

    void Deallocate(FAllocatorBlock b) {
        Tracking().Deallocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
        allocator_traits::Deallocate(*this, b);
    }

    auto Reallocate(FAllocatorBlock& b, size_t s) {
        IF_CONSTEXPR(allocator_traits::reallocate_can_fail::value) {
            const size_t oldSize = b.SizeInBytes;
            if (allocator_traits::Reallocate(*this, b, s)) {
                if (oldSize)
                    Tracking().Deallocate(oldSize, SnapSize(oldSize));
                if (b.SizeInBytes)
                    Tracking().Allocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
                return true;
            }
            else {
                return false;
            }
        }
        else {
            if (b.SizeInBytes)
                Tracking().Deallocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
            allocator_traits::Reallocate(*this, b, s);
            if (b.SizeInBytes)
                Tracking().Allocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
            return;
        }
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        if (allocator_traits::Acquire(*this, b)) {
            Tracking().Allocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
            return true;
        }
        else {
            return false;
        }
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        if (allocator_traits::Steal(*this, b)) {
            Tracking().Deallocate(b.SizeInBytes, SnapSize(b.SizeInBytes));
            return true;
        }
        else {
            return false;
        }
    }

public: // memory tracking
    using has_memory_tracking = std::true_type;

    _Allocator& InnerAlloc() NOEXCEPT {
        return allocator_traits::Get(*this);
    }

    const _Allocator& InnerAlloc() const NOEXCEPT {
        return allocator_traits::Get(*this);
    }

    static FMemoryTracking& Tracking() NOEXCEPT {
        return domain_tag::TrackingData();
    }

    FMemoryTracking& TrackingData() NOEXCEPT {
        return Tracking(); // this overload can be used with TAllocatorTraits<>
    }
};
//----------------------------------------------------------------------------
template <
    typename _DomainLhs, typename _AllocatorLhs,
    typename _DomainRhs, typename _AllocatorRhs >
bool operator ==(
    const TTrackingAllocator<_DomainLhs, _AllocatorLhs>& lhs,
    const TTrackingAllocator<_DomainRhs, _AllocatorRhs>& rhs ) NOEXCEPT {
    return TAllocatorTraits<_AllocatorLhs>::Equals(lhs.InnerAlloc(), rhs.InnerAlloc());
}
//----------------------------------------------------------------------------
template <
    typename _DomainLhs, typename _AllocatorLhs,
    typename _DomainRhs, typename _AllocatorRhs >
bool operator !=(
    const TTrackingAllocator<_DomainLhs, _AllocatorLhs>& lhs,
    const TTrackingAllocator<_DomainRhs, _AllocatorRhs>& rhs ) NOEXCEPT {
    return not operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Handles memory transfer from one domain to another
//----------------------------------------------------------------------------
template <typename _DomainDst, typename _Allocator, typename _DomainSrc>
bool StealAllocatorBlock(
    TTrackingAllocator<_DomainDst, _Allocator>* dst,
    TTrackingAllocator<_DomainSrc, _Allocator>& src,
    FAllocatorBlock b ) NOEXCEPT {
    using traits_t = TAllocatorTraits<TTrackingAllocator<_DomainSrc, _Allocator>>;
    return traits_t::StealAndAcquire(dst, src, b);
}
//----------------------------------------------------------------------------
// Also handles transfer between 2 different allocators, if already handled by parents
//----------------------------------------------------------------------------
template <typename _DomainDst, typename _AllocatorDst, typename _DomainSrc, typename _AllocatorSrc,
    class = Meta::TEnableIf< has_stealallocatorblock_v<_AllocatorDst, _AllocatorSrc> > >
bool StealAllocatorBlock(
    TTrackingAllocator<_DomainDst, _AllocatorDst>* dst,
    TTrackingAllocator<_DomainSrc, _AllocatorSrc>& src,
    FAllocatorBlock b ) NOEXCEPT {
    using src_t = TAllocatorTraits<TTrackingAllocator<_DomainSrc, _AllocatorSrc>>;
    return src_t::StealAndAcquire(dst, src, b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
