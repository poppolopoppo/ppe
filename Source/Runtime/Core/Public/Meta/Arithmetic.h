#pragma once

#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Meta {
namespace details {
template <typename _Lhs, typename _Rhs, typename _Result = decltype(std::declval<_Lhs>() < std::declval<_Rhs>()) >
std::is_integral<_Result> has_trivial_less_(int);
template <typename _Lhs, typename _Rhs>
std::false_type has_trivial_less_(...);
} //!details
template <typename _Lhs, typename _Rhs>
using has_trivial_less_t = decltype(details::has_trivial_less_<_Lhs, _Rhs>(0));
template <typename _Lhs, typename _Rhs>
CONSTEXPR bool has_trivial_less_v = has_trivial_less_t<_Lhs, _Rhs>::value;
//----------------------------------------------------------------------------
} //!namespace Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = std::enable_if_t<Meta::has_trivial_less_v<_Lhs, _Rhs>> >
CONSTEXPR auto Max(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? b : a);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = std::enable_if_t<Meta::has_trivial_less_v<_Lhs, _Rhs>> >
CONSTEXPR auto Min(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? a : b);
}
//----------------------------------------------------------------------------
template <typename T, typename U>
CONSTEXPR auto Clamp(const T& value, const U& vmin, const U& vmax) NOEXCEPT {
    return Min(vmax, Max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C>
CONSTEXPR auto Max3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Max(a, Max(b, c));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C>
CONSTEXPR auto Min3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Min(a, Min(b, c));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
