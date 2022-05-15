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
    static Meta::TEnableIf<std::is_base_of_v<T, U>, U*> New(_Args&&... args) {
        return TRACKING_NEW(Unique, U)(std::forward<_Args>(args)...);
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
    template <typename U>
    TUniquePtr& operator =(TUniquePtr<U>&& rvalue) NOEXCEPT {
        reset();

        _ptr = rvalue._ptr;
        rvalue._ptr = nullptr;

        return (*this);
    }

    ~TUniquePtr() NOEXCEPT {
        reset();
    }

    CONSTEXPR T& operator *() const NOEXCEPT { Assert(_ptr); return (*_ptr); }
    CONSTEXPR T* operator ->() const NOEXCEPT { Assert(_ptr); return _ptr; }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!_ptr); }
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
    Meta::TAddPointer<U> create(_Args&&... args) {
        reset();

        const Meta::TAddPointer<U> result =
            New<U>(std::forward<_Args>(args)...);

        _ptr = result;
        return result;
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

    friend void swap(TUniquePtr& lhs, TUniquePtr& rhs) NOEXCEPT {
        std::swap(lhs._ptr, rhs._ptr);
    }

    template <typename U>
    friend bool operator ==(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr == rhs._ptr);
    }
    template <typename U>
    friend bool operator !=(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr != rhs._ptr);
    }
    template <typename U>
    friend bool operator < (const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr < rhs._ptr);
    }
    template <typename U>
    friend bool operator >=(const TUniquePtr& lhs, const TUniquePtr<U>& rhs) NOEXCEPT {
        return (lhs._ptr >= rhs._ptr);
    }

private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
TUniquePtr<T> MakeUnique(_Args&&... args) {
    using FPointer = typename TUniquePtr<T>::FPointer;
    const FPointer ptr{ TUniquePtr<T>::New(std::forward<_Args>(args)...) };
    return TUniquePtr<T>(ptr);
}
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POINTER(TUniquePtr<T>, typename T)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
