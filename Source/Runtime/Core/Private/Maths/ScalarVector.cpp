#include "stdafx.h"

#include "Maths/ScalarVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
    /*
#define DEF_SCALARVECTOR(_Scalar) \
    template class TScalarVector<_Scalar, 2>; \
    template class TScalarVector<_Scalar, 3>; \
    template class TScalarVector<_Scalar, 4>;
//----------------------------------------------------------------------------
DEF_SCALARVECTOR(int)
DEF_SCALARVECTOR(unsigned)
DEF_SCALARVECTOR(float)
DEF_SCALARVECTOR(double)
//----------------------------------------------------------------------------
#undef DEF_SCALARVECTOR
*/
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_forceinit_constructor<uint2>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<word3>::value);
STATIC_ASSERT(Meta::has_forceinit_constructor<float4>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Constants {
const float2 Float2_One     = float2(1.0f);
const float2 Float2_Half    = float2(0.5f);
const float2 Float2_Zero    = float2(0.0f);
const float3 Float3_One     = float3(1.0f);
const float3 Float3_Half    = float3(0.5f);
const float3 Float3_Zero    = float3(0.0f);
const float4 Float4_One     = float4(1.0f);
const float4 Float4_Half    = float4(0.5f);
const float4 Float4_Zero    = float4(0.0f);
} //!Constants
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE