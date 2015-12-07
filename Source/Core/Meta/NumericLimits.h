#pragma once

#include "Core/Meta/Aliases.h"

#include <limits>
#include <type_traits>

namespace Core {

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct NumericLimits {
    STATIC_ASSERT(std::is_arithmetic<T>::value);
    typedef typename std::numeric_limits<T> limits_type;

    static const T Epsilon;
    static const T Inf;
    static const T MaxValue;
    static const T MinValue;
    static const T Nan;
    static const T Default;
    static const T Zero;
};
template <typename T> const T NumericLimits<T>::Epsilon = limits_type::epsilon();
template <typename T> const T NumericLimits<T>::Inf = limits_type::infinity();
template <typename T> const T NumericLimits<T>::MaxValue = limits_type::max();
template <typename T> const T NumericLimits<T>::MinValue = limits_type::min();
template <typename T> const T NumericLimits<T>::Nan = limits_type::signaling_NaN();
template <typename T> const T NumericLimits<T>::Default = T();
template <typename T> const T NumericLimits<T>::Zero = T(0);
//----------------------------------------------------------------------------
template <>
struct NumericLimits<u128> {
    typedef NumericLimits<u64> limits_type;

    static const u128 Epsilon;
    static const u128 Inf;
    static const u128 MaxValue;
    static const u128 MinValue;
    static const u128 Nan;
    static const u128 Default;
    static const u128 Zero;
};
//----------------------------------------------------------------------------
template <>
struct NumericLimits<u256> {
    typedef NumericLimits<u128> limits_type;

    static const u256 Epsilon;
    static const u256 Inf;
    static const u256 MaxValue;
    static const u256 MinValue;
    static const u256 Nan;
    static const u256 Default;
    static const u256 Zero;
};
//----------------------------------------------------------------------------
extern template struct NumericLimits<bool>;
extern template struct NumericLimits<i8>;
extern template struct NumericLimits<i16>;
extern template struct NumericLimits<i32>;
extern template struct NumericLimits<i64>;
extern template struct NumericLimits<u8>;
extern template struct NumericLimits<u16>;
extern template struct NumericLimits<u32>;
extern template struct NumericLimits<u64>;
extern template struct NumericLimits<float>;
extern template struct NumericLimits<double>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

