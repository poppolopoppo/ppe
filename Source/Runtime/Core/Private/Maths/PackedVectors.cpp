// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/PackedVectors.h"

#include "Maths/ScalarMatrixHelpers.h"
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
UX10Y10Z10W2N Quaternion_to_UX10Y10Z10W2N(const FQuaternion& quaternion) NOEXCEPT {
    float4 q = quaternion.data;
    const size_t index = q.xyz.MaxComponentIndex();
    if (0 == index) q = q.yzwx;
    if (1 == index) q = q.xzwy;
    if (2 == index) q = q.xywz;

    UX10Y10Z10W2N packed;
    packed.Pack_Float01(q.xyz * (Sign(q.w) * Sqrt2OO) + 0.5f, (u8)index);
    return packed;
}
//----------------------------------------------------------------------------
FQuaternion UX10Y10Z10W2N_to_Quaternion(const UX10Y10Z10W2N& packed) NOEXCEPT {
    float4 quaternion(
        packed._x * Sqrt2_v<float> - Sqrt2OO,
        packed._y * Sqrt2_v<float> - Sqrt2OO,
        packed._z * Sqrt2_v<float> - Sqrt2OO,
        0.f );
    quaternion.w = Sqrt(1.f - Saturate(LengthSq(quaternion)));

    const size_t index = packed._w;
    if (0 == index) quaternion = quaternion.wxyz;
    if (1 == index) quaternion = quaternion.xwyz;
    if (2 == index) quaternion = quaternion.xywz;

    return FQuaternion(quaternion);
}
//----------------------------------------------------------------------------
// https://www.shadertoy.com/view/NsfBWf
//----------------------------------------------------------------------------
float2 OctahedralNormalEncode(const float3& n/* [-1,1], should be normalized */) NOEXCEPT {
    Assert_NoAssume(IsNormalized(n));

    float2 p{ n.template Get<0>(), n.template Get<1>() };
    p /= Abs(n).HSum();
    return ((n.template Get<2>() <= 0.f) ? ( (1.f - Abs(p.yx)) * SignNotZero(p) ) : p);
}
//----------------------------------------------------------------------------
float3 OctahedralNormalDecode(const float2& v) NOEXCEPT {
    float3 n{ v, 1.f - Abs(v.template Get<0>()) - Abs(v.template Get<1>()) };
    if (n.z < 0)
        n.xy = float2((1.f - Abs(n.yx)) * SignNotZero(n.xy));
    return Normalize(n);
}
//----------------------------------------------------------------------------
ubyte2n Normal_to_Octahedral(const float3& normal) NOEXCEPT {
    return ubyte2n(FloatM11_to_UByte0255(OctahedralNormalEncode(normal)));
}
//----------------------------------------------------------------------------
float3  Octahedral_to_Normal(const ubyte2n& oct) NOEXCEPT {
    return OctahedralNormalDecode(UByte0255_to_FloatM11(ubyte2(oct)));
}
//----------------------------------------------------------------------------
// https://www.shadertoy.com/view/lllXz4
//----------------------------------------------------------------------------
u32 FibonacciSphereNormalEncode(const float3& p/* [-1,1], should be normalized */, const u32 n) NOEXCEPT {
    Assert_NoAssume(IsNormalized(p));

    const float m = Saturate(1.0f - 1.0f / n);
    const float cosTheta = p.z;

    const float k  = Max(2.0f, FloorToFloat( Log(n * PI_v<float> * Sqrt(5.0f) * (1.0f - cosTheta*cosTheta))/ Log(PHI_v<float>+1.0f)));
    const float Fk = pow(PHI, k) / Sqrt(5.0f);
    const float2 F( RoundToFloat(Fk), RoundToFloat(Fk * PHI) ); // k, k+1

    const float ooN = Rcp(static_cast<float>(n));
    const float2 ka = 2.0f*F*ooN;
    const float2 kb = 2.0f*PI*( Fractional((F+1.0f)*PHI) - (PHI-1.0f) );
    const float2x2 iB = float2x2{ ka.y, -ka.x,
                                  kb.y, -kb.x } / (ka.y*kb.x - ka.x*kb.y);
    const float2 c = FloorToFloat( iB.Multiply(float2(Min(Atan2(p.y, p.x), PI_v<float>), cosTheta - m)) );

    float d = 8.0f;
    float j = 0.0f;
    for( int s=0; s<4; s++ )  {
        const float2 uv = float2( float(s-2*(s/2)), float(s/2) );

        const float i = Dot(F, uv + c); // all quantities are ingeters (can take a round() for extra safety)

        const float phi = 2.0f * PI * Fractional(i*PHI);
        const float cosPhi = m - 2.0f*i*ooN;
        const float sinPhi = Sqrt(1.0f - cosPhi*cosPhi);

        const float3 q( Cos(phi) * sinPhi, Sin(phi) * sinPhi, cosPhi );
        const float squaredDistance = Dot(q-p, q-p);
        if (squaredDistance < d)  {
            d = squaredDistance;
            j = i;
        }
    }

    return RoundToUnsigned(j);
}
//----------------------------------------------------------------------------
float3 FibonacciSphereNormalDecode(const u32 index, const u32 n) NOEXCEPT {
    const float2 xy((float(index) + 0.5f) / n, Fractional(float(index) / PHI));
    const float2 pt(2.f *PI * xy.y, Acos(2.f * xy.x - 1.f) - PI * 0.5f);
    return float3(Cos(pt.x) * Cos(pt.y), Sin(pt.x) * Cos(pt.y), Sin(pt.y));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
