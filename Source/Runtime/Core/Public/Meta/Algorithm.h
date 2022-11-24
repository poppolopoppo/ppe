#pragma once

#include "Meta/Aliases.h"
#include "Meta/Arithmetic.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Constexpr bubble-sort (works with forward iterators)
//----------------------------------------------------------------------------
template <typename _It, typename _Pred>
CONSTEXPR void BubbleSort(_It first, _It last, _Pred&& pred) {
    auto nth = last;
    for (auto it = first; it != last; ++it) {
        auto kt = first;
        for (auto jt = kt++; kt != nth; jt = kt++) {
            using std::swap;
            if (pred(*kt, *jt))
                swap(*kt, *jt);
        }
    }
}
//----------------------------------------------------------------------------
// Constexpr quick-sort
//----------------------------------------------------------------------------
template <typename T, typename _Pred>
CONSTEXPR void Quicksort(T* p, int n, _Pred&& pred) NOEXCEPT {
    if (n > 0) {
        using std::swap;

        int m = 0;

        for (int i = 1; i < n; ++i) {
            if (pred(p[i], p[0]))
                swap(p[++m], p[i]);
        }

        swap(p[0], p[m]);

        Quicksort(p, m, pred);
        Quicksort(p + m + 1, n - m - 1, pred);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Pred>
CONSTEXPR void Quicksort(T* p, int n) NOEXCEPT {
    Quicksort(p, n, Meta::TLess<T>{});
}
//----------------------------------------------------------------------------
// Binary-search
//----------------------------------------------------------------------------
template <typename _It, typename _Value, typename _Pred>
CONSTEXPR CONSTF _It LowerBound(_It first, _It last, const _Value& value, _Pred pred) NOEXCEPT {
    return std::lower_bound(first, last, value, pred);
}
//----------------------------------------------------------------------------
template <typename _It, typename _Value>
CONSTEXPR CONSTF _It LowerBound(_It first, _It last, const _Value& value) NOEXCEPT {
    return std::lower_bound(first, last, value, Meta::TLess{});
}
//----------------------------------------------------------------------------
// Constexpr lexicographical compare
//----------------------------------------------------------------------------
template <typename _It, typename _Jt, typename _Less>
CONSTEXPR CONSTF bool LexicographicalCompare(
    _It first1, _It last1,
    _Jt first2, _Jt last2,
    _Less compare ) NOEXCEPT {
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
CONSTEXPR CONSTF bool LexicographicalCompare(_It first1, _It last1, _Jt first2, _Jt last2) NOEXCEPT {
    return LexicographicalCompare(first1, last1, first2, last2,
        TLess<typename std::iterator_traits<_It>::value_type>{} );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
