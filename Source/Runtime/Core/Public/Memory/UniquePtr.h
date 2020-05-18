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
    typedef T value_type;

    template <typename U>
    friend class TUniquePtr;

    struct FPointer {
        T* Value;
        CONSTEXPR operator T* () const NOEXCEPT {
            return Value;
        }
    };

    template <typename U = T, typename... _Args>
    static FPointer New(_Args&&... args) {
        return FPointer{ TRACKING_NEW(Unique, U)(std::forward<_Args>(args)...) };
    }

    CONSTEXPR TUniquePtr() NOEXCEPT
    :   _ptr(nullptr)
    {}

    // should use reset(), MakeUnique(), or New<>()
    CONSTEXPR TUniquePtr(FPointer ptr) NOEXCEPT
    :   _ptr(ptr)
    {}

    template <typename U>
    CONSTEXPR TUniquePtr(TUniquePtr<U>&& rvalue) NOEXCEPT
    :   _ptr(rvalue._ptr) {
        rvalue._ptr = nullptr;
    }

    ~TUniquePtr() NOEXCEPT {
        reset();
    }

    template <typename U>
    TUniquePtr& operator =(TUniquePtr<U>&& rvalue) NOEXCEPT {
        reset();

        _ptr = rvalue._ptr;
        rvalue._ptr = nullptr;

        return (*this);
    }

    CONSTEXPR T& operator *() const NOEXCEPT { Assert(_ptr); return (*_ptr); }
    CONSTEXPR T* operator ->() const NOEXCEPT { Assert(_ptr); return _ptr; }

    PPE_FAKEBOOL_OPERATOR_DECL() { return _ptr; }
    bool valid() const { return (!!_ptr); }

    CONSTEXPR T* get() const NOEXCEPT { return _ptr; }

    using deleter_f = void (*)(T*) NOEXCEPT;
    static deleter_f Deleter() { return &tracking_delete<T>; }

    void reset() NOEXCEPT {
        if (_ptr) {
            Deleter()(_ptr);
            _ptr = nullptr;
        }
    }

    template <typename U = T, typename... _Args>
    void reset(_Args&&... args) {
        reset();

        _ptr = New<U>(std::forward<_Args>(args)...);
    }

    template <typename U>
    U *as() const { return checked_cast<U*>(get()); }

    template <typename U>
    void Swap(TUniquePtr<U>& other) NOEXCEPT {
        std::swap(_ptr, other._ptr);
    }

    friend hash_t hash_value(const TUniquePtr& uniq) NOEXCEPT {
        return hash_ptr(uniq.get());
    }

private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
TUniquePtr<T> MakeUnique(_Args&&... args) {
    return { TUniquePtr<T>::New(std::forward<_Args>(args)...) };
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
CONSTEXPR bool operator ==(const TUniquePtr<_Lhs>& lhs, const TUniquePtr<_Rhs>& rhs) NOEXCEPT {
    return (lhs.get() == rhs.get());
}
template <typename _Lhs, typename _Rhs>
CONSTEXPR bool operator !=(const TUniquePtr<_Lhs>& lhs, const TUniquePtr<_Rhs>& rhs) NOEXCEPT {
    return (not operator ==(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
CONSTEXPR bool operator < (const TUniquePtr<_Lhs>& lhs, const TUniquePtr<_Rhs>& rhs) NOEXCEPT {
    return (lhs.get() < rhs.get());
}
template <typename _Lhs, typename _Rhs>
CONSTEXPR bool operator >=(const TUniquePtr<_Lhs>& lhs, const TUniquePtr<_Rhs>& rhs) NOEXCEPT {
    return (not operator <(lhs, rhs));
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(TUniquePtr<_Lhs>& lhs, TUniquePtr<_Rhs>& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
