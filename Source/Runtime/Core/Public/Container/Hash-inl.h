#pragma once

#include "Container/Hash.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
CONSTEXPR void hash_combine_impl_(hash_t& seed, const T& value) NOEXCEPT {
    return hash_combine(seed, value);
}
template <typename _Arg0, typename _Arg1, typename... _Args>
CONSTEXPR void hash_combine_impl_(hash_t& seed, const _Arg0& arg0, const _Arg1& arg1, const _Args&... args) NOEXCEPT {
    hash_combine(seed, arg0);
    return hash_combine_impl_(seed, arg1, args...);
}
} //!details
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void hash_combine(hash_t& seed, const T& value) NOEXCEPT {
    using PPE::hash_value;
    seed._value = FPlatformHash::HashCombine(seed, hash_value(value));
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename _Arg1, typename... _Args>
CONSTEXPR void hash_combine(hash_t& seed, const _Arg0& arg0, const _Arg1& arg1, const _Args&... args) NOEXCEPT {
    hash_combine(seed, 2/* arg0+arg1 */+ sizeof...(args));
    details::hash_combine_impl_(seed, arg0, arg1, args...);
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
    IF_CONSTEXPR(Meta::is_random_access_iterator_v<_It>) {
        Assert(first <= last);
        if (Likely(first != last)) {
            const size_t count = std::distance(first, last);
            Assert_NoAssume(count && (std::addressof(last[-1]) - std::addressof(*first)) == static_cast<ptrdiff_t>(count) - 1);
            hash_range(seed, std::addressof(*first), count);
        }
    }
    else {
        size_t count = 0;
        for (; first != last; ++first, ++count)
            hash_combine(seed, *first);
        hash_combine(seed, count); // post-fix for non-random access iterators, better than nothing ?
    }
    return seed;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR void hash_range(hash_t& seed, const T* values, size_t count) NOEXCEPT {
    hash_combine(seed, count);
    forrange(p, values, values + count)
        hash_combine(seed, *p);
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR hash_t hash_range(const T* values, size_t count) NOEXCEPT {
    hash_t seed{ PPE_HASH_VALUE_SEED };
    hash_range(seed, values, count);
    return seed;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR hash_t hash_view(const TMemoryView<T>& view) NOEXCEPT {
    return hash_range(view.data(), view.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
