#include "stdafx.h"

#include "NumericLimits.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
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
