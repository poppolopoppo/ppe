#pragma once

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// helpers to ease manipulations of enum classes
//----------------------------------------------------------------------------
#define _ENUM_FLAGS_IMPL(_PREFIX, _ENUMTYPE) \
    _PREFIX CONSTEXPR _ENUMTYPE operator |(_ENUMTYPE lhs, _ENUMTYPE rhs) { return _ENUMTYPE(u64(lhs) | u64(rhs)); } \
    _PREFIX CONSTEXPR _ENUMTYPE operator +(_ENUMTYPE lhs, _ENUMTYPE rhs) { return _ENUMTYPE(u64(lhs) | u64(rhs)); } \
    _PREFIX CONSTEXPR _ENUMTYPE operator -(_ENUMTYPE lhs, _ENUMTYPE rhs) { return _ENUMTYPE(u64(lhs) & ~u64(rhs)); } \
    _PREFIX CONSTEXPR bool      operator ^(_ENUMTYPE lhs, _ENUMTYPE rhs) { return (0 != (u64(lhs) & u64(rhs))); } \
    _PREFIX CONSTEXPR bool      operator &(_ENUMTYPE lhs, _ENUMTYPE rhs) { return (u64(rhs) == (u64(lhs) & u64(rhs))); } \
    STATIC_ASSERT(std::is_enum<_ENUMTYPE>::value)
//----------------------------------------------------------------------------
#define ENUM_FLAGS(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline, _ENUMTYPE)
#define ENUM_FLAGS_FRIEND(_ENUMTYPE) _ENUM_FLAGS_IMPL(inline friend, _ENUMTYPE)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE