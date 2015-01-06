#pragma once

#include "Core/Container/Hash.h"

#include <xhash>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE void _individual_hash_value(size_t * ) {}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
FORCE_INLINE void _individual_hash_value(size_t *outH, const _Arg0& arg0, const _Args&... args) {
    *outH = hash_value(arg0);
    _individual_hash_value(outH + 1, args...);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Arg1, typename... _Args>
inline size_t hash_value(const _Arg0& arg0, const _Arg1& arg1, const _Args&... args) {

    STATIC_CONST_INTEGRAL(size_t, count, Meta::CountOfVariadicArgs<_Arg0 COMMA _Arg1 COMMA _Args...>::value);
    STATIC_ASSERT(count >= 2);

#ifdef ARCH_X64
    static const size_t _FNV_offset_basis = 14695981039346656037ULL;
    static const size_t _FNV_prime = 1099511628211ULL;
#else
    static const size_t _FNV_offset_basis = 2166136261U;
    static const size_t _FNV_prime = 16777619U;
#endif

    size_t individual_h[count];
    individual_h[0] = hash_value(arg0);
    _individual_hash_value(&individual_h[1], arg1, args...);

    size_t h = individual_h[0];
    for (size_t i = 1; i < count; ++i)
        h = ((_FNV_offset_basis + h) ^ individual_h[i] ) * _FNV_prime;

#ifdef ARCH_X64
    h ^= h >> 32;
#endif

    return h;
}
//----------------------------------------------------------------------------
template <typename _It>
size_t hash_value_seq(const _It& begin, const _It& end) {
    if (end == begin)
        return 0;

#ifdef ARCH_X64
    static const size_t _FNV_offset_basis = 14695981039346656037ULL;
    static const size_t _FNV_prime = 1099511628211ULL;
#else
    static const size_t _FNV_offset_basis = 2166136261U;
    static const size_t _FNV_prime = 16777619U;
#endif

    _It it = begin;
    size_t h = hash_value(*it);

    for (++it; it != end; ++it)
        h = ((_FNV_offset_basis + h) ^ hash_value(*it)) * _FNV_prime;

#ifdef ARCH_X64
    h ^= h >> 32;
#endif

    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
