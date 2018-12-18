#pragma once

#include "Core.h"

#include "HAL/PlatformHash.h"
#include "Memory/HashFunctions.h"
#include "Meta/Hash_fwd.h"
#include "Meta/TypeTraits.h"

#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE hash_t hash_value(bool v)  { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(i8 v)    { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(u8 v)    { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(i16 v)   { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(u16 v)   { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(i32 v)   { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(u32 v)   { return hash_uint(size_t(v)); }
FORCE_INLINE hash_t hash_value(i64 v)   { return hash_uint(u64(v)); }
FORCE_INLINE hash_t hash_value(u64 v)   { return hash_uint(v); }
FORCE_INLINE hash_t hash_value(u128 v)  { return hash_uint(v); }
FORCE_INLINE hash_t hash_value(u256 v)  { return hash_as_pod(v); }
FORCE_INLINE hash_t hash_value(float v) { return hash_uint(*(u32*)&v); }
FORCE_INLINE hash_t hash_value(double v){ return hash_uint(*(u64*)&v); }
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE hash_t hash_value(const T *ptr) {
    return hash_ptr(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE auto hash_value(T value)
    -> typename std::enable_if<std::is_enum<T>::value, hash_t>::type {
    return hash_as_pod(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helpers for combining hash values:
//----------------------------------------------------------------------------
template <typename T>
void hash_combine(hash_t& seed, const T& value);
//----------------------------------------------------------------------------
template <typename T>
void hash_range(hash_t& seed, const T *values, size_t count);
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_range(const T *values, size_t count);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
void hash_combine(hash_t& seed, const _Arg0& arg0, const _Args&... args);
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
hash_t hash_tuple(_Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _It>
hash_t hash_range(_It first, _It last);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
template <typename T>
struct THash {
    hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_value;
        return hash_value(value);
    }
};
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
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
    Meta::has_default_constructor<T>::value> > {
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
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Hash = Meta::THash<T>, typename _Equal = Meta::TEqualTo<T> >
class THashMemoizer {
public:
    THashMemoizer() = default;

    THashMemoizer(const T& value)
        : _value(value)
        , _hash(_Hash()(_value)) {}
    THashMemoizer(T&& rvalue)
        : _value(std::move(rvalue))
        , _hash(_Hash()(_value)) {}

    THashMemoizer(const THashMemoizer& other)
        : _value(other._value)
        , _hash(other._hash) {}
    THashMemoizer& operator =(const THashMemoizer& other) {
        _value = other._value;
        _hash = other._hash;
        return *this;
    }

    THashMemoizer(THashMemoizer&& rvalue) noexcept
        : _value(std::move(rvalue._value))
        , _hash(rvalue._hash) {}
    THashMemoizer& operator =(THashMemoizer&& rvalue) noexcept {
        _value = std::move(rvalue._value);
        _hash = rvalue._hash;
        return *this;
    }

    const T& Value() const { return _value; }
    hash_t Hash() const { return _hash; }

    operator T& () { return _value; }
    operator const T& () const { return _value; }

    void Swap(THashMemoizer& other) {
        std::swap(_value, other._value);
        std::swap(_hash, other._hash);
    }

    friend hash_t hash_value(const THashMemoizer& value) { return value._hash;  }
    friend void swap(THashMemoizer& lhs, THashMemoizer& rhs) { lhs.Swap(rhs); }

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/Hash-inl.h"
