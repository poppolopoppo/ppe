#pragma once

#include "Container/Hash.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void hash_combine(hash_t& seed, const T& value) NOEXCEPT {
    using PPE::hash_value;
    seed._value = FPlatformHash::HashCombine(seed, hash_value(value));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void hash_combine_impl_(hash_t& seed, const T& value) NOEXCEPT {
    return hash_combine(seed, value);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Arg1, typename... _Args>
CONSTEXPR void hash_combine_impl_(hash_t& seed, const _Arg0& arg0, const _Arg1& arg1, const _Args&... args) NOEXCEPT {
    hash_combine(seed, arg0);
    return hash_combine_impl_(seed, arg1, args...);
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR void hash_combine(hash_t& seed, const _Arg0& arg0, const _Args&... args) NOEXCEPT {
    details::hash_combine_impl_(seed, arg0, args...);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR hash_t hash_tuple(_Arg0&& arg0, _Args&&... args) NOEXCEPT {
    hash_t seed(PPE_HASH_VALUE_SEED);
    hash_combine(seed, arg0, args...);
    return seed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _It>
CONSTEXPR hash_t hash_range(_It first, _It last) NOEXCEPT {
    hash_t seed(PPE_HASH_VALUE_SEED);
    for (; first != last; ++first)
        hash_combine(seed, *first);
    return seed;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void hash_range(hash_t& seed, const T *values, size_t count) NOEXCEPT {
    forrange(i, 0, count)
        hash_combine(seed, values[i]);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR hash_t hash_range(const T *values, size_t count) NOEXCEPT {
    hash_t seed(PPE_HASH_VALUE_SEED);
    hash_range(seed, values, count);
    return seed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
