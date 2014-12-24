#pragma once

#include "Core/Container/Hash.h"

#include <xhash>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value(const T& value) {
    return std::hash<T>()(value);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
size_t hash_value(T(&staticArray)[_Dim]) {
    return hash_value_seq(&staticArray[0], &staticArray[_Dim]);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
inline size_t hash_value(const _Arg0& arg0, const _Args&... args) {
    using Core::hash_value;
#ifdef ARCH_X64
    const size_t _FNV_offset_basis = 14695981039346656037ULL;
    const size_t _FNV_prime = 1099511628211ULL;
#else
    const size_t _FNV_offset_basis = 2166136261U;
    const size_t _FNV_prime = 16777619U;
#endif
    size_t h = ((_FNV_offset_basis + hash_value(arg0)) ^ hash_value(args...)) * _FNV_prime;
#ifdef ARCH_X64
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    h ^= h >> 32;
#else
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
#endif
    return h;
}
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value_type(const T& value, typename std::enable_if<sizeof(T) <= sizeof(size_t)>::type* /* = 0 */) {
    static_assert(std::is_pod<T>::value, "T must be a POD type");
    const union { T value; size_t hash; } u = { value };
    return u.hash;
}
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value_type(const T& value, typename std::enable_if<(sizeof(T) > sizeof(size_t)) && (sizeof(T) <= 2 * sizeof(size_t))> ::type* /* = 0 */) {
    static_assert(std::is_pod<T>::value, "T must be a POD type");
    const union { T value; size_t hash[2]; } u = { value };
    return hash_value(u.hash[0], u.hash[1]);
}
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_value_seq(const _It& begin, const _It& end) {
    using Core::hash_value;
#ifdef ARCH_X64
    const size_t _FNV_offset_basis = 14695981039346656037ULL;
    const size_t _FNV_prime = 1099511628211ULL;
#else
    const size_t _FNV_offset_basis = 2166136261U;
    const size_t _FNV_prime = 16777619U;
#endif
    size_t h = 0;
    for (_It it = begin; it != end; ++it)
        h = ((_FNV_offset_basis + h) ^ hash_value(*it)) * _FNV_prime;
#ifdef ARCH_X64
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    h ^= h >> 32;
#else
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
#endif
    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t hash_value(const std::thread::id& id) {
    return id.hash();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
