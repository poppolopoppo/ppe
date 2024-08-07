﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/QuaternionHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, float f0, float f1, float f2) {
    const float4 result(
        BarycentricLerp<float>(v0.x, v1.x, v2.x, f0, f1, f2),
        BarycentricLerp<float>(v0.y, v1.y, v2.y, f0, f1, f2),
        BarycentricLerp<float>(v0.z, v1.z, v2.z, f0, f1, f2),
        BarycentricLerp<float>(v0.w, v1.w, v2.w, f0, f1, f2) );

    return FQuaternion(Normalize(result));
}
//----------------------------------------------------------------------------
FQuaternion BarycentricLerp(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const float3& uvw) {
    return BarycentricLerp(v0, v1, v2, uvw.x, uvw.y, uvw.z);
}
//----------------------------------------------------------------------------
FQuaternion Lerp(const FQuaternion& v0, const FQuaternion& v1, float f) {
    float inverse = 1.0f - f;

    float4 result;

    if (Dot(v0, v1) >= 0.0f) {
        result.x = (inverse * v0.x) + (f * v1.x);
        result.y = (inverse * v0.y) + (f * v1.y);
        result.z = (inverse * v0.z) + (f * v1.z);
        result.w = (inverse * v0.w) + (f * v1.w);
    }
    else {
        result.x = (inverse * v0.x) - (f * v1.x);
        result.y = (inverse * v0.y) - (f * v1.y);
        result.z = (inverse * v0.z) - (f * v1.z);
        result.w = (inverse * v0.w) - (f * v1.w);
    }

    return FQuaternion(Normalize(result));
}
//----------------------------------------------------------------------------
FQuaternion SLerp(const FQuaternion& v0, const FQuaternion& v1, float f) {
    float opposite;
    float inverse;
    float dot = Dot(v0, v1);

    if (Abs(dot) > 1.0f - Epsilon) {
        inverse = 1.0f - f;
        opposite = f * (dot < 0 ? -1 : 1);
    }
    else {
        float facos = FPlatformMaths::Acos(Abs(dot));
        float invSin = (1.0f / FPlatformMaths::Sin(facos));

        inverse = FPlatformMaths::Sin((1.0f - f) * facos) * invSin;
        opposite = FPlatformMaths::Sin(f * facos) * invSin * (dot < 0 ? -1 : 1);
    }

    const float4 result(
        (inverse * v0.x) + (opposite * v1.x),
        (inverse * v0.y) + (opposite * v1.y),
        (inverse * v0.z) + (opposite * v1.z),
        (inverse * v0.w) + (opposite * v1.w) );

    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion SQuad(const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3, float f) {
    const FQuaternion start = SLerp(v0, v3, f);
    const FQuaternion end = SLerp(v1, v2, f);
    return SLerp(start, end, (2*f) * (1-f));
}
//----------------------------------------------------------------------------
void SQuadSetup(FQuaternion *p0, FQuaternion *p1, FQuaternion *p2,
                const FQuaternion& v0, const FQuaternion& v1, const FQuaternion& v2, const FQuaternion& v3 ) {
    Assert(p0);
    Assert(p1);
    Assert(p2);

    FQuaternion q0((v0 + v1).LengthSq() < (v0 - v1).LengthSq() ? -v0.data : v0.data);
    FQuaternion q2((v1 + v2).LengthSq() < (v1 - v2).LengthSq() ? -v2.data : v2.data);
    FQuaternion q3((v2 + v3).LengthSq() < (v2 - v3).LengthSq() ? -v3.data : v3.data);
    FQuaternion q1 = v1;

    const FQuaternion q1Exp = q1.Exponential();
    const FQuaternion q2Exp = q2.Exponential();

    *p0 = q1 * (-0.25f * ((q1Exp * q2).Logarithm() + (q1Exp * q0).Logarithm() )).Exponential();
    *p1 = q2 * (-0.25f * ((q2Exp * q3).Logarithm() + (q2Exp * q1).Logarithm() )).Exponential();
    *p2 = q2;
}
//----------------------------------------------------------------------------
void Extract3AxisFromQuaternion(float3 *paxisx, float3 *paxisy, float3 *paxisz,
                                const FQuaternion& quaternion ) {
    Assert(paxisx);
    Assert(paxisy);
    Assert(paxisz);

    const float4& q = quaternion.data;

    *paxisx     =   float3( 1.0f, 0.0f, 0.0f)
                +   float3(-2.0f, 2.0f, 2.0f) * q.y * q.yxw
                +   float3(-2.0f,-2.0f, 2.0f) * q.z * q.zwx;
    *paxisy     =   float3( 0.0f, 1.0f, 0.0f)
                +   float3( 2.0f,-2.0f, 2.0f) * q.z * q.wzy
                +   float3( 2.0f,-2.0f,-2.0f) * q.x * q.yxw;
    *paxisz     =   float3( 0.0f, 0.0f, 1.0f)
                +   float3( 2.0f, 2.0f,-2.0f) * q.x * q.zwx
                +   float3(-2.0f, 2.0f,-2.0f) * q.y * q.wzy;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Based on:
// http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
// http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
//
static FQuaternion MakeQuaternionBetween_(const float3& A, const float3& B, float NormAB) {
    float W = NormAB + Dot(A, B);

    FQuaternion Q;

    if (W >= SmallEpsilon * NormAB) {
        Q = FQuaternion(
            A.y * B.z - A.z * B.y,
            A.z * B.x - A.x * B.z,
            A.x * B.y - A.y * B.x,
            W );
    }
    else {
        // A and B point in opposite directions
        W = 0.f;
        Q = (Abs(A.x) > Abs(A.y)
            ?   FQuaternion(-A.z, 0.f, A.x, W)
            :   FQuaternion(0.f, -A.z, A.y, W) );
    }

    return Q.Normalize();
}
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionBetweenVectors(const float3& v0, const float3& v1) {
    const float l = Sqrt(LengthSq(v0) * LengthSq(v1));
    return MakeQuaternionBetween_(v0, v1, l);
}
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionBetweenNormals(const float3& n0, const float3& n1) {
    return MakeQuaternionBetween_(n0, n1, 1.f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis) {
    return MakeAxisQuaternion(axis, 0.0f, 1.0f); // SinCos(0)
}
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis, float radians) {
    float fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return MakeAxisQuaternion(axis, fsin, fcos);
}
//----------------------------------------------------------------------------
FQuaternion MakeAxisQuaternion(const float3& axis, float fsin, float fcos) {
    Assert(Abs(1.0f - LengthSq(axis)) < Epsilon);

    const float4 result(
        axis.x * fsin,
        axis.y * fsin,
        axis.z * fsin,
        fcos );

    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion MakeQuaternionFromRotationMatrix(const float4x4& matrix) {
    float sqrt;
    float half;
    float scale = matrix._11() + matrix._22() + matrix._33();

    float4 result;

    if (scale > 0.0f) {
        sqrt = FPlatformMaths::Sqrt(scale + 1.0f);
        result.w = sqrt * 0.5f;
        sqrt = 0.5f / sqrt;

        result.x = (matrix._23() - matrix._32()) * sqrt;
        result.y = (matrix._31() - matrix._13()) * sqrt;
        result.z = (matrix._12() - matrix._21()) * sqrt;
    }
    else if ((matrix._11() >= matrix._22()) && (matrix._11() >= matrix._33())) {
        sqrt = FPlatformMaths::Sqrt(1.0f + matrix._11() - matrix._22() - matrix._33());
        half = 0.5f / sqrt;

        result.x = 0.5f * sqrt;
        result.y = (matrix._12() + matrix._21()) * half;
        result.z = (matrix._13() + matrix._31()) * half;
        result.w = (matrix._23() - matrix._32()) * half;
    }
    else if (matrix._22() > matrix._33()) {
        sqrt = FPlatformMaths::Sqrt(1.0f + matrix._22() - matrix._11() - matrix._33());
        half = 0.5f / sqrt;

        result.x = (matrix._21() + matrix._12()) * half;
        result.y = 0.5f * sqrt;
        result.z = (matrix._32() + matrix._23()) * half;
        result.w = (matrix._31() - matrix._13()) * half;
    }
    else {
        sqrt = FPlatformMaths::Sqrt(1.0f + matrix._33() - matrix._11() - matrix._22());
        half = 0.5f / sqrt;

        result.x = (matrix._31() + matrix._13()) * half;
        result.y = (matrix._32() + matrix._23()) * half;
        result.z = 0.5f * sqrt;
        result.w = (matrix._12() - matrix._21()) * half;
    }

    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion MakeYawPitchRollQuaternion(float yaw, float pitch, float roll) {
    float halfRoll = roll * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfYaw = yaw * 0.5f;

    float sinRoll, cosRoll;
    SinCos(halfRoll, &sinRoll, &cosRoll);

    float sinPitch, cosPitch;
    SinCos(halfPitch, &sinPitch, &cosPitch);

    float sinYaw, cosYaw;
    SinCos(halfYaw, &sinYaw, &cosYaw);

    const float4 result(
        (cosYaw * sinPitch * cosRoll) + (sinYaw * cosPitch * sinRoll),
        (sinYaw * cosPitch * cosRoll) - (cosYaw * sinPitch * sinRoll),
        (cosYaw * cosPitch * sinRoll) - (sinYaw * sinPitch * cosRoll),
        (cosYaw * cosPitch * cosRoll) + (sinYaw * sinPitch * sinRoll) );

    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion Make3AxisQuaterion(const float3& axisx, const float3& axisy, const float3& axisz) {
    FQuaternion quaternion;
    quaternion.x = axisz.y - axisy.z;
    quaternion.y = axisx.z - axisz.x;
    quaternion.z = axisy.x - axisx.y;
    quaternion.w = 1.0f + axisx.x + axisy.y + axisz.z;
    return quaternion.Normalize();
}
//----------------------------------------------------------------------------
FQuaternion MakeBarycentricQuaternion(
    const FQuaternion& value1,
    const FQuaternion& value2,
    const FQuaternion& value3,
    float amount12,
    float amount13 ) {
    const FQuaternion start = SLerp(value1, value2, amount12 + amount13);
    const FQuaternion end = SLerp(value1, value3, amount12 + amount13);
    return SLerp(start, end, amount13 / (amount12 + amount13));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
