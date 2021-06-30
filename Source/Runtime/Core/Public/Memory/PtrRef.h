#pragma once

#include "Core_fwd.h"

#include "Meta/Hash_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wraps a T& inside a T* to avoid copying T when using value semantics
//----------------------------------------------------------------------------
template <typename T>
struct TPtrRef {
    T* Ptr;

    TPtrRef() = default;

    CONSTEXPR TPtrRef(Meta::FForceInit) NOEXCEPT : Ptr(nullptr) {}

    CONSTEXPR TPtrRef(T* ptr) NOEXCEPT : Ptr(ptr) { Assert(ptr); }
    CONSTEXPR TPtrRef(T& ref) NOEXCEPT : Ptr(&ref) {}

    CONSTEXPR TPtrRef(const TPtrRef&) NOEXCEPT = default;
    CONSTEXPR TPtrRef& operator =(const TPtrRef&) NOEXCEPT = default;

    CONSTEXPR TPtrRef(TPtrRef&& rvalue) NOEXCEPT : TPtrRef() {
        std::swap(Ptr, rvalue.Ptr);
    }
    CONSTEXPR TPtrRef& operator =(TPtrRef&& rvalue) NOEXCEPT {
        Ptr = rvalue.Ptr;
        rvalue.Ptr = nullptr;
        return (*this);
    }

    CONSTEXPR bool valid() const { return (nullptr != Ptr); }

    CONSTEXPR T* get() const NOEXCEPT { return Ptr; }
    CONSTEXPR T** ref() NOEXCEPT { return std::addressof(Ptr); }

    CONSTEXPR T& operator * () const { Assert(Ptr); return (*Ptr); }
    CONSTEXPR T* operator ->() const { Assert(Ptr); return Ptr; }

    CONSTEXPR operator T* () const NOEXCEPT { return Ptr; }
    CONSTEXPR operator T& () const NOEXCEPT { return (*Ptr); }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr == rhs.Ptr); }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, std::nullptr_t) { return (lhs.Ptr == nullptr); }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, std::nullptr_t) { return (not operator ==(lhs, nullptr)); }

    CONSTEXPR friend bool operator ==(std::nullptr_t, const TPtrRef& rhs) { return (rhs.Ptr == nullptr); }
    CONSTEXPR friend bool operator !=(std::nullptr_t, const TPtrRef& rhs) { return (not operator ==(nullptr, rhs)); }

    CONSTEXPR friend bool operator < (const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr <  rhs.Ptr); }
    CONSTEXPR friend bool operator >=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator < (lhs, rhs)); }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const T& rhs) { return (*lhs == rhs); }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const T& rhs) { return (not operator ==(lhs, rhs)); }

    CONSTEXPR friend bool operator ==(const T& lhs, const TPtrRef& rhs) { return (lhs == *rhs); }
    CONSTEXPR friend bool operator !=(const T& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    friend void swap(TPtrRef& lhs, TPtrRef& rhs) NOEXCEPT {
        std::swap(lhs.Ptr, rhs.Ptr);
    }

    friend hash_t hash_value(const TPtrRef& ref) NOEXCEPT {
        return hash_as_pod(ref.Ptr);
    }
};
//----------------------------------------------------------------------------
template <>
struct TPtrRef<void> {
    void* Ptr;

    TPtrRef() = default;

    CONSTEXPR TPtrRef(Meta::FForceInit) NOEXCEPT : Ptr(nullptr) {}

    CONSTEXPR TPtrRef(void* ptr) NOEXCEPT : Ptr(ptr) { Assert(ptr); }

    CONSTEXPR TPtrRef(const TPtrRef&) NOEXCEPT = default;
    CONSTEXPR TPtrRef& operator =(const TPtrRef&) NOEXCEPT = default;

    CONSTEXPR TPtrRef(TPtrRef&& rvalue) NOEXCEPT : Ptr(rvalue.Ptr) {
        rvalue.Ptr = nullptr;
    }
    CONSTEXPR TPtrRef& operator =(TPtrRef&& rvalue) NOEXCEPT {
        Ptr = rvalue.Ptr;
        rvalue.Ptr = nullptr;
        return (*this);
    }

    CONSTEXPR bool valid() const { return (nullptr != Ptr); }

    CONSTEXPR void* get() const NOEXCEPT { return Ptr; }

    CONSTEXPR void* operator ->() const { Assert(Ptr); return Ptr; }

    CONSTEXPR operator void* () const NOEXCEPT { return Ptr; }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr == rhs.Ptr); }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    CONSTEXPR friend bool operator ==(const TPtrRef& lhs, std::nullptr_t) { return (lhs.Ptr == nullptr); }
    CONSTEXPR friend bool operator !=(const TPtrRef& lhs, std::nullptr_t) { return (not operator ==(lhs, nullptr)); }

    CONSTEXPR friend bool operator ==(std::nullptr_t, const TPtrRef& rhs) { return (rhs.Ptr == nullptr); }
    CONSTEXPR friend bool operator !=(std::nullptr_t, const TPtrRef& rhs) { return (not operator ==(nullptr, rhs)); }

    CONSTEXPR friend bool operator < (const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr <  rhs.Ptr); }
    CONSTEXPR friend bool operator >=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator < (lhs, rhs)); }

    friend void swap(TPtrRef& lhs, TPtrRef& rhs) NOEXCEPT {
        std::swap(lhs.Ptr, rhs.Ptr);
    }

    friend hash_t hash_value(const TPtrRef& ref) NOEXCEPT {
        return hash_as_pod(ref.Ptr);
    }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TPtrRef<T>, typename T)
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TPtrRef<T> MakePtrRef(T& ref) {
    return TPtrRef{ &ref };
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR TPtrRef<T> MakePtrRef(T* ptr) {
    return TPtrRef{ ptr };
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T& DerefPtr(T& ref) {
    return ref;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T& DerefPtr(T* ptr) {
    return *ptr;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T& DerefPtr(TPtrRef<T> ptrRef) {
    return *ptrRef;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
