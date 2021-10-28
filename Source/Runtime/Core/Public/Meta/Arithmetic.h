#pragma once

#include "Meta/TypeTraits.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Result = decltype(!!(std::declval<T>()))>
using is_boolean_t_ = std::is_same<TDecay<_Result>, bool>;
template <typename _Lhs, typename _Rhs, typename _Result = decltype(std::declval<TConstReference<_Lhs>>() == std::declval<TConstReference<_Rhs>>()) >
is_boolean_t_<_Result> has_trivial_equal_(int);
template <typename _Lhs, typename _Rhs>
std::false_type has_trivial_equal_(...);
template <typename _Lhs, typename _Rhs, typename _Result = decltype(std::declval<TConstReference<_Lhs>>() < std::declval<TConstReference<_Rhs>>()) >
std::is_same<_Result, bool> has_trivial_less_(int);
template <typename _Lhs, typename _Rhs>
std::false_type has_trivial_less_(...);
} //!details
//----------------------------------------------------------------------------
// SFINAE to check if _Lhs and _Rhs can be compared with ==
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_equal_t = decltype(details::has_trivial_equal_<_Lhs, _Rhs>(0));
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_equal_v = has_trivial_equal_t<_Lhs, _Rhs>::value;
//----------------------------------------------------------------------------
// SFINAE to check if _Lhs and _Rhs can be compared with <
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_less_t = decltype(details::has_trivial_less_<_Lhs, _Rhs>(0));
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_less_v = has_trivial_less_t<_Lhs, _Rhs>::value;
//----------------------------------------------------------------------------
// SFINAE to check if _Lhs and _Rhs can be compared with == and <
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs = _Lhs>
CONSTEXPR bool has_trivial_compare_v = (
    has_trivial_equal_v<_Lhs, _Rhs> &&
    has_trivial_less_v<_Lhs, _Rhs> );
template <typename _Lhs, typename _Rhs = _Lhs>
using has_trivial_compare_t = std::bool_constant<has_trivial_compare_v<_Lhs, _Rhs>>;
//----------------------------------------------------------------------------
// Unlike STL variants theses operators allow to compare _Lhs with a compatible _Rhs
//----------------------------------------------------------------------------
#define PPE_META_OPERATOR(_NAME, _OP) \
    template <typename _Lhs = void> \
    struct _NAME { \
        template <typename _Rhs> \
        CONSTEXPR bool operator()(const _Lhs& lhs, const _Rhs& rhs) const NOEXCEPT { \
            return (lhs _OP rhs); \
        } \
    }; \
    template <> \
    struct _NAME<void> { \
        template <typename _Lhs, typename _Rhs> \
        CONSTEXPR bool operator()(const _Lhs& lhs, const _Rhs& rhs) const NOEXCEPT { \
            return (lhs _OP rhs); \
        } \
    }
//----------------------------------------------------------------------------
PPE_META_OPERATOR(TGreater, > );
PPE_META_OPERATOR(TGreaterEqual, >= );
PPE_META_OPERATOR(TLess, < );
PPE_META_OPERATOR(TLessEqual, <= );
PPE_META_OPERATOR(TEqualTo, == );
//----------------------------------------------------------------------------
// Indirect variants, see DerefPtr<>() in TypeTraits.h
//----------------------------------------------------------------------------
namespace details {
template <typename _Ptr, template <typename T> class _Op>
struct TDerefOperator_ : _Op<decltype( *std::declval<const _Ptr&>() )> {
    using value_type = decltype(*std::declval<_Ptr>());
    using parent_type = _Op<value_type>;
    STATIC_ASSERT(is_pointer_v<_Ptr>);
    CONSTEXPR bool operator()(const _Ptr& lhs, const _Ptr& rhs) const NOEXCEPT {
        return ( (lhs == rhs) || ((!!lhs & !!rhs) && parent_type::operator()(*lhs, *rhs)) );
    }
    CONSTEXPR bool operator()(const _Ptr& lhs, const value_type& rhs) const NOEXCEPT {
        return (!!lhs && parent_type::operator()(*lhs, rhs));
    }
    CONSTEXPR bool operator()(const value_type& lhs, const _Ptr& rhs) const NOEXCEPT {
        return (!!rhs && parent_type::operator()(lhs, *rhs));
    }
};
} //!details
//----------------------------------------------------------------------------
template <typename _Ptr = void>
using TDerefGreater = details::TDerefOperator_< _Ptr, TGreater >;
template <typename _Ptr = void>
using TDerefGreaterEqual = details::TDerefOperator_< _Ptr, TGreaterEqual >;
template <typename _Ptr = void>
using TDerefLess = details::TDerefOperator_< _Ptr, TLess >;
template <typename _Ptr = void>
using TDerefLessEqual = details::TDerefOperator_< _Ptr, TLessEqual >;
template <typename _Ptr = void>
using TDerefEqualTo = details::TDerefOperator_< _Ptr, TEqualTo >;
//----------------------------------------------------------------------------
#undef PPE_META_OPERATOR
//----------------------------------------------------------------------------
// Comparison operator
//----------------------------------------------------------------------------
template <typename T = void>
struct TCompareTo {
    TEnableIf<has_trivial_compare_v<T>, int> operator ()(const T& lhs, const T& rhs) const NOEXCEPT {
        return (lhs < rhs ? -1 : lhs == rhs ? 0 : 1);
    }
};
template <>
struct TCompareTo<void> {
    template <typename U, typename V>
    TEnableIf<has_trivial_compare_v<U, V>, int> operator ()(const U& lhs, const V& rhs) const NOEXCEPT {
        return (lhs < rhs ? -1 : lhs == rhs ? 0 : 1);
    }
};
//----------------------------------------------------------------------------
// Arithmetic operators
//----------------------------------------------------------------------------
template <typename T = void >
using TPlus = std::plus<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TMinus = std::minus<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TMultiplies = std::multiplies<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TDivides = std::divides<T>;
//----------------------------------------------------------------------------
// Generic Min/Max/Clamp/Compare() functions
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = TEnableIf<has_trivial_less_v<_Lhs, _Rhs>>>
CONSTEXPR std::common_type_t<_Lhs, _Rhs> Max(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? b : a);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = TEnableIf<has_trivial_less_v<_Lhs, _Rhs>> >
CONSTEXPR std::common_type_t<_Lhs, _Rhs> Min(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? a : b);
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = TEnableIf<has_trivial_less_v<_A, _B> && has_trivial_less_v<_B, _C>> >
CONSTEXPR std::common_type_t<_A, std::common_type_t<_B, _C>> Max3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Max(a, Max(b, c));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = TEnableIf<has_trivial_less_v<_A, _B> && has_trivial_less_v<_B, _C>> >
CONSTEXPR std::common_type_t<_A, std::common_type_t<_B, _C>> Min3(const _A& a, const _B& b, const _C& c) NOEXCEPT {
    return Min(a, Min(b, c));
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = TEnableIf<has_trivial_compare_v<_Lhs, _Rhs>> >
CONSTEXPR int Compare(const _Lhs& a, const _Rhs& b) NOEXCEPT {
    return (a < b ? -1 : a == b ? 0 : 1);
}
//----------------------------------------------------------------------------
template <typename T, typename U, class = TEnableIf<has_trivial_less_v<T, U>> >
CONSTEXPR std::common_type_t<T, U> Clamp(const T& value, const U& vmin, const U& vmax) NOEXCEPT {
    return Min(vmax, Max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR size_t BitCount{ sizeof(T) << 3 };
//----------------------------------------------------------------------------
template <typename C, typename B, typename A>
inline /*CONSTEXPR*/ size_t OffsetOf(B A::*member) {
    constexpr std::aligned_storage_t<sizeof(C), alignof(C)> pod{};
    const C* const c = reinterpret_cast<const C*>(&pod);
    const A* const a = c;
    return (reinterpret_cast<size_t>(&(a->*member)) - reinterpret_cast<size_t>(c));
}
//----------------------------------------------------------------------------
template <typename B, typename A>
inline /*CONSTEXPR*/ size_t StandardLayoutOffset(B A::*member) {
    STATIC_ASSERT(std::is_standard_layout_v<A>);
    return OffsetOf<A, B, A>(member);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta

using Meta::Min;
using Meta::Max;
using Meta::Min3;
using Meta::Max3;
using Meta::Compare;
using Meta::Clamp;

} //!namespace PPE
