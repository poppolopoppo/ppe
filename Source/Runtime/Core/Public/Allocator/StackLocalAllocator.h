#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/Alloca.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FStackLocalAllocator is wrapping global Alloca/RelocateAlloca/FreeAlloca()
//----------------------------------------------------------------------------
class PPE_CORE_API FStackLocalAllocator : private FGenericAllocator {
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

#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;
#endif

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_BOUNDARY);

    FStackLocalAllocator() = default;

    static size_t SnapSize(size_t s) NOEXCEPT {
        return AllocaSnapSize(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ Alloca(s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        FreeAlloca(b.Data, b.SizeInBytes);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) const {
        b.Data = RelocateAlloca(b.Data, s, b.SizeInBytes, true);
        b.SizeInBytes = s;
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        UNUSED(b); // nothing to do
        return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        UNUSED(b); // nothing to do
        return true;
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& TrackingData() NOEXCEPT {
        return MEMORYDOMAIN_TRACKING_DATA(Alloca);
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
