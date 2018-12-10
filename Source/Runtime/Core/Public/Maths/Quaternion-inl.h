#pragma once

#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(Meta::FForceInit)
:   data(0.0f, 0.0f, 0.0f, 1.0f) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(float broadcast)
:   data(broadcast) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float4& value)
:   data(value) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float3& value, float w)
:   data(value, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const float2& xy, float z, float w)
:   data(xy, z, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(float x, float y, float z, float w)
:   data(x, y, z, w) {}
//----------------------------------------------------------------------------
inline FQuaternion::FQuaternion(const FQuaternion& other)
:   data(other.data) {}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator =(const FQuaternion& other) {
    data = other.data;
    return *this;
}
//----------------------------------------------------------------------------
inline float FQuaternion::Length() const {
    return PPE::Length(data);
}
//----------------------------------------------------------------------------
inline float FQuaternion::LengthSq() const {
    return PPE::LengthSq(data);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Conjugate() const {
    return FQuaternion(-x, -y, -z, w);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Negate() const {
    return FQuaternion(-data);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator +=(const FQuaternion& other) {
    data += other.data;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator -=(const FQuaternion& other) {
    data -= other.data;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator +(const FQuaternion& other) const {
    return FQuaternion(data + other.data);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator -(const FQuaternion& other) const {
    return FQuaternion(data - other.data);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator *=(float scale) {
    data *= scale;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator *(float scale) const {
    return FQuaternion(data * scale);
}
//----------------------------------------------------------------------------
inline FQuaternion operator *(float scale, const FQuaternion& quaternion) {
    return quaternion * scale;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
