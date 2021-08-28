#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Domain>
class PPE_CORE_API TVirtualAllocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::false_type;
    using has_acquire = std::false_type;
    using has_steal = std::false_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, ALLOCATION_GRANULARITY);

    TVirtualAllocator() = default;

    size_t SnapSize(size_t s) const NOEXCEPT {
        return FVirtualMemory::SnapSize(s);
    }

    FAllocatorBlock Allocate(size_t s) const {
        FAllocatorBlock blk;
        blk.SizeInBytes = s;
        blk.Data = FVirtualMemory::InternalAlloc(s ARGS_IF_MEMORYDOMAINS(*TrackingData()));
        return blk;
    }

    void Deallocate(FAllocatorBlock b) const {
        FVirtualMemory::InternalFree(b.Data, b.SizeInBytes ARGS_IF_MEMORYDOMAINS(*TrackingData()));
    }

#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::true_type;

    NODISCARD FMemoryTracking* TrackingData() NOEXCEPT {
        return std::addressof(_Domain::TrackingData());
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE