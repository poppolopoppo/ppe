#pragma once

#include "Core.h"

#include "Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float Dot(const Quaternion& lhs, const Quaternion& rhs);
//----------------------------------------------------------------------------
Quaternion Lerp(const Quaternion& v0, const Quaternion& v1, float f);
//----------------------------------------------------------------------------
Quaternion SLerp(const Quaternion& v0, const Quaternion& v1, float f);
//----------------------------------------------------------------------------
Quaternion SQuad(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const Quaternion& v3, float f);
//----------------------------------------------------------------------------
void SQuadSetup(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const Quaternion& v3,
                Quaternion& p0, Quaternion& p1, Quaternion& p2);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float radians);
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos);
//----------------------------------------------------------------------------
Quaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix);
//----------------------------------------------------------------------------
Quaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll);
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
} //!namespace Core

#include "QuaternionHelpers-inl.h"
