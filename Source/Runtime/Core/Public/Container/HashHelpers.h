#pragma once

#include "Core_fwd.h"

#include "Container/Hash.h"
#include "IO/String_fwd.h"
#include "IO/StringView.h"
#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
//----------------------------------------------------------------------------
// Empty key abstraction for hash-like containers:
//----------------------------------------------------------------------------
struct FEmptyKey {
    CONSTEXPR explicit FEmptyKey(int) {} // avoid promotion/copy
};
CONSTEXPR const FEmptyKey EmptyKey{0};
//----------------------------------------------------------------------------
template <typename T, class = void>
struct TEmptyKey {
    static CONSTEXPR bool is_empty(const T& v) NOEXCEPT = delete;
    static CONSTEXPR void set_empty(T* v) NOEXCEPT = delete;
};
template <typename T>
struct TEmptyKey< T, Meta::TEnableIf<std::is_integral_v<T>> > {
    STATIC_CONST_INTEGRAL(T, value, std::numeric_limits<T>::max());
    static CONSTEXPR bool is_empty(const T& v) NOEXCEPT { return (value == v); }
    static CONSTEXPR void set_empty(T* v) NOEXCEPT { *v = value; }
};
template <typename T>
struct TEmptyKey< T, Meta::TEnableIf<std::is_enum_v<T>> > {
    STATIC_CONST_INTEGRAL(T, value, static_cast<T>(std::numeric_limits<u64>::max()));
    static CONSTEXPR bool is_empty(const T& v) NOEXCEPT { return (value == v); }
    static CONSTEXPR void set_empty(T* v) NOEXCEPT { *v = value; }
};
template <typename T>
struct TEmptyKey< T, Meta::TEnableIf<
    not std::is_enum_v<T> &&
    not std::is_integral_v<T> &&
    has_constructor<T, FEmptyKey>::value >> {
    static CONSTEXPR const T value{ EmptyKey };
    static CONSTEXPR bool is_empty(const T& v) NOEXCEPT { return (v == value); }
    static CONSTEXPR void set_empty(T* v) NOEXCEPT { *v = value; }
};
template <typename T>
struct TEmptyKey< T, Meta::TEnableIf<
    not std::is_enum_v<T> &&
    not std::is_integral_v<T> &&
    not has_constructor<T, FEmptyKey>::value &&
    has_default_constructor<T>::value >> {
    static CONSTEXPR const T value{ Meta::MakeForceInit<T>() };
    static CONSTEXPR bool is_empty(const T& v) NOEXCEPT { return (value == v); }
    static CONSTEXPR void set_empty(T* v) NOEXCEPT { *v = value; }
};
template <>
struct TEmptyKey< u128, void > {
    using empty_u64 = TEmptyKey<u64>;
    static CONSTEXPR const u128 value{ empty_u64::value, empty_u64::value };
    static CONSTEXPR bool is_empty(const u128& v) NOEXCEPT { return (value == v); }
    static CONSTEXPR void set_empty(u128* v) NOEXCEPT { *v = value; }
};
template <>
struct TEmptyKey< u256, void > {
    using empty_u128 = TEmptyKey<u128>;
    static CONSTEXPR const u256 value{ empty_u128::value, empty_u128::value };
    static CONSTEXPR bool is_empty(const u256& v) NOEXCEPT { return (value == v); }
    static CONSTEXPR void set_empty(u256* v) NOEXCEPT { *v = value; }
};
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
using Meta::EmptyKey;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Hash = Meta::THash<T>, typename _Equal = Meta::TEqualTo<T> >
class THashMemoizer {
public:
    THashMemoizer() = default;

    CONSTEXPR THashMemoizer(Meta::FForceInit) NOEXCEPT
        : _value(Meta::MakeForceInit<T>())
        , _hash(Meta::ForceInit)
    {}

    THashMemoizer(const THashMemoizer& other) = default;
    THashMemoizer& operator =(const THashMemoizer& other) = default;

    THashMemoizer(THashMemoizer&& rvalue) = default;
    THashMemoizer& operator =(THashMemoizer&& rvalue) = default;

    THashMemoizer(const T& value, hash_t h) NOEXCEPT
        : _value(value)
        , _hash(h) {}
    THashMemoizer(T&& rvalue, hash_t h) NOEXCEPT
        : _value(std::move(rvalue))
        , _hash(h) {}

    THashMemoizer(const T& value) : THashMemoizer(value, _Hash()(value)) {}
    THashMemoizer(T&& rvalue) : THashMemoizer(std::move(rvalue), _Hash()(rvalue)) {}

    const T& Value() const { return _value; }
    hash_t Hash() const { return _hash; }

    operator T& () { return _value; }
    operator const T& () const { return _value; }

    T& operator *() { return _value; }
    const T& operator *() const { return _value; }

    T* operator ->() { return &_value; }
    const T* operator ->() const { return &_value; }

    void Swap(THashMemoizer& other) NOEXCEPT {
        std::swap(_value, other._value);
        std::swap(_hash, other._hash);
    }

    friend hash_t hash_value(const THashMemoizer& value) NOEXCEPT { return value._hash;  }
    friend void swap(THashMemoizer& lhs, THashMemoizer& rhs) NOEXCEPT { lhs.Swap(rhs); }

    friend inline bool operator ==(const THashMemoizer& lhs, const THashMemoizer& rhs) {
        return (lhs._hash == rhs._hash && _Equal()(lhs._value, rhs._value));
    }
    friend inline bool operator !=(const THashMemoizer& lhs, const THashMemoizer& rhs) {
        return not operator ==(lhs, rhs);
    }

private:
    T _value;
    hash_t _hash;
};
//----------------------------------------------------------------------------
template <typename T, typename _Hash, typename _Equal>
CONSTEXPR bool is_pod_type(THashMemoizer<T, _Hash, _Equal>*) NOEXCEPT {
    return Meta::is_pod_v<T>;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR THashMemoizer<T> Memoize(const T& value) {
    return { value };
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
using TBasicStringViewHashMemoizer = THashMemoizer<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
