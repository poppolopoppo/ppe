#pragma once

#include "Meta/Assert.h"
#include "Meta/TypeTraits.h"

#if PPE_HAS_CXX17
#   include <optional>
#endif

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TODO: use C++17 std::optional when available
#if PPE_HAS_CXX17
template <typename T>
using TOptional = std::optional<T>;
#else
template <typename T, typename = TEnableIf< is_pod_v<T> > >
struct TOptional {
    TOptional() = default;

    TOptional(const TOptional&) = default;
    TOptional& operator =(const TOptional&) = default;

    TOptional(TOptional&&) = default;
    TOptional& operator =(TOptional&&) = default;

    TOptional(T value) { operator =(value); }
    TOptional& operator =(T value) { emplace(value); return (*this); }

    T operator *() const { Assert(_hasValue); return *data_(); }
    const T* operator ->() const { Assert(_hasValue); return data_(); }

    explicit operator bool() const { return _hasValue; }

    bool has_value() const { return _hasValue; }
    void reset() { _hasValue = false; }
    void emplace(T value) {
        _hasValue = true;
        *data_() = value;
    }

    void swap(TOptional& other) {
        std::swap(_hasValue);
        std::swap(*data_(), *other.data_());
    }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type _data;
    bool _hasValue = false;

    FORCE_INLINE T* data_() { return reinterpret_cast<T*>(&_data); }
    FORCE_INLINE const T* data_() const { return reinterpret_cast<const T*>(&_data); }
};
#endif
//----------------------------------------------------------------------------
template <typename T>
TOptional<T> MakeOptional(T&& value) NOEXCEPT {
    return std::make_optional(std::move(value));
}
//----------------------------------------------------------------------------
template <typename T>
T* MakePtrRef(TOptional<T>& optional) NOEXCEPT {
    return (optional.has_value() ? std::addressof(optional.value()) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
const T* MakePtrRef(const TOptional<T>& optional) NOEXCEPT {
    return (optional.has_value() ? std::addressof(optional.value()) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const TOptional<T>& optional) {
    return (optional.has_value()
        ? hash_value(*optional)
        : hash_t{} );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE

#if PPE_HAS_CXX17
namespace std {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// For ADL:
//----------------------------------------------------------------------------
template <typename T>
auto hash_value(const std::optional<T>& optional) {
    return PPE::Meta::hash_value(optional);
}
//----------------------------------------------------------------------------
template <typename T>
auto MakePtrRef(std::optional<T>& optional) NOEXCEPT {
    return PPE::Meta::MakePtrRef(optional);
}
//----------------------------------------------------------------------------
template <typename T>
auto MakePtrRef(const std::optional<T>& optional) NOEXCEPT {
    return PPE::Meta::MakePtrRef(optional);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace std
#endif
