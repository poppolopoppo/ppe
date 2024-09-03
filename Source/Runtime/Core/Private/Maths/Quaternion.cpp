// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/Quaternion.h"

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FQuaternion FQuaternion::Identity (0, 0, 0, 1);
const FQuaternion FQuaternion::One      (1, 1, 1, 1);
const FQuaternion FQuaternion::Zero     (0, 0, 0, 0);
//----------------------------------------------------------------------------
bool FQuaternion::IsIdentity() const {
    return NearlyEquals(vec, float4::W);
}
//----------------------------------------------------------------------------
bool FQuaternion::IsNormalized() const {
    return Abs(LengthSq() - 1) <= Epsilon;
}
//----------------------------------------------------------------------------
float FQuaternion::Angle() const {
    Assert(Abs(Dot2(vec)) > SmallEpsilon);
    return (2.0f * FPlatformMaths::Acos(vec.w));
}
//----------------------------------------------------------------------------
float3 FQuaternion::Axis() const {
    const float length = Dot2(vec);
    Assert(Abs(length) > SmallEpsilon);
    return (vec.xyz * Rcp(length));
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Exponential() const {
    float angle = Angle();
    float fsin = FPlatformMaths::Sin(angle);

    float4 result;

    if (Abs(fsin) > SmallEpsilon) {
        result.xyz = (fsin / angle) * vec.xyz;
    }
    else {
        result = vec;
    }

    result.w = Cos(angle);
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Logarithm() const {
    float4 result;

    if (Abs(vec.w) < 1.0f + Epsilonf) {
        float angle = FPlatformMaths::Acos(Min(vec.w, 1.f));
        float fsin = FPlatformMaths::Sin(angle);

        if (Abs(fsin) > SmallEpsilon) {
            result.xyz = (angle / fsin) * vec.xyz;
        }
        else {
            result = vec;
        }
    }
    else {
        result = vec;
    }

    result.w = 0.0f;
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Invert() const {
    return FQuaternion(-vec.xyz, vec.w);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Normalize() const {
    return FQuaternion(vec * Rcp(Length()));
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::NormalizeInvert() const {
    return Normalize().Invert();
}
//----------------------------------------------------------------------------
float3 FQuaternion::Transform(const float3& value) const {
    float3 vector;
    float vx = vec.x + vec.x;
    float vy = vec.y + vec.y;
    float vz = vec.z + vec.z;
    float vwx = vec.w * vx;
    float vwy = vec.w * vy;
    float vwz = vec.w * vz;
    float vxx = vec.x * vx;
    float vxy = vec.x * vy;
    float vxz = vec.x * vz;
    float vyy = vec.y * vy;
    float vyz = vec.y * vz;
    float vzz = vec.z * vz;

    vector.x = ((value.x * ((1.0f - vyy) - vzz)) + (value.y * (vxy - vwz))) + (value.z * (vxz + vwy));
    vector.y = ((value.x * (vxy + vwz)) + (value.y * ((1.0f - vxx) - vzz))) + (value.z * (vyz - vwx));
    vector.z = ((value.x * (vxz - vwy)) + (value.y * (vyz + vwx))) + (value.z * ((1.0f - vxx) - vyy));

    return vector;
}
//----------------------------------------------------------------------------
float3 FQuaternion::InvertTransform(const float3& value) const {
    const float3 q(-vec.xy, vec.z); // Inverse
    const float3 t = 2.f * Cross(q, value);
    return value + (vec.w * t) + Cross(q, t);
}
//----------------------------------------------------------------------------
FQuaternion& FQuaternion::operator *=(const FQuaternion& other) {
    float lx = vec.x;
    float ly = vec.y;
    float lz = vec.z;
    float lw = vec.w;

    float rx = other.vec.x;
    float ry = other.vec.y;
    float rz = other.vec.z;
    float rw = other.vec.w;

    vec.x = (rx * lw + lx * rw + ry * lz) - (rz * ly);
    vec.y = (ry * lw + ly * rw + rz * lx) - (rx * lz);
    vec.z = (rz * lw + lz * rw + rx * ly) - (ry * lx);
    vec.w = (rw * lw) - (rx * lx + ry * ly + rz * lz);

    return *this;
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::operator *(const FQuaternion& other) const {
    float lx = vec.x;
    float ly = vec.y;
    float lz = vec.z;
    float lw = vec.w;

    float rx = other.vec.x;
    float ry = other.vec.y;
    float rz = other.vec.z;
    float rw = other.vec.w;

    return FQuaternion( (rx * lw + lx * rw + ry * lz) - (rz * ly),
                        (ry * lw + ly * rw + rz * lx) - (rx * lz),
                        (rz * lw + lz * rw + rx * ly) - (ry * lx),
                        (rw * lw) - (rx * lx + ry * ly + rz * lz) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
