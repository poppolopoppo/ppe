#pragma once

#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(Meta::FForceInit)
:   _value(0.0f, 0.0f, 0.0f, 1.0f) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(float broadcast)
:   _value(broadcast) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float4& value)
:   _value(value) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float3& value, float w)
:   _value(value, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float2& xy, float z, float w)
:   _value(xy.x(), xy.y(), z, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(float x, float y, float z, float w)
:   _value(x, y, z, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const FQuaternion& other)
:   _value(other._value) {}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator =(const FQuaternion& other) {
    _value = other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline float FQuaternion::Length() const {
    return Length4(_value);
}
//----------------------------------------------------------------------------
inline float FQuaternion::LengthSq() const {
    return LengthSq4(_value);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Conjugate() const {
    return FQuaternion(-x(), -y(), -z(), w());
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Negate() const {
    return FQuaternion(-_value);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator +=(const FQuaternion& other) {
    _value += other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator -=(const FQuaternion& other) {
    _value -= other._value;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator +(const FQuaternion& other) const {
    return FQuaternion(_value + other._value);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator -(const FQuaternion& other) const {
    return FQuaternion(_value - other._value);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator *=(float scale) {
    _value *= scale;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator *(float scale) const {
    return FQuaternion(_value * scale);
}
//----------------------------------------------------------------------------
inline FQuaternion operator *(float scale, const FQuaternion& quaternion) {
    return quaternion * scale;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
