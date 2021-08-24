#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Container/IntrusiveList.h"
#include "HAL/PlatformMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Divide allocations in _BlockStep buckets, one allocator per _Bucket
//----------------------------------------------------------------------------
template <
    typename _Allocator,
    size_t _BlockMinSize,
    size_t _BlockMaxSize,
    size_t _BlockStep = ALLOCATION_BOUNDARY >
class TBucketAllocator : private _Allocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using propagate_on_container_copy_assignment = typename allocator_traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename allocator_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename allocator_traits::propagate_on_container_swap;

    using is_always_equal = typename allocator_traits::is_always_equal;

    using has_BlockMaxSize = std::true_type;
    using has_owns = typename allocator_traits::has_owns;
    using has_reallocate = typename allocator_traits::has_reallocate;
    using has_acquire = typename allocator_traits::has_acquire;
    using has_steal = typename allocator_traits::has_steal;
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = typename allocator_traits::has_memory_tracking;
#endif

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    STATIC_CONST_INTEGRAL(size_t, BlockMinSize, _BlockMinSize);
    STATIC_CONST_INTEGRAL(size_t, BlockMaxSize, _BlockMaxSize);
    STATIC_CONST_INTEGRAL(size_t, BlockStep, _BlockStep);

    STATIC_ASSERT(BlockMinSize < BlockMaxSize);
    STATIC_ASSERT(Meta::IsAligned(BlockStep, BlockMinSize));
    STATIC_ASSERT(Meta::IsAligned(BlockStep, BlockMaxSize));

    STATIC_CONST_INTEGRAL(size_t, NumBuckets, (BlockMaxSize - BlockMinSize) / BlockStep);

    TBucketAllocator() = default;

    explicit TBucketAllocator(const _Allocator& alloc)
    :   _Allocator(alloc)
    {}
    explicit TBucketAllocator(_Allocator&& ralloc)
    :   _Allocator(std::move(ralloc))
    {}

    // copy
    TBucketAllocator(const TBucketAllocator& other)
    :   _Allocator(allocator_traits::SelectOnCopy(other))
    {}
    TBucketAllocator& operator =(const TBucketAllocator& other) {
        allocator_traits::Copy(this, other);
        return (*this);
    }

    // move
    TBucketAllocator(TBucketAllocator&& rvalue) NOEXCEPT
    :   _Allocator(allocator_traits::SelectOnMove(std::move(rvalue)))
    {}
    TBucketAllocator& operator =(TBucketAllocator&& rvalue) NOEXCEPT {
        allocator_traits::Move(this, std::move(rvalue));
        return (*this);
    }

    size_t MaxSize() const NOEXCEPT {
        return Min(allocator_traits::MaxSize(*this), BlockMaxSize);
    }

    size_t SnapSize(size_t s) const NOEXCEPT {
        Assert(s >= BlockMinSize);
        Assert(s <= BlockMaxSize);

        s = allocator_traits::SnapSize(*this, s);
        return ((s + BlockStep - 1 - BlockMinSize) / BlockStep) * BlockStep;
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return allocator_traits::Owns(*this, b);
    }

    FAllocatorBlock Allocate(size_t s) {
        return allocator_traits::Allocate(*this, SnapSize(s));
    }

    void Deallocate(FAllocatorBlock b) {
        Assert_NoAssume(SnapSize(b.SizeInBytes) == b.SizeInBytes);
        allocator_traits::Deallocate(*this, b);
    }

    auto Reallocate(FAllocatorBlock& b, size_t s) {
        Assert_NoAssume(SnapSize(b.SizeInBytes) == b.SizeInBytes);
        return  allocator_traits::Reallocate(b, s);
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        return (SnapSize(b.SizeInBytes) == b.SizeInBytes &&
                allocator_traits::Acquire(*this, b));
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        Assert_NoAssume(SnapSize(b.SizeInBytes) == b.SizeInBytes);
        return allocator_traits::Steal(*this, b);
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT {
        return allocator_traits::TrackingData(*this);
    }

    auto AllocatorWithoutTracking() NOEXCEPT {
        return allocator_traits::AllocatorWithoutTracking(*this);
    }
#endif

    friend bool operator ==(const TBucketAllocator& lhs, TBucketAllocator& rhs) NOEXCEPT {
        return allocator_traits::Equals(lhs, rhs);
    }

    friend bool operator !=(const TBucketAllocator& lhs, TBucketAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TBucketAllocator& lhs, TBucketAllocator& rhs) NOEXCEPT {
        allocator_traits::Swap(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
