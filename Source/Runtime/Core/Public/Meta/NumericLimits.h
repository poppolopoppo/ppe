#pragma once

#include "Meta/Aliases.h"

#include <limits>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TNumericLimits {
    STATIC_ASSERT(std::is_arithmetic<T>::value);
    typedef typename std::numeric_limits<T> limits_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, limits_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  limits_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  limits_type::is_signed);

    static constexpr T DefaultValue() { return T(); }
    static constexpr T Epsilon() { return limits_type::epsilon(); }
    static constexpr T Inf() { return limits_type::infinity(); }
    static constexpr T MaxValue() { return limits_type::max(); }
    static constexpr T MinValue() { return limits_type::min(); }
    static constexpr T Nan() { return limits_type::signaling_NaN(); }
    static constexpr T Zero() { return T(0); }
};
//----------------------------------------------------------------------------
template <>
struct TNumericLimits<u128> {
    typedef TNumericLimits<uint64_t> limits_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, limits_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  limits_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  limits_type::is_signed);

    static constexpr u128 DefaultValue() { return u128{ limits_type::DefaultValue(), limits_type::DefaultValue() }; }
    static constexpr u128 Epsilon() { return u128{ limits_type::Epsilon(), limits_type::Epsilon() }; }
    static constexpr u128 Inf() { return u128{ limits_type::Inf(), limits_type::Inf() }; }
    static constexpr u128 MaxValue() { return u128{ limits_type::MaxValue(), limits_type::MaxValue() }; }
    static constexpr u128 MinValue() { return u128{ limits_type::MinValue(), limits_type::MinValue() }; }
    static constexpr u128 Nan() { return u128{ limits_type::Nan(), limits_type::Nan() }; }
    static constexpr u128 Zero() { return u128{ limits_type::Zero(), limits_type::Zero() }; }
};
//----------------------------------------------------------------------------
template <>
struct TNumericLimits<u256> {
    typedef TNumericLimits<u128> limits_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, limits_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  limits_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  limits_type::is_signed);

    static constexpr u256 DefaultValue() { return u256{ limits_type::DefaultValue(), limits_type::DefaultValue() }; }
    static constexpr u256 Epsilon() { return u256{ limits_type::Epsilon(), limits_type::Epsilon() }; }
    static constexpr u256 Inf() { return u256{ limits_type::Inf(), limits_type::Inf() }; }
    static constexpr u256 MaxValue() { return u256{ limits_type::MaxValue(), limits_type::MaxValue() }; }
    static constexpr u256 MinValue() { return u256{ limits_type::MinValue(), limits_type::MinValue() }; }
    static constexpr u256 Nan() { return u256{ limits_type::Nan(), limits_type::Nan() }; }
    static constexpr u256 Zero() { return u256{ limits_type::Zero(), limits_type::Zero() }; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Sz = sizeof(T)>
struct TIntegral;
template <typename T> struct TIntegral<T, 1> { typedef  u8 type; };
template <typename T> struct TIntegral<T, 2> { typedef u16 type; };
template <typename T> struct TIntegral<T, 4> { typedef u32 type; };
template <typename T> struct TIntegral<T, 8> { typedef u64 type; };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE