#include "stdafx.h"

#include "Maths/PackedVectors.h"

#include "Maths/ScalarVectorHelpers.h"
#include "Maths/QuaternionHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(u32) == sizeof(UX10Y10Z10W2N));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://www.benicourt.com/blender/wp-content/uploads/2015/03/gdc2015_rendering_the_world_of_far_cry_4.pdf
//----------------------------------------------------------------------------
UX10Y10Z10W2N Quaternion_to_UX10Y10Z10W2N(const FQuaternion& quaternion) {
    float4 q = quaternion.Value();
    const size_t index = BiggestComponent4(q);
    if (0 == index) q = q.yzwx();
    if (1 == index) q = q.xzwy();
    if (1 == index) q = q.xywz();

    UX10Y10Z10W2N packed;
    packed.Pack_Float01(q.xyz() * (Sign(q.w()) * F_Sqrt2OO) + 0.5f, (u8)index);
    return packed;
}
//----------------------------------------------------------------------------
FQuaternion UX10Y10Z10W2N_to_Quaternion(const UX10Y10Z10W2N& packed) {
    float4 quaternion(
        packed._x *  F_Sqrt2 - F_Sqrt2OO,
        packed._y *  F_Sqrt2 - F_Sqrt2OO,
        packed._z *  F_Sqrt2 - F_Sqrt2OO,
        0.f );
    quaternion.w() = Sqrt(1.f - Saturate(LengthSq3(quaternion)));

    const size_t index = packed._w;
    if (0 == index) quaternion = quaternion.wxyz();
    if (1 == index) quaternion = quaternion.xwyz();
    if (2 == index) quaternion = quaternion.xywz();

    return FQuaternion(quaternion);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
