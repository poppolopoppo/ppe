#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wraps a T& inside a T* to avoid copying T when using value semantics
template <typename T>
struct TPtrRef {
    T* Ptr;

    TPtrRef(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR TPtrRef() NOEXCEPT : Ptr(nullptr) {}

    CONSTEXPR TPtrRef(T* ptr) NOEXCEPT : Ptr(ptr) { Assert(ptr); }
    CONSTEXPR TPtrRef(T& ref) NOEXCEPT : Ptr(&ref) {}

    CONSTEXPR TPtrRef(const TPtrRef&) NOEXCEPT = default;
    CONSTEXPR TPtrRef& operator =(const TPtrRef&) NOEXCEPT = default;

    CONSTEXPR T* get() const NOEXCEPT { return Ptr; }

    CONSTEXPR T& operator * () const { Assert(Ptr); return (*Ptr); }
    CONSTEXPR T* operator ->() const { Assert(Ptr); return Ptr; }

    CONSTEXPR operator T* () const NOEXCEPT { return Ptr; }
    CONSTEXPR operator T& () const NOEXCEPT { return (*Ptr); }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const TPtrRef& rhs) NOEXCEPT {
        return (lhs.Ptr == rhs.Ptr);
    }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const TPtrRef& rhs) NOEXCEPT {
        return (lhs.Ptr != rhs.Ptr);
    }

    CONSTEXPR friend bool operator < (const TPtrRef& lhs, const TPtrRef& rhs) NOEXCEPT {
        return (lhs.Ptr <  rhs.Ptr);
    }
    CONSTEXPR friend bool operator >=(const TPtrRef& lhs, const TPtrRef& rhs) NOEXCEPT {
        return (lhs.Ptr >= rhs.Ptr);
    }
};
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TPtrRef<T> MakePtrRef(T& ref) {
    return TPtrRef{ &ref };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
