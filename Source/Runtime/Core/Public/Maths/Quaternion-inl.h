#pragma once

#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(Meta::FForceInit) NOEXCEPT
:   vec(float4::W) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(float broadcast) NOEXCEPT
:   vec(broadcast) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(const float4& value) NOEXCEPT
:   vec(value) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(const float3& value, float w) NOEXCEPT
:   vec(value, w) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(const float2& xy, float z, float w) NOEXCEPT
:   vec(xy, z, w) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(float x, float y, float z, float w) NOEXCEPT
:   vec(x, y, z, w) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion::FQuaternion(const FQuaternion& other) NOEXCEPT
:   vec(other.vec) {}
//----------------------------------------------------------------------------
CONSTEXPR inline FQuaternion& FQuaternion::operator =(const FQuaternion& other) NOEXCEPT {
    vec = other.vec;
    return *this;
}
//----------------------------------------------------------------------------
inline float FQuaternion::Length() const {
    return PPE::Length(vec);
}
//----------------------------------------------------------------------------
inline float FQuaternion::LengthSq() const {
    return PPE::LengthSq(vec);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Conjugate() const {
    return FQuaternion(-x, -y, -z, w);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::Negate() const {
    return FQuaternion(-vec);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator +=(const FQuaternion& other) {
    vec += other.vec;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator -=(const FQuaternion& other) {
    vec -= other.vec;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator +(const FQuaternion& other) const {
    return FQuaternion(vec + other.vec);
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator -(const FQuaternion& other) const {
    return FQuaternion(vec - other.vec);
}
//----------------------------------------------------------------------------
inline FQuaternion& FQuaternion::operator *=(float scale) {
    vec *= scale;
    return *this;
}
//----------------------------------------------------------------------------
inline FQuaternion FQuaternion::operator *(float scale) const {
    return FQuaternion(vec * scale);
}
//----------------------------------------------------------------------------
inline FQuaternion operator *(float scale, const FQuaternion& quaternion) NOEXCEPT {
    return quaternion * scale;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
