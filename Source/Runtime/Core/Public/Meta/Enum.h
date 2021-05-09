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
CONSTEXPR bool EnumIsFlags(_Enum) { return false; }
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool enum_is_flags_v = EnumIsFlags(_Enum{});
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
using TEnumOrd = std::underlying_type_t<_Enum>;
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR auto EnumOrd(_Enum value) {
    return TEnumOrd<_Enum>(value);
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR _Enum EnumAnd(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) & EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR _Enum EnumOr(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) | EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR _Enum EnumAdd(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) | EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR _Enum EnumRemove(_Enum lhs, _Enum rhs) {
    return _Enum(EnumOrd(lhs) & ~EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool EnumXor(_Enum lhs, _Enum rhs) {
    return !!(EnumOrd(lhs) & EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool EnumHas(_Enum lhs, TEnumOrd<_Enum> rhs) {
    return ((EnumOrd(lhs) & rhs) == rhs);
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool EnumHas(_Enum lhs, _Enum rhs) {
    return EnumHas(lhs, EnumOrd(rhs));
}
//----------------------------------------------------------------------------
template <typename _Enum, class = TEnableIf<std::is_enum_v<_Enum>> >
CONSTEXPR bool EnumLess(_Enum lhs, _Enum rhs) {
    return (EnumOrd(lhs) < EnumOrd(rhs));
}
//----------------------------------------------------------------------------
#define _ENUM_FLAGS_IMPL(_PREFIX, _ENUMTYPE) \
    CONSTEXPR bool EnumIsFlags(_ENUMTYPE) { return true; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE    BitAnd(_ENUMTYPE lhs, _ENUMTYPE rhs)      { return ::PPE::Meta::EnumAnd(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE    BitOr(_ENUMTYPE lhs, _ENUMTYPE rhs)       { return ::PPE::Meta::EnumOr(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE    operator |(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumOr(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE    operator +(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumAdd(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE    operator -(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumRemove(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&   operator|=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumOr(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&   operator+=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumAdd(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR _ENUMTYPE&   operator-=(_ENUMTYPE& lhs, _ENUMTYPE rhs) { lhs = ::PPE::Meta::EnumRemove(lhs, rhs); return lhs; } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator ^(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumXor(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator &(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumHas(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator <(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumLess(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator>=(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return not ::PPE::Meta::EnumLess(lhs, rhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator >(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return ::PPE::Meta::EnumLess(rhs, lhs); } \
    MAYBE_UNUSED _PREFIX CONSTEXPR bool         operator<=(_ENUMTYPE lhs, _ENUMTYPE rhs)  { return not ::PPE::Meta::EnumLess(rhs, lhs); } \
    STATIC_ASSERT(std::is_enum_v<_ENUMTYPE>)
//----------------------------------------------------------------------------
#define ENUM_FLAGS(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline, _ENUMTYPE)
#define ENUM_FLAGS_FRIEND(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline friend, _ENUMTYPE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
