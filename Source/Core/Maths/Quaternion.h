#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarMatrix.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FQuaternion {
public:
    FORCE_INLINE  FQuaternion() {}
    FQuaternion(Meta::FForceInit);
    explicit FQuaternion(float broadcast);
    explicit FQuaternion(const float4& value);
    FQuaternion(const float3& value, float w);
    FQuaternion(const float2& xy, float z, float w);
    FQuaternion(float x, float y, float z, float w);

    FQuaternion(const FQuaternion& other);
    FQuaternion& operator =(const FQuaternion& other);

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

    FQuaternion Conjugate() const;
    FQuaternion Exponential() const;
    FQuaternion Logarithm() const;
    FQuaternion Invert() const;
    FQuaternion Negate() const;
    FQuaternion Normalize() const;

    float3 Transform(const float3& value) const;

    bool operator ==(const FQuaternion& other) const { return _value == other._value; }
    bool operator !=(const FQuaternion& other) const { return !operator ==(other); }

    FQuaternion& operator +=(const FQuaternion& other);
    FQuaternion& operator -=(const FQuaternion& other);

    FQuaternion operator +(const FQuaternion& other) const;
    FQuaternion operator -(const FQuaternion& other) const;

    FQuaternion& operator *=(float scale);
    FQuaternion operator *(float scale) const;

    FQuaternion& operator *=(const FQuaternion& other);
    FQuaternion operator *(const FQuaternion& other) const;

    FORCE_INLINE static FQuaternion Identity() { return FQuaternion(0,0,0,1); }
    FORCE_INLINE static FQuaternion One() { return FQuaternion(1,1,1,1); }
    FORCE_INLINE static FQuaternion Zero() { return FQuaternion(0,0,0,0); }

private:
    float4 _value;
};
//----------------------------------------------------------------------------
FQuaternion operator *(float scale, const FQuaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Quaternion-inl.h"
