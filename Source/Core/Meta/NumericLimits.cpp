#include "stdafx.h"

#include "NumericLimits.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const u128 NumericLimits<u128>::Epsilon = { limits_type::Epsilon, 0 };
const u128 NumericLimits<u128>::Inf = { limits_type::Inf, limits_type::Inf };
const u128 NumericLimits<u128>::MaxValue = { limits_type::MaxValue, limits_type::MaxValue };
const u128 NumericLimits<u128>::MinValue = { limits_type::MinValue, limits_type::MinValue };
const u128 NumericLimits<u128>::Nan = { limits_type::Nan, limits_type::Nan };
const u128 NumericLimits<u128>::Default = { limits_type::Default, limits_type::Default };
const u128 NumericLimits<u128>::Zero = { limits_type::Zero, limits_type::Zero };
//----------------------------------------------------------------------------
const u256 NumericLimits<u256>::Epsilon = { limits_type::Epsilon, limits_type::Zero };
const u256 NumericLimits<u256>::Inf = { limits_type::Inf, limits_type::Inf };
const u256 NumericLimits<u256>::MaxValue = { limits_type::MaxValue, limits_type::MaxValue };
const u256 NumericLimits<u256>::MinValue = { limits_type::MinValue, limits_type::MinValue };
const u256 NumericLimits<u256>::Nan = { limits_type::Nan, limits_type::Nan };
const u256 NumericLimits<u256>::Default = { limits_type::Default, limits_type::Default };
const u256 NumericLimits<u256>::Zero = { limits_type::Zero, limits_type::Zero };
//----------------------------------------------------------------------------
template struct NumericLimits<bool>;
template struct NumericLimits<i8>;
template struct NumericLimits<i16>;
template struct NumericLimits<i32>;
template struct NumericLimits<i64>;
template struct NumericLimits<u8>;
template struct NumericLimits<u16>;
template struct NumericLimits<u32>;
template struct NumericLimits<u64>;
template struct NumericLimits<float>;
template struct NumericLimits<double>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
