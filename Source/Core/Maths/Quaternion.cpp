#include "stdafx.h"

#include "Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Quaternion::IsIdentity() const {
    return  _value.x() == 0 &&
            _value.y() == 0 &&
            _value.z() == 0 &&
            _value.w() == 1;
}
//----------------------------------------------------------------------------
bool Quaternion::IsNormalized() const {
    return std::abs(LengthSq() - 1) <= F_Epsilon;
}
//----------------------------------------------------------------------------
float Quaternion::Angle() const {
    Assert(std::abs(Dot3(_value, _value)) > F_Epsilon);
    return (2.0f * std::acos(_value.w()));
}
//----------------------------------------------------------------------------
float3 Quaternion::Axis() const {
    const float length = Dot3(_value, _value);
    Assert(std::abs(length) > F_Epsilon);
    const float inv = 1.0f / length;
    return float3(inv * _value.x(), inv * _value.y(), inv * _value.z());
}
//----------------------------------------------------------------------------
Quaternion Quaternion::Exponential() const {
    float angle = Angle();
    float fsin = std::sin(angle);

    float4 result;

    if (std::abs(fsin) > F_Epsilon) {
        float coeff = fsin / angle;
        result.x() = coeff * _value.x();
        result.y() = coeff * _value.y();
        result.z() = coeff * _value.z();
    }
    else {
        result = _value;
    }

    result.w() = std::cos(angle);
    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion Quaternion::Logarithm() const {
    float4 result;

    if (std::abs(_value.w()) < 1.0f) {
        float angle = std::acos(_value.w());
        float fsin = std::sin(angle);

        if (std::abs(fsin) > F_Epsilon) {
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
    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion Quaternion::Invert() const {
    float lengthSq = LengthSq();
    Assert(lengthSq > F_Epsilon);

    float inv = 1.0f / lengthSq;
    return Quaternion(-x() * inv, -y() * inv, -z() * inv, w() * inv);
}
//----------------------------------------------------------------------------
Quaternion Quaternion::Normalize() const {
    float length = Length();
    Assert(length > F_Epsilon);

    float inv = 1.0f / length;
    return Quaternion(_value * inv);
}
//----------------------------------------------------------------------------
float3 Quaternion::Transform(const float3& value) const {
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
Quaternion& Quaternion::operator *=(const Quaternion& other) {
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
Quaternion Quaternion::operator *(const Quaternion& other) const {
    float lx = _value.x();
    float ly = _value.y();
    float lz = _value.z();
    float lw = _value.w();

    float rx = other._value.x();
    float ry = other._value.y();
    float rz = other._value.z();
    float rw = other._value.w();

    return Quaternion(  (rx * lw + lx * rw + ry * lz) - (rz * ly),
                        (ry * lw + ly * rw + rz * lx) - (rx * lz),
                        (rz * lw + lz * rw + rx * ly) - (ry * lx),
                        (rw * lw) - (rx * lx + ry * ly + rz * lz) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
