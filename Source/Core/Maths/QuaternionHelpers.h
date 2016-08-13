#pragma once

#include "Core/Core.h"

#include "Core/Maths/Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Quaternion BarycentricLerp(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, float f0, float f1, float f2);
Quaternion BarycentricLerp(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const float3& uvw);
//----------------------------------------------------------------------------
float Dot(const Quaternion& lhs, const Quaternion& rhs);
//----------------------------------------------------------------------------
Quaternion Lerp(const Quaternion& v0, const Quaternion& v1, float f);
//----------------------------------------------------------------------------
Quaternion SLerp(const Quaternion& v0, const Quaternion& v1, float f);
//----------------------------------------------------------------------------
Quaternion SQuad(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const Quaternion& v3, float f);
//----------------------------------------------------------------------------
void SQuadSetup(Quaternion *p0, Quaternion *p1, Quaternion *p2,
                const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const Quaternion& v3 );
//----------------------------------------------------------------------------
void Extract3AxisFromQuaternion(float3 *paxisx, float3 *paxisy, float3 *paxisz,
                                const Quaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis);
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float radians);
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos);
//----------------------------------------------------------------------------
Quaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix);
//----------------------------------------------------------------------------
Quaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll);
//----------------------------------------------------------------------------
Quaternion Make3AxisQuaterion(const float3& axisx, const float3& axisy, const float3& axisz);
//----------------------------------------------------------------------------
Quaternion MakeBarycentricQuaternion(
    const Quaternion& value1,
    const Quaternion& value2,
    const Quaternion& value3,
    float amount12,
    float amount13 );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Packing tangent space in a quaternion
// https://dl.dropboxusercontent.com/u/16861957/gdc2015_rendering_the_world_of_far_cry_4.pdf
//----------------------------------------------------------------------------
inline Quaternion TangentSpaceToQuaternion(const float3& tangent, const float3& binormal, const float3& normal) {
    return Make3AxisQuaterion(tangent, binormal, normal);
}
//----------------------------------------------------------------------------
inline void QuaternionToTangentSpace(   float3 *ptangent, float3 *pbinormal, float3 *pnormal,
                                        const Quaternion& quaternion) {
    Extract3AxisFromQuaternion(ptangent, pbinormal, pnormal, quaternion);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/QuaternionHelpers-inl.h"
