#pragma once

#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
namespace details {
template <typename _Lhs, typename _Rhs, typename _Result = decltype(std::declval<_Lhs>() == std::declval<_Rhs>()) >
std::is_integral<_Result> has_trivial_equal_(int);
template <typename _Lhs, typename _Rhs>
std::false_type has_trivial_equal_(...);
template <typename _Lhs, typename _Rhs, typename _Result = decltype(std::declval<_Lhs>() < std::declval<_Rhs>()) >
std::is_integral<_Result> has_trivial_less_(int);
template <typename _Lhs, typename _Rhs>
std::false_type has_trivial_less_(...);
} //!details
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_equal_t = decltype(details::has_trivial_equal_<_Lhs, _Rhs>(0));
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_equal_v = has_trivial_equal_t<_Lhs, _Rhs>::value;
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_less_t = decltype(details::has_trivial_less_<_Lhs, _Rhs>(0));
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_less_v = has_trivial_less_t<_Lhs, _Rhs>::value;
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_compare_v = (
    has_trivial_less_v<_Lhs, _Rhs> &&
    has_trivial_less_v<_Lhs, _Rhs> );
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_compare_t = std::bool_constant<has_trivial_compare_v<_Lhs, _Rhs>>;
//----------------------------------------------------------------------------
template <typename T>
struct TComparable {
    friend CONSTEXPR bool operator ==(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) == 0);
    }
    friend CONSTEXPR bool operator !=(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) != 0);
    }
    friend CONSTEXPR bool operator <(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) < 0);
    }
    friend CONSTEXPR bool operator <=(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) <= 0);
    }
    friend CONSTEXPR bool operator >(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) > 0);
    }
    friend CONSTEXPR bool operator >=(const T& lhs, const T& rhs) NOEXCEPT {
        return (lhs.Compare(rhs) >= 0);
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TCompareTo {
    TEnableIf<has_trivial_compare_v<T>, int> operator ()(const T& lhs, const T& rhs) const NOEXCEPT {
        return (lhs < rhs ? -1 : lhs == rhs ? 0 : 1);
    }
};
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = Meta::TEnableIf<Meta::has_trivial_less_v<_Lhs, _Rhs>> >
CONSTEXPR auto Max(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? b : a);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = Meta::TEnableIf<Meta::has_trivial_less_v<_Lhs, _Rhs>> >
CONSTEXPR auto Min(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? a : b);
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = Meta::TEnableIf<Meta::has_trivial_less_v<_A, _B> && Meta::has_trivial_less_v<_B, _C>> >
CONSTEXPR auto Max3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Max(a, Max(b, c));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = Meta::TEnableIf<Meta::has_trivial_less_v<_A, _B> && Meta::has_trivial_less_v<_B, _C>> >
CONSTEXPR auto Min3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Min(a, Min(b, c));
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = Meta::TEnableIf<Meta::has_trivial_compare_v<_Lhs, _Rhs>> >
CONSTEXPR int Compare(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? -1 : a == b ? 0 : 1);
}
//----------------------------------------------------------------------------
template <typename T, typename U, class = Meta::TEnableIf<Meta::has_trivial_less_v<T, U>> >
CONSTEXPR auto Clamp(const T& value, const U& vmin, const U& vmax) NOEXCEPT {
    return Min(vmax, Max(vmin, value));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
