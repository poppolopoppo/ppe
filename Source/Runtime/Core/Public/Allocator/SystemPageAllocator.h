#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FSystemPageAllocator : public FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;
    using has_maxsize = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, PAGE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, PageSize, PAGE_SIZE);

    FSystemPageAllocator() = default;

    size_t MaxSize() const NOEXCEPT {
        return PageSize;
    }

    size_t SnapSize(size_t s) const NOEXCEPT {
        return Meta::RoundToNext(Alignment, s);
    }

    FAllocatorBlock Allocate(size_t s);
    void Deallocate(FAllocatorBlock b);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!PPE