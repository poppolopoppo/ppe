#pragma once

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// helpers to ease manipulations of enum classes
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR CONSTF bool EnumIsFlags(_Enum) { return false; }
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool enum_is_flags_v = EnumIsFlags(_Enum{});
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool enum_no_flags_v = not EnumIsFlags(_Enum{});
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
using TEnumOrd = std::underlying_type_t<_Enum>;
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF auto EnumOrd(_Enum value) {
    IF_CONSTEXPR(std::is_enum_v<_Enum>) {
        return TEnumOrd<_Enum>(value);
    }
    else {
        STATIC_ASSERT(std::is_integral_v<_Enum>);
        return value;
    }
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumOneComplement(_Enum value) {
    return _Enum(~EnumOrd(value));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumAnd(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) & EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumOr(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) | EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumAdd(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) | EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumRemove(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) & ~EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF _Enum EnumSet(_Enum set, _Enum flags, bool enabled) {
    return (enabled ? EnumAdd(set, flags) : EnumRemove(set, flags));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF bool EnumXor(_Enum lhs, _Enum rhs) {
    return !!(EnumOrd(lhs) & EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF bool EnumHas(_Enum lhs, TEnumOrd<_Enum> rhs) {
    return ((EnumOrd(lhs) & rhs) == rhs);
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF bool EnumHas(_Enum lhs, _Enum rhs) {
    return ((EnumOrd(lhs) & EnumOrd(rhs)) == EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR CONSTF bool EnumLess(_Enum lhs, _Enum rhs) {
    return (EnumOrd(lhs) < EnumOrd(rhs));
}
//----------------------------------------------------------------------------
#define _ENUM_FLAGS_IMPL(_PREFIX, _ENUMTYPE) \
    _PREFIX CONSTEXPR bool EnumIsFlags(_ENUMTYPE) { return true; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    BitAnd(_ENUMTYPE lhs, _ENUMTYPE rhs)      { return ::PPE::Meta::EnumAnd(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    BitOr(_ENUMTYPE lhs, _ENUMTYPE rhs)       { return ::PPE::Meta::EnumOr(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    operator ~(_ENUMTYPE value)               { return ::PPE::Meta::EnumOneComplement(value); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    operator |(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumOr(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    operator +(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumAdd(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF _ENUMTYPE    operator -(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumRemove(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&          operator|=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumOr(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&          operator+=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumAdd(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&          operator-=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumRemove(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator ^(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumXor(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator &(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumHas(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator <(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumLess(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator>=(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return not ::PPE::Meta::EnumLess(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator >(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumLess(rhs, lhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR CONSTF bool         operator<=(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return not ::PPE::Meta::EnumLess(rhs, lhs); } \
    STATIC_ASSERT(std::is_enum_v<_ENUMTYPE>)
//----------------------------------------------------------------------------
#define ENUM_FLAGS(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline, _ENUMTYPE)
#define ENUM_FLAGS_FRIEND(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline friend, _ENUMTYPE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
