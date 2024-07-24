#pragma once

#include "Core.h"

#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, float f0, float f1, float f2);
NODISCARD PPE_CORE_API FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const float3& uvw);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion Lerp(const FQuaternion& v0, const FQuaternion& v1, float f);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion SLerp(const FQuaternion& v0, const FQuaternion& v1, float f);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion SQuad(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3, float f);
//----------------------------------------------------------------------------
PPE_CORE_API void SQuadSetup(FQuaternion *p0, FQuaternion *p1, FQuaternion *p2,
                const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3 );
//----------------------------------------------------------------------------
PPE_CORE_API void Extract3AxisFromQuaternion(float3 *paxisx, float3 *paxisy, float3 *paxisz,
                                             const FQuaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Generates the 'smallest' (geodesic) rotation between two vectors of arbitrary length
NODISCARD PPE_CORE_API FQuaternion MakeQuaternionBetweenVectors(const float3& v0, const float3& v1);
//----------------------------------------------------------------------------
// Generates the 'smallest' (geodesic) rotation between two normalized vectors
NODISCARD PPE_CORE_API FQuaternion MakeQuaternionBetweenNormals(const float3& n0, const float3& n1);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeAxisQuaternion(const float3& axis);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeAxisQuaternion(const float3& axis, float radians);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion Make3AxisQuaterion(const float3& axisx, const float3& axisy, const float3& axisz);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FQuaternion MakeBarycentricQuaternion(
    const FQuaternion& value1,
    const FQuaternion& value2,
    const FQuaternion& value3,
    float amount12,
    float amount13 );
//----------------------------------------------------------------------------
NODISCARD inline float Dot(const FQuaternion& lhs, const FQuaternion& rhs) {
    return Dot(lhs.data, rhs.data);
}
//----------------------------------------------------------------------------
NODISCARD inline bool NearlyEquals(const FQuaternion& a, const FQuaternion& b, float maxRelDiff = Epsilon) {
    return NearlyEquals(a.data, b.data, maxRelDiff);
}
//----------------------------------------------------------------------------
NODISCARD inline bool IsINF(const FQuaternion& q) { return (IsINF(q.data)); }
//----------------------------------------------------------------------------
NODISCARD inline bool IsNAN(const FQuaternion& q) { return (IsNAN(q.data)); }
//----------------------------------------------------------------------------
NODISCARD inline bool IsNANorINF(const FQuaternion& q) { return (IsNANorINF(q.data)); }
//----------------------------------------------------------------------------
// Packing tangent space in a quaternion
// https://www.benicourt.com/blender/wp-content/uploads/2015/03/gdc2015_rendering_the_world_of_far_cry_4.pdf
//----------------------------------------------------------------------------
NODISCARD inline FQuaternion TangentSpaceToQuaternion(const float3& tangent, const float3& binormal, const float3& normal) {
    return Make3AxisQuaterion(tangent, binormal, normal);
}
//----------------------------------------------------------------------------
inline void QuaternionToTangentSpace(   float3 *ptangent, float3 *pbinormal, float3 *pnormal,
                                        const FQuaternion& quaternion) {
    Extract3AxisFromQuaternion(ptangent, pbinormal, pnormal, quaternion);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
