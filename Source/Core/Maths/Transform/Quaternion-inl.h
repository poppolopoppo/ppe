#pragma once

#include "Core/Maths/Transform/Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(float broadcast)
:   _value(broadcast) {}
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(const float4& value)
:   _value(value) {}
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(const float3& value, float w)
:   _value(value, w) {}
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(const float2& xy, float z, float w)
:   _value(xy.x(), xy.y(), z, w) {}
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(float x, float y, float z, float w)
:   _value(x, y, z, w) {}
//----------------------------------------------------------------------------
inline Quaternion::Quaternion(const Quaternion& other)
:   _value(other._value) {}
//----------------------------------------------------------------------------
inline Quaternion& Quaternion::operator =(const Quaternion& other) {
    _value = other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline bool Quaternion::IsIdentity() const {
    return  _value.x() == 0 &&
            _value.y() == 0 &&
            _value.z() == 0 &&
            _value.w() == 1;
}
//----------------------------------------------------------------------------
inline bool Quaternion::IsNormalized() const {
    return std::abs(LengthSq() - 1) <= F_Epsilon;
}
//----------------------------------------------------------------------------
inline float Quaternion::Angle() const {
    Assert(std::abs(Dot3(_value, _value)) > F_Epsilon);
    return (2.0f * std::acos(_value.w()));
}
//----------------------------------------------------------------------------
inline float3 Quaternion::Axis() const {
    const float length = Dot3(_value, _value);
    Assert(std::abs(length) > F_Epsilon);
    const float inv = 1.0f / length;
    return float3(inv * _value.x(), inv * _value.y(), inv * _value.z());
}
//----------------------------------------------------------------------------
inline float Quaternion::Length() const {
    return Length4(_value);
}
//----------------------------------------------------------------------------
inline float Quaternion::LengthSq() const {
    return LengthSq4(_value);
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x(), -y(), -z(), w());
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::Invert() const {
    float lengthSq = LengthSq();
    Assert(lengthSq > F_Epsilon);

    float inv = 1.0f / lengthSq;
    return Quaternion(-x() * inv, -y() * inv, -z() * inv, w() * inv);
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::Negate() const {
    return Quaternion(-_value);
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::Normalize() const {
    float length = Length();
    Assert(length > F_Epsilon);

    float inv = 1.0f / length;
    return Quaternion(_value * inv);
}
//----------------------------------------------------------------------------
inline float3 Quaternion::Transform(const float3& value) const {
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
inline Quaternion& Quaternion::operator +=(const Quaternion& other) {
    _value += other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline Quaternion& Quaternion::operator -=(const Quaternion& other) {
    _value -= other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::operator +(const Quaternion& other) const {
    return Quaternion(_value + other._value);
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::operator -(const Quaternion& other) const {
    return Quaternion(_value - other._value);
}
//----------------------------------------------------------------------------
inline Quaternion& Quaternion::operator *=(float scale) {
    _value *= scale;
    return *this;
}
//----------------------------------------------------------------------------
inline Quaternion Quaternion::operator *(float scale) const {
    return Quaternion(_value * scale);
}
//----------------------------------------------------------------------------
inline Quaternion& Quaternion::operator *=(const Quaternion& other) {
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
inline Quaternion Quaternion::operator *(const Quaternion& other) const {
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
