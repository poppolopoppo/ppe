#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "HAL/PlatformMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSystemMallocator : private FAllocatorPolicy {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type; // STL allocators can't have a state

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::true_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FSystemMallocator() = default;

    static size_t SnapSize(size_t s) NOEXCEPT {
        return Meta::RoundToNextPow2(s, Alignment);
    }

    NODISCARD FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ FPlatformMemory::SystemAlignedMalloc(s, ALLOCATION_BOUNDARY), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        FPlatformMemory::SystemAlignedFree(b.Data, ALLOCATION_BOUNDARY);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = FPlatformMemory::SystemAlignedRealloc(b.Data, s, ALLOCATION_BOUNDARY);
        b.SizeInBytes = s;
    }

    NODISCARD bool Acquire(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }

    NODISCARD bool Steal(FAllocatorBlock b) NOEXCEPT {
        Unused(b); // nothing to do
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!PPE
