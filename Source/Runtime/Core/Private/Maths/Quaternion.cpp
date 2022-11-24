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
    return  data.x == 0 &&
            data.y == 0 &&
            data.z == 0 &&
            data.w == 1;
}
//----------------------------------------------------------------------------
bool FQuaternion::IsNormalized() const {
    return Abs(LengthSq() - 1) <= Epsilon;
}
//----------------------------------------------------------------------------
float FQuaternion::Angle() const {
    Assert(Abs(Dot(data, data)) > SmallEpsilon);
    return (2.0f * FPlatformMaths::Acos(data.w));
}
//----------------------------------------------------------------------------
float3 FQuaternion::Axis() const {
    const float length = Dot(data, data);
    Assert(Abs(length) > SmallEpsilon);
    const float inv = 1.0f / length;
    return float3(inv * data.x, inv * data.y, inv * data.z);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Exponential() const {
    float angle = Angle();
    float fsin = FPlatformMaths::Sin(angle);

    float4 result;

    if (Abs(fsin) > SmallEpsilon) {
        float coeff = fsin / angle;
        result.x = coeff * data.x;
        result.y = coeff * data.y;
        result.z = coeff * data.z;
    }
    else {
        result = data;
    }

    result.w = FPlatformMaths::Cos(angle);
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Logarithm() const {
    float4 result;

    if (Abs(data.w) < 1.0f) {
        float angle = FPlatformMaths::Acos(data.w);
        float fsin = FPlatformMaths::Sin(angle);

        if (Abs(fsin) > SmallEpsilon) {
            float coeff = angle / fsin;
            result.x = coeff * data.x;
            result.y = coeff * data.y;
            result.z = coeff * data.z;
        }
        else {
            result = data;
        }
    }
    else {
        result = data;
    }

    result.w = 0.0f;
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Invert() const {
    return FQuaternion(-data.x, -data.y, -data.z, data.w);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Normalize() const {
    return FQuaternion(data * Rcp(Length()));
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::NormalizeInvert() const {
    return Normalize().Invert();
}
//----------------------------------------------------------------------------
float3 FQuaternion::Transform(const float3& value) const {
    float3 vector;
    float vx = data.x + data.x;
    float vy = data.y + data.y;
    float vz = data.z + data.z;
    float vwx = data.w * vx;
    float vwy = data.w * vy;
    float vwz = data.w * vz;
    float vxx = data.x * vx;
    float vxy = data.x * vy;
    float vxz = data.x * vz;
    float vyy = data.y * vy;
    float vyz = data.y * vz;
    float vzz = data.z * vz;

    vector.x = ((value.x * ((1.0f - vyy) - vzz)) + (value.y * (vxy - vwz))) + (value.z * (vxz + vwy));
    vector.y = ((value.x * (vxy + vwz)) + (value.y * ((1.0f - vxx) - vzz))) + (value.z * (vyz - vwx));
    vector.z = ((value.x * (vxz - vwy)) + (value.y * (vyz + vwx))) + (value.z * ((1.0f - vxx) - vyy));

    return vector;
}
//----------------------------------------------------------------------------
float3 FQuaternion::InvertTransform(const float3& value) const {
    const float3 q(-data.x, -data.y, data.z); // Inverse
    const float3 t = 2.f * Cross(q, value);
    return value + (data.w * t) + Cross(q, t);
}
//----------------------------------------------------------------------------
FQuaternion& FQuaternion::operator *=(const FQuaternion& other) {
    float lx = data.x;
    float ly = data.y;
    float lz = data.z;
    float lw = data.w;

    float rx = other.data.x;
    float ry = other.data.y;
    float rz = other.data.z;
    float rw = other.data.w;

    data.x = (rx * lw + lx * rw + ry * lz) - (rz * ly);
    data.y = (ry * lw + ly * rw + rz * lx) - (rx * lz);
    data.z = (rz * lw + lz * rw + rx * ly) - (ry * lx);
    data.w = (rw * lw) - (rx * lx + ry * ly + rz * lz);

    return *this;
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::operator *(const FQuaternion& other) const {
    float lx = data.x;
    float ly = data.y;
    float lz = data.z;
    float lw = data.w;

    float rx = other.data.x;
    float ry = other.data.y;
    float rz = other.data.z;
    float rw = other.data.w;

    return FQuaternion( (rx * lw + lx * rw + ry * lz) - (rz * ly),
                        (ry * lw + ly * rw + rz * lx) - (rx * lz),
                        (rz * lw + lz * rw + rx * ly) - (ry * lx),
                        (rw * lw) - (rx * lx + ry * ly + rz * lz) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
