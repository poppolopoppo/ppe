#pragma once

#include "Core/Core.h"

#include "Core/Maths/Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, float f0, float f1, float f2);
FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const float3& uvw);
//----------------------------------------------------------------------------
float Dot(const FQuaternion& lhs, const FQuaternion& rhs);
//----------------------------------------------------------------------------
FQuaternion Lerp(const FQuaternion& v0, const FQuaternion& v1, float f);
//----------------------------------------------------------------------------
FQuaternion SLerp(const FQuaternion& v0, const FQuaternion& v1, float f);
//----------------------------------------------------------------------------
FQuaternion SQuad(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3, float f);
//----------------------------------------------------------------------------
void SQuadSetup(FQuaternion *p0, FQuaternion *p1, FQuaternion *p2,
                const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3 );
//----------------------------------------------------------------------------
void Extract3AxisFromQuaternion(float3 *paxisx, float3 *paxisy, float3 *paxisz,
                                const FQuaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis);
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis, float radians);
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos);
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix);
//----------------------------------------------------------------------------
FQuaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll);
//----------------------------------------------------------------------------
FQuaternion Make3AxisQuaterion(const float3& axisx, const float3& axisy, const float3& axisz);
//----------------------------------------------------------------------------
FQuaternion MakeBarycentricQuaternion(
    const FQuaternion& value1,
    const FQuaternion& value2,
    const FQuaternion& value3,
    float amount12,
    float amount13 );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Packing tangent space in a quaternion
// https://dl.dropboxusercontent.com/u/16861957/gdc2015_rendering_the_world_of_far_cry_4.pdf
//----------------------------------------------------------------------------
inline FQuaternion TangentSpaceToQuaternion(const float3& tangent, const float3& binormal, const float3& normal) {
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
} //!namespace Core

#include "Core/Maths/QuaternionHelpers-inl.h"
