#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPageAllocator : public FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;
    using has_maxsize = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, PAGE_SIZE);
    STATIC_CONST_INTEGRAL(size_t, PageSize, PAGE_SIZE);

    FPageAllocator() = default;

    NODISCARD static size_t MaxSize() NOEXCEPT {
        return PageSize;
    }

    NODISCARD size_t SnapSize(size_t s) const NOEXCEPT {
        return Meta::RoundToNextPow2(Alignment, s);
    }

    NODISCARD PPE_CORE_API FAllocatorBlock Allocate(size_t s);
    PPE_CORE_API void Deallocate(FAllocatorBlock b);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!PPE
