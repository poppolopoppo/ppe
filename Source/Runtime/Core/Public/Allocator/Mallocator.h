#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/Malloc.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FMallocator is wrapping global malloc/realloc/free()
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FMallocator() = default;

    size_t SnapSize(size_t s) const NOEXCEPT {
        return malloc_snap_size(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ PPE::malloc(s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        PPE::free(b.Data);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = PPE::realloc(b.Data, s);
        b.SizeInBytes = s;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Alignment>
class TAlignedMallocator : private FMallocator {
public:
    using typename FMallocator::propagate_on_container_copy_assignment;
    using typename FMallocator::propagate_on_container_move_assignment;
    using typename FMallocator::propagate_on_container_swap;

    using typename FMallocator::is_always_equal;

    using typename FMallocator::has_maxsize;
    using typename FMallocator::has_owns;
    using typename FMallocator::has_reallocate;
    using typename FMallocator::has_acquire;
    using typename FMallocator::has_steal;

    STATIC_CONST_INTEGRAL(size_t, Alignment, Max(size_t(ALLOCATION_BOUNDARY), _Alignment));
    STATIC_ASSERT(Meta::IsPow2(Alignment));

    TAlignedMallocator() = default;

    size_t SnapSize(size_t s) const NOEXCEPT {
        return FMallocator::SnapSize(Meta::RoundToNextPow2(s, _Alignment));
    }

    FAllocatorBlock Allocate(size_t s) const {
        STATIC_ASSERT(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, Alignment));
        void* result;
        IF_CONSTEXPR(Meta::need_alignment_v<_Alignment>)
            result = PPE::aligned_malloc(s, Alignment);
        else
            result = PPE::malloc_for_new(s);
        return FAllocatorBlock{ result, s };
    }

    void Deallocate(FAllocatorBlock b) const {
        IF_CONSTEXPR(Meta::need_alignment_v<_Alignment>)
            PPE::aligned_free(b.Data);
        else
            PPE::free_for_delete(b.Data, b.SizeInBytes);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        IF_CONSTEXPR(Meta::need_alignment_v<_Alignment>)
            b.Data = PPE::aligned_realloc(b.Data, s, _Alignment);
        else
            b.Data = PPE::realloc_for_new(b.Data, s, b.SizeInBytes);
        b.SizeInBytes = s;
    }

    using FMallocator::Acquire;
    using FMallocator::Steal;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
