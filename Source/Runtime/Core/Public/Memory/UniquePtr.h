#pragma once

#include "Core_fwd.h"

#include "Allocator/TrackingMalloc.h"

#include <memory>

#define _FWD_UNIQUEPTR_IMPL(T, _PREFIX)                                     \
    class CONCAT(_PREFIX, T);                                               \
    typedef ::PPE::TUniquePtr<CONCAT(_PREFIX, T)>          CONCAT(U,  T);   \
    typedef ::PPE::TUniquePtr<const CONCAT(_PREFIX, T)>    CONCAT(UC, T)

#define FWD_UNIQUEPTR(T_WITHOUT_F)              _FWD_UNIQUEPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTEFARCE_UNIQUEPTR(T_WITHOUT_I)    _FWD_UNIQUEPTR_IMPL(T_WITHOUT_I, I)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TUniquePtr : Meta::FNonCopyable {
public:
    typedef Meta::TRemoveConst<T> value_type;

    template <typename U>
    friend class TUniquePtr;

    template <typename U = value_type, typename... _Args>
    NODISCARD static Meta::TEnableIf<std::is_base_of_v<T, U>, U*> New(_Args&&... args)
        requires (std::is_constructible_v<U, _Args&&...>) {
        return TRACKING_NEW(Unique, U)(std::forward<_Args>(args)...);
    }

    CONSTEXPR TUniquePtr() NOEXCEPT = default;

    ~TUniquePtr() NOEXCEPT {
        reset();
    }

    // should use reset(), MakeUnique(), or New<>()
    CONSTEXPR explicit TUniquePtr(TPtrRef<T> ptr) NOEXCEPT
    :   _ptr(ptr)
    {}

    template <typename U>
    CONSTEXPR TUniquePtr(TUniquePtr<U>&& rvalue) NOEXCEPT
    :   _ptr(rvalue._ptr) {
        rvalue._ptr = nullptr;
    }

    template <typename U>
    TUniquePtr& operator =(TUniquePtr<U>&& rvalue) NOEXCEPT {
        reset();

        _ptr = rvalue._ptr;
        rvalue._ptr = nullptr;

        return (*this);
    }

    template <typename... _Args>
    TUniquePtr(std::piecewise_construct_t, _Args&&... args)
        requires (std::is_constructible_v<T, _Args&&...>)
        : _ptr(New(std::forward<_Args>(args)...))
    {}

    CONSTEXPR T& operator *() const NOEXCEPT { Assert(_ptr); return (*_ptr); }
    CONSTEXPR T* operator ->() const NOEXCEPT { Assert(_ptr); return _ptr; }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!_ptr); }
    NODISCARD bool valid() const { return (!!_ptr); }

    CONSTEXPR T* get() const NOEXCEPT { return _ptr; }

    using deleter_f = void (*)(value_type*) NOEXCEPT;
    static deleter_f Deleter() { return &tracking_delete<value_type>; }

    void reset() NOEXCEPT {
        if (_ptr) {
            Deleter()(_ptr);
            _ptr = nullptr;
        }
    }

    template <typename U = value_type, typename... _Args>
    U* create(_Args&&... args)
        requires (std::is_constructible_v<U, _Args&&...>) {
        reset();

        const Meta::TAddPointer<U> result =
            New<U>(std::forward<_Args>(args)...);

        _ptr = result;
        return result;
    }

    template <typename U>
    NODISCARD U *as() const { return checked_cast<U*>(get()); }

    template <typename U>
    void Swap(TUniquePtr<U>& other) NOEXCEPT {
        std::swap(_ptr, other._ptr);
    }

    NODISCARD friend hash_t hash_value(const TUniquePtr& uniq) NOEXCEPT {
        return hash_ptr(uniq.get());
    }

    friend void swap(TUniquePtr& lhs, TUniquePtr& rhs) NOEXCEPT {
        std::swap(lhs._ptr, rhs._ptr);
    }

    template <typename U>
    NODISCARD friend bool operator ==(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr == rhs._ptr);
    }
    template <typename U>
    NODISCARD friend bool operator !=(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr != rhs._ptr);
    }
    template <typename U>
    NODISCARD friend bool operator < (const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr < rhs._ptr);
    }
    template <typename U>
    NODISCARD friend bool operator >=(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr >= rhs._ptr);
    }

private:
    value_type* _ptr{ nullptr };
};
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
NODISCARD TUniquePtr<T> MakeUnique(_Args&&... args)
    requires (std::is_constructible_v<T, _Args&&...>) {
    return { std::piecewise_construct, std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POINTER(TUniquePtr<T>, typename T)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
