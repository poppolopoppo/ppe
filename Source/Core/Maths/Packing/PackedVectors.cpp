#include "stdafx.h"

#include "PackedVectors.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::Epsilon{ FloatM11_to_UX10Y10Z10W2N(1.0f/1024,1.0f/1024,1.0f/1024,0) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::Inf{ FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,0) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::MaxValue{ FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,2) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::MinValue{ FloatM11_to_UX10Y10Z10W2N(-1.0f,-1.0f,-1.0f,0) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::Nan{ FloatM11_to_UX10Y10Z10W2N(1.0f,1.0f,1.0f,0) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::Default{ FloatM11_to_UX10Y10Z10W2N(0.0f,0.0f,0.0f,0) };
const UX10Y10Z10W2N NumericLimits< UX10Y10Z10W2N >::Zero{ FloatM11_to_UX10Y10Z10W2N(0.0f,0.0f,0.0f,0) };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
