#include "stdafx.h"

#include "QuaternionHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float radians) {
    float fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return MakeAxisQuaternion(axis, fsin, fcos);
}
//----------------------------------------------------------------------------
Quaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos) {
    Assert(std::abs(1 - LengthSq3(axis)) < F_Epsilon);

    float4 result;
    result.x() = axis.x() * fsin;
    result.y() = axis.y() * fsin;
    result.z() = axis.z() * fsin;
    result.w() = fcos;

    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix) {
    float sqrt;
    float half;
    float scale = matrix._11() + matrix._22() + matrix._33();

    float4 result;

    if (scale > 0.0f) {
        sqrt = std::sqrt(scale + 1.0f);
        result.w() = sqrt * 0.5f;
        sqrt = 0.5f / sqrt;

        result.x() = (matrix._23() - matrix._32()) * sqrt;
        result.y() = (matrix._31() - matrix._13()) * sqrt;
        result.z() = (matrix._12() - matrix._21()) * sqrt;
    }
    else if ((matrix._11() >= matrix._22()) && (matrix._11() >= matrix._33())) {
        sqrt = std::sqrt(1.0f + matrix._11() - matrix._22() - matrix._33());
        half = 0.5f / sqrt;

        result.x() = 0.5f * sqrt;
        result.y() = (matrix._12() + matrix._21()) * half;
        result.z() = (matrix._13() + matrix._31()) * half;
        result.w() = (matrix._23() - matrix._32()) * half;
    } else if (matrix._22() > matrix._33())
    {
        sqrt = std::sqrt(1.0f + matrix._22() - matrix._11() - matrix._33());
        half = 0.5f / sqrt;

        result.x() = (matrix._21() + matrix._12()) * half;
        result.y() = 0.5f * sqrt;
        result.z() = (matrix._32() + matrix._23()) * half;
        result.w() = (matrix._31() - matrix._13()) * half;
    }
    else{
        sqrt = std::sqrt(1.0f + matrix._33() - matrix._11() - matrix._22());
        half = 0.5f / sqrt;

        result.x() = (matrix._31() + matrix._13()) * half;
        result.y() = (matrix._32() + matrix._23()) * half;
        result.z() = 0.5f * sqrt;
        result.w() = (matrix._12() - matrix._21()) * half;
    }

    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll) {
    float halfRoll = roll * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfYaw = yaw * 0.5f;

    float sinRoll, cosRoll;
    SinCos(halfRoll, &sinRoll, &cosRoll);

    float sinPitch, cosPitch;
    SinCos(halfPitch, &sinPitch, &cosPitch);

    float sinYaw, cosYaw;
    SinCos(halfYaw, &sinYaw, &cosYaw);

    float4 result;
    result.x() = (cosYaw * sinPitch * cosRoll) + (sinYaw * cosPitch * sinRoll);
    result.y() = (sinYaw * cosPitch * cosRoll) - (cosYaw * sinPitch * sinRoll);
    result.z() = (cosYaw * cosPitch * sinRoll) - (sinYaw * sinPitch * cosRoll);
    result.w() = (cosYaw * cosPitch * cosRoll) + (sinYaw * sinPitch * sinRoll);

    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion MakeBarycentricQuaternion(
    const Quaternion& value1,
    const Quaternion& value2,
    const Quaternion& value3,
    float amount12,
    float amount13 ) {
    const Quaternion start = SLerp(value1, value2, amount12 + amount13);
    const Quaternion end = SLerp(value1, value3, amount12 + amount13);
    return SLerp(start, end, amount13 / (amount12 + amount13));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
