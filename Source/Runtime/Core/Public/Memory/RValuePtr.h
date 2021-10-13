#pragma once

#include "Core_fwd.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Captures rvalue references, mainly to make move semantics work with std::initializer_list<>
//----------------------------------------------------------------------------
template <typename T>
struct TRValuePtr {
    T* Ptr;

    CONSTEXPR TRValuePtr(T* ptr) NOEXCEPT
    :   Ptr(ptr)
    {}

    CONSTEXPR TRValuePtr(T&& rvalue) NOEXCEPT
    :   Ptr(std::addressof(rvalue)) {
        STATIC_ASSERT(std::is_rvalue_reference_v<decltype(rvalue)>);
    }

    CONSTEXPR operator T&& () const {
        return std::move(*Ptr);
    }
};
//----------------------------------------------------------------------------
template <typename T>
using TRValueInitializerList = std::initializer_list<TRValuePtr<T>>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
