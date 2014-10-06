#pragma once

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
};
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::Epsilon = NumericLimits<T>::limits_type::epsilon();
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::Inf = NumericLimits<T>::limits_type::infinity();
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::MaxValue = NumericLimits<T>::limits_type::max();
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::MinValue = NumericLimits<T>::limits_type::min();
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::Nan = NumericLimits<T>::limits_type::signaling_NaN();
//----------------------------------------------------------------------------
template <typename T>
const T NumericLimits<T>::Default = T();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
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
