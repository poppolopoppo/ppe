#pragma once

#include "Core/Maths/Transform/Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
    //----------------------------------------------------------------------------
inline Quaternion::Quaternion()
:   _value(0.0f, 0.0f, 0.0f, 1.0f) {}
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
inline Quaternion Quaternion::Negate() const {
    return Quaternion(-_value);
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
inline Quaternion operator *(float scale, const Quaternion& quaternion) {
    return quaternion * scale;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
