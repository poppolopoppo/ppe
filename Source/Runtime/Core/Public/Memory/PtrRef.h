#pragma once

#include "Core_fwd.h"

#include "Meta/Hash_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wraps a T& inside a T* to avoid copying T when using value semantics
//----------------------------------------------------------------------------
template <typename T>
struct TPtrRef {
    T* Ptr{ nullptr };

    TPtrRef() = default;

    CONSTEXPR TPtrRef(Meta::FForceInit) NOEXCEPT : Ptr(nullptr) {}
    CONSTEXPR TPtrRef(std::nullptr_t null) NOEXCEPT : Ptr(null) {}

    CONSTEXPR TPtrRef(T* ptr) NOEXCEPT : Ptr(ptr) { Assert(ptr); }
    CONSTEXPR TPtrRef& operator =(T* ptr) NOEXCEPT { Assert(ptr); Ptr = ptr; return (*this); }

    CONSTEXPR TPtrRef(T& ref) NOEXCEPT : Ptr(std::addressof(ref)) {}
    CONSTEXPR TPtrRef& operator =(T& ref) NOEXCEPT { Ptr = std::addressof(ref); return (*this); }

    template <typename U, Meta::TEnableIf<std::is_assignable_v<T*&, U*>>* = nullptr>
    CONSTEXPR TPtrRef(const TPtrRef<U>& other)
        : Ptr(other.Ptr)
    {}

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

    NODISCARD CONSTEXPR bool valid() const { return (nullptr != Ptr); }

    NODISCARD CONSTEXPR T* get() const NOEXCEPT { return Ptr; }
    NODISCARD CONSTEXPR T** ref() NOEXCEPT { return std::addressof(Ptr); }

    CONSTEXPR void reset() NOEXCEPT {
        Ptr = nullptr;
    }

    NODISCARD CONSTEXPR T& operator * () const { Assert(Ptr); return (*Ptr); }
    NODISCARD CONSTEXPR T* operator ->() const { Assert(Ptr); return Ptr; }

    NODISCARD CONSTEXPR operator T& () const NOEXCEPT { Assert(Ptr); return (*Ptr); }
    NODISCARD CONSTEXPR operator T* () const NOEXCEPT { return Ptr; }

    // can forward operator () calls to inner ptr if T defines it
    template <typename... _Args, decltype(std::declval<T&>()(std::forward<_Args>(std::declval<_Args&&>())...))* = nullptr>
    CONSTEXPR auto operator ()(_Args&& ...args) const {
        return (*Ptr)(std::forward<_Args>(args)...);
    }

    NODISCARD CONSTEXPR TPtrRef& operator++() {
        ++Ptr;
        return (*this);
    }
    NODISCARD CONSTEXPR TPtrRef operator++(int) {
        TPtrRef tmp(*this);
        ++(*this);
        return tmp;
    }

    NODISCARD CONSTEXPR TPtrRef& operator--() {
        --Ptr;
        return (*this);
    }
    NODISCARD CONSTEXPR TPtrRef operator--(int) {
        TPtrRef tmp(*this);
        --(*this);
        return tmp;
    }

    NODISCARD CONSTEXPR TPtrRef& operator+=(ptrdiff_t offset) {
        Ptr += offset;
        return (*this);
    }
    NODISCARD CONSTEXPR TPtrRef operator+(ptrdiff_t offset) const {
        TPtrRef tmp(*this);
        return (tmp += offset);
    }

    NODISCARD CONSTEXPR TPtrRef& operator-=(ptrdiff_t offset) {
        Ptr -= offset;
        return (*this);
    }
    NODISCARD CONSTEXPR TPtrRef operator-(ptrdiff_t offset) const {
        TPtrRef tmp(*this);
        return (tmp -= offset);
    }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr == rhs.Ptr); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, std::nullptr_t) { return (lhs.Ptr == nullptr); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, std::nullptr_t) { return (not operator ==(lhs, nullptr)); }

    NODISCARD CONSTEXPR friend bool operator ==(std::nullptr_t, const TPtrRef& rhs) { return (rhs.Ptr == nullptr); }
    NODISCARD CONSTEXPR friend bool operator !=(std::nullptr_t, const TPtrRef& rhs) { return (not operator ==(nullptr, rhs)); }

    NODISCARD CONSTEXPR friend bool operator < (const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr <  rhs.Ptr); }
    NODISCARD CONSTEXPR friend bool operator >=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator < (lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const T& rhs) { return (*lhs == rhs); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const T& rhs) { return (not operator ==(lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const T& lhs, const TPtrRef& rhs) { return (lhs == *rhs); }
    NODISCARD CONSTEXPR friend bool operator !=(const T& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const T* rhs) { return (lhs.get() == rhs); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const T* rhs) { return (not operator ==(lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const T* lhs, const TPtrRef& rhs) { return (lhs == rhs.get()); }
    NODISCARD CONSTEXPR friend bool operator !=(const T* lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    friend void swap(TPtrRef& lhs, TPtrRef& rhs) NOEXCEPT {
        std::swap(lhs.Ptr, rhs.Ptr);
    }

    NODISCARD friend hash_t hash_value(const TPtrRef& ref) NOEXCEPT {
        return hash_as_pod(ref.Ptr);
    }
};
//----------------------------------------------------------------------------
template <>
struct TPtrRef<void> {
    void* Ptr;

    TPtrRef() = default;

    CONSTEXPR TPtrRef(Meta::FForceInit) NOEXCEPT : Ptr(nullptr) {}
    CONSTEXPR TPtrRef(Meta::FDefaultValue) NOEXCEPT : Ptr(nullptr) {}

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

    NODISCARD CONSTEXPR bool valid() const { return (nullptr != Ptr); }

    NODISCARD CONSTEXPR void* get() const NOEXCEPT { return Ptr; }

    NODISCARD CONSTEXPR void* operator ->() const { Assert(Ptr); return Ptr; }

    NODISCARD CONSTEXPR operator void* () const NOEXCEPT { return Ptr; }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr == rhs.Ptr); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator ==(lhs, rhs)); }

    NODISCARD CONSTEXPR friend bool operator ==(const TPtrRef& lhs, std::nullptr_t) { return (lhs.Ptr == nullptr); }
    NODISCARD CONSTEXPR friend bool operator !=(const TPtrRef& lhs, std::nullptr_t) { return (not operator ==(lhs, nullptr)); }

    NODISCARD CONSTEXPR friend bool operator ==(std::nullptr_t, const TPtrRef& rhs) { return (rhs.Ptr == nullptr); }
    NODISCARD CONSTEXPR friend bool operator !=(std::nullptr_t, const TPtrRef& rhs) { return (not operator ==(nullptr, rhs)); }

    NODISCARD CONSTEXPR friend bool operator < (const TPtrRef& lhs, const TPtrRef& rhs) { return (lhs.Ptr <  rhs.Ptr); }
    NODISCARD CONSTEXPR friend bool operator >=(const TPtrRef& lhs, const TPtrRef& rhs) { return (not operator < (lhs, rhs)); }

    friend void swap(TPtrRef& lhs, TPtrRef& rhs) NOEXCEPT {
        std::swap(lhs.Ptr, rhs.Ptr);
    }

    NODISCARD friend hash_t hash_value(const TPtrRef& ref) NOEXCEPT {
        return hash_as_pod(ref.Ptr);
    }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TPtrRef<T>, typename T)
PPE_ASSUME_TEMPLATE_AS_POINTER(TPtrRef<T>, typename T)
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TPtrRef<T> MakePtrRef(T& ref) {
    return TPtrRef{ &ref };
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR TPtrRef<T> MakePtrRef(T* ptr) {
    return TPtrRef{ ptr };
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TPtrRef<T>& ref) {
    return oss << (*ref);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
