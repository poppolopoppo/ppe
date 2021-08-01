#pragma once

#include "Meta/Aliases.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Constexpr quicksort
//----------------------------------------------------------------------------
template <typename T, typename _Pred>
CONSTEXPR void StaticQuicksort(T* p, int n, const _Pred& pred) NOEXCEPT {
    if (n > 0) {
        using std::swap;

        int m = 0;

        for (int i = 1; i < n; ++i) {
            if (pred(p[i], p[0]))
                swap(p[++m], p[i]);
        }

        swap(p[0], p[m]);

        StaticQuicksort(p, m, pred);
        StaticQuicksort(p + m + 1, n - m - 1, pred);
    }
}
//----------------------------------------------------------------------------
// Constexpr lexicographical compare
//----------------------------------------------------------------------------
template <typename _It, typename _Jt, typename _Less>
constexpr bool LexicographicalCompare(
    _It first1, _It last1,
    _Jt first2, _Jt last2,
    _Less compare ) {
    for (; first2 != last2; ++first1, (void)++first2) {
        if (first1 == last1 || compare(*first1, *first2))
            return true;

        if (compare(*first2, *first1))
            return false;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _It, typename _Jt>
constexpr bool LexicographicalCompare(_It first1, _It last1, _Jt first2, _Jt last2) {
    return LexicographicalCompare(first1, last1, first2, last2,
        TLess<typename std::iterator_traits<_It>::value_type>{} );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
