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
typename std::enable_if<std::is_enum<T>::value, hash_t>::type hash_value(T&& value) {
    return hash_as_pod(std::forward<T>(value));
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
// Enables ADL/KL in Core namespace
//----------------------------------------------------------------------------
template <typename T>
struct Hash : public std::unary_function<const T&, size_t> {
    hash_t operator ()(const T& value) const {
        using Core::hash_value;
        return hash_value(value);
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct EqualTo : public std::equal_to<T> {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Hash-inl.h"
