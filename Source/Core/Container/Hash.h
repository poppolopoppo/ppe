#pragma once

#include "Core/Core.h"

#include "Core/Memory/HashFunctions.h"
#include "Core/Meta/Hash_fwd.h"

#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define HASH_AS_POD_DEF(_Prefix, _Type) \
    _Prefix hash_t hash_value(_Type value) { return hash_as_pod(value); }
//----------------------------------------------------------------------------
HASH_AS_POD_DEF(inline, i8)
HASH_AS_POD_DEF(inline, u8)
HASH_AS_POD_DEF(inline, i16)
HASH_AS_POD_DEF(inline, u16)
HASH_AS_POD_DEF(inline, i32)
HASH_AS_POD_DEF(inline, u32)
HASH_AS_POD_DEF(inline, i64)
HASH_AS_POD_DEF(inline, u64)
HASH_AS_POD_DEF(inline, u128)
HASH_AS_POD_DEF(inline, u256)
HASH_AS_POD_DEF(inline, float)
HASH_AS_POD_DEF(inline, double)
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if<std::is_enum<T>::value, hash_t>::type hash_value(T value) {
    return hash_as_pod(value);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const T *ptr) {
    return hash_as_pod(intptr_t(ptr));
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
template <typename T>
struct THash : public std::unary_function<const T&, size_t> {
    hash_t operator ()(const T& value) const {
        using Core::hash_value;
        return hash_value(value);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Hash = THash<T>, typename _Equal = Meta::TEqualTo<T> >
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
} //!namespace Core

#include "Core/Container/Hash-inl.h"
