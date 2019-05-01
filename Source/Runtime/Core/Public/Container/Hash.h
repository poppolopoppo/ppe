#pragma once

#include "Core_fwd.h"

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
    CONSTEXPR hash_t operator ()(const T& value) const NOEXCEPT {
        using PPE::hash_value;
        return hash_value(value);
    }
};
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/Hash-inl.h"
