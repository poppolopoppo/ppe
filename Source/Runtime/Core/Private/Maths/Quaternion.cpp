#include "stdafx.h"

#include "Quaternion.h"

#include "ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FQuaternion FQuaternion::Identity (0, 0, 0, 1);
const FQuaternion FQuaternion::One      (1, 1, 1, 1);
const FQuaternion FQuaternion::Zero     (0, 0, 0, 0);
//----------------------------------------------------------------------------
bool FQuaternion::IsIdentity() const {
    return  _value.x() == 0 &&
            _value.y() == 0 &&
            _value.z() == 0 &&
            _value.w() == 1;
}
//----------------------------------------------------------------------------
bool FQuaternion::IsNormalized() const {
    return Abs(LengthSq() - 1) <= F_Epsilon;
}
//----------------------------------------------------------------------------
float FQuaternion::Angle() const {
    Assert(Abs(Dot3(_value, _value)) > F_SmallEpsilon);
    return (2.0f * std::acos(_value.w()));
}
//----------------------------------------------------------------------------
float3 FQuaternion::Axis() const {
    const float length = Dot3(_value, _value);
    Assert(Abs(length) > F_SmallEpsilon);
    const float inv = 1.0f / length;
    return float3(inv * _value.x(), inv * _value.y(), inv * _value.z());
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Exponential() const {
    float angle = Angle();
    float fsin = std::sin(angle);

    float4 result;

    if (Abs(fsin) > F_SmallEpsilon) {
        float coeff = fsin / angle;
        result.x() = coeff * _value.x();
        result.y() = coeff * _value.y();
        result.z() = coeff * _value.z();
    }
    else {
        result = _value;
    }

    result.w() = std::cos(angle);
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Logarithm() const {
    float4 result;

    if (Abs(_value.w()) < 1.0f) {
        float angle = std::acos(_value.w());
        float fsin = std::sin(angle);

        if (Abs(fsin) > F_SmallEpsilon) {
            float coeff = angle / fsin;
            result.x() = coeff * _value.x();
            result.y() = coeff * _value.y();
            result.z() = coeff * _value.z();
        }
        else {
            result = _value;
        }
    }
    else {
        result = _value;
    }

    result.w() = 0.0f;
    return FQuaternion(result);
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Invert() const {
    return FQuaternion(-_value.x(), -_value.y(), -_value.z(), _value.w());
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::Normalize() const {
    return FQuaternion(_value * Rcp(Length()));
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::NormalizeInvert() const {
    return Normalize().Invert();
}
//----------------------------------------------------------------------------
float3 FQuaternion::Transform(const float3& value) const {
    float3 vector;
    float x = _value.x() + _value.x();
    float y = _value.y() + _value.y();
    float z = _value.z() + _value.z();
    float wx = _value.w() * x;
    float wy = _value.w() * y;
    float wz = _value.w() * z;
    float xx = _value.x() * x;
    float xy = _value.x() * y;
    float xz = _value.x() * z;
    float yy = _value.y() * y;
    float yz = _value.y() * z;
    float zz = _value.z() * z;

    vector.x() = ((value.x() * ((1.0f - yy) - zz)) + (value.y() * (xy - wz))) + (value.z() * (xz + wy));
    vector.y() = ((value.x() * (xy + wz)) + (value.y() * ((1.0f - xx) - zz))) + (value.z() * (yz - wx));
    vector.z() = ((value.x() * (xz - wy)) + (value.y() * (yz + wx))) + (value.z() * ((1.0f - xx) - yy));

    return vector;
}
//----------------------------------------------------------------------------
float3 FQuaternion::InvertTransform(const float3& value) const {
    const float3 q(-_value.x(), -_value.y(), _value.z()); // Inverse
    const float3 t = 2.f * Cross(q, value);
    return value + (_value.w() * t) + Cross(q, t);
}
//----------------------------------------------------------------------------
FQuaternion& FQuaternion::operator *=(const FQuaternion& other) {
    float lx = _value.x();
    float ly = _value.y();
    float lz = _value.z();
    float lw = _value.w();

    float rx = other._value.x();
    float ry = other._value.y();
    float rz = other._value.z();
    float rw = other._value.w();

    _value.x() = (rx * lw + lx * rw + ry * lz) - (rz * ly);
    _value.y() = (ry * lw + ly * rw + rz * lx) - (rx * lz);
    _value.z() = (rz * lw + lz * rw + rx * ly) - (ry * lx);
    _value.w() = (rw * lw) - (rx * lx + ry * ly + rz * lz);

    return *this;
}
//----------------------------------------------------------------------------
FQuaternion FQuaternion::operator *(const FQuaternion& other) const {
    float lx = _value.x();
    float ly = _value.y();
    float lz = _value.z();
    float lw = _value.w();

    float rx = other._value.x();
    float ry = other._value.y();
    float rz = other._value.z();
    float rw = other._value.w();

    return FQuaternion( (rx * lw + lx * rw + ry * lz) - (rz * ly),
                        (ry * lw + ly * rw + rz * lx) - (rx * lz),
                        (rz * lw + lz * rw + rx * ly) - (ry * lx),
                        (rw * lw) - (rx * lx + ry * ly + rz * lz) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
