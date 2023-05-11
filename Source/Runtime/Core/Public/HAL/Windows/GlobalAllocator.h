#pragma once

#include "Core_fwd.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "Allocator/AllocatorBase.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TWin32GlobalAllocator<> is wrapping GlobalMalloc/GlobalReAlloc/GlobalFree()
//----------------------------------------------------------------------------
template <UINT _uFlags>
class TWin32GlobalAllocator : private FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = std::true_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type; // ::GlobalReAlloc() inner workings are... weird
    using has_reallocate = std::false_type;
    using has_acquire = std::true_type;
    using has_steal = std::true_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, sizeof(uintptr_t));

    TWin32GlobalAllocator() = default;

    size_t SnapSize(size_t s) const NOEXCEPT { return s; }

    FAllocatorBlock Allocate(size_t s) const {
        return FAllocatorBlock{ ::GlobalAlloc(_uFlags, s), s };
    }

    void Deallocate(FAllocatorBlock b) const {
        ::GlobalFree((::HGLOBAL)b.Data);
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
using FWin32GlobalAllocatorHnd = TWin32GlobalAllocator<GHND>;
using FWin32GlobalAllocatorPtr = TWin32GlobalAllocator<GPTR>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
