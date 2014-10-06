#pragma once

#include "Core.h"

#include "ScalarMatrix.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Quaternion {
public:
    explicit Quaternion(float broadcast);
    explicit Quaternion(const float4& value);
    Quaternion(const float3& value, float w);
    Quaternion(const float2& xy, float z, float w);
    Quaternion(float x, float y, float z, float w);

    Quaternion(const Quaternion& other);
    Quaternion& operator =(const Quaternion& other);

    const float4& Value() const { return _value; }

    FORCE_INLINE float x() const { return _value.x(); };
    FORCE_INLINE float y() const { return _value.y(); };
    FORCE_INLINE float z() const { return _value.z(); };
    FORCE_INLINE float w() const { return _value.w(); };

    FORCE_INLINE float& x() { return _value.x(); };
    FORCE_INLINE float& y() { return _value.y(); };
    FORCE_INLINE float& z() { return _value.z(); };
    FORCE_INLINE float& w() { return _value.w(); };

    bool IsIdentity() const;
    bool IsNormalized() const;

    float Angle() const;
    float3 Axis() const;

    float Length() const;
    float LengthSq() const;

    Quaternion Conjugate() const;
    Quaternion Exponential() const;
    Quaternion Logarithm() const;
    Quaternion Invert() const;
    Quaternion Negate() const;
    Quaternion Normalize() const;

    float3 Transform(const float3& value) const;

    bool operator ==(const Quaternion& other) const { return _value == other._value; }
    bool operator !=(const Quaternion& other) const { return !operator ==(other); }

    Quaternion& operator +=(const Quaternion& other);
    Quaternion& operator -=(const Quaternion& other);

    Quaternion operator +(const Quaternion& other) const;
    Quaternion operator -(const Quaternion& other) const;

    Quaternion& operator *=(float scale);
    Quaternion operator *(float scale) const;

    Quaternion& operator *=(const Quaternion& other);
    Quaternion operator *(const Quaternion& other) const;

    static Quaternion Identity() { return Quaternion(0,0,0,1); }
    static Quaternion One() { return Quaternion(1,1,1,1); }
    static Quaternion Zero() { return Quaternion(0,0,0,0); }

private:
    float4 _value;
};
//----------------------------------------------------------------------------
Quaternion operator *(float scale, const Quaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Quaternion-inl.h"
