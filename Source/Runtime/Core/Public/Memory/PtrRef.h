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
    CONSTEXPR TPtrRef(std::nullptr_t null) NOEXCEPT : Ptr(null) {}

    CONSTEXPR TPtrRef(T* ptr) NOEXCEPT : Ptr(ptr) { Assert(ptr); }
    CONSTEXPR TPtrRef& operator =(T* ptr) NOEXCEPT { Assert(ptr); Ptr = ptr; return (*this); }

    CONSTEXPR TPtrRef(T& ref) NOEXCEPT : Ptr(std::addressof(ref)) {}
    CONSTEXPR TPtrRef& operator =(T& ref) NOEXCEPT { Ptr = std::addressof(ref); return (*this); }

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

    CONSTEXPR T* get() const NOEXCEPT { return Ptr; }
    CONSTEXPR T** ref() NOEXCEPT { return std::addressof(Ptr); }

    CONSTEXPR T& operator * () const { Assert(Ptr); return (*Ptr); }
    CONSTEXPR T* operator ->() const { Assert(Ptr); return Ptr; }

    CONSTEXPR operator T& () const NOEXCEPT { Assert(Ptr); return (*Ptr); }
    CONSTEXPR operator T* () const NOEXCEPT { return Ptr; }

    CONSTEXPR TPtrRef& operator++() {
        ++Ptr;
        return (*this);
    }
    CONSTEXPR TPtrRef operator++(int) {
        TPtrRef tmp(*this);
        ++(*this);
        return tmp;
    }

    CONSTEXPR TPtrRef& operator--() {
        --Ptr;
        return (*this);
    }
    CONSTEXPR TPtrRef operator--(int) {
        TPtrRef tmp(*this);
        --(*this);
        return tmp;
    }

    CONSTEXPR TPtrRef& operator+=(ptrdiff_t offset) {
        Ptr += offset;
        return (*this);
    }
    CONSTEXPR TPtrRef operator+(ptrdiff_t offset) const {
        TPtrRef tmp(*this);
        return (tmp += offset);
    }

    CONSTEXPR TPtrRef& operator-=(ptrdiff_t offset) {
        Ptr -= offset;
        return (*this);
    }
    CONSTEXPR TPtrRef operator-(ptrdiff_t offset) const {
        TPtrRef tmp(*this);
        return (tmp -= offset);
    }

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
PPE_ASSUME_TEMPLATE_AS_POINTER(TPtrRef<T>, typename T)
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
