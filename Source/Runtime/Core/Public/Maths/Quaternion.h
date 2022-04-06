#pragma once

#include "Core.h"

#include "Maths/ScalarMatrix.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FQuaternion {
public:
    FORCE_INLINE FQuaternion() {}
    FQuaternion(Meta::FForceInit);
    explicit FQuaternion(float broadcast);
    explicit FQuaternion(const float4& value);
    FQuaternion(const float3& value, float w);
    FQuaternion(const float2& xy, float z, float w);
    FQuaternion(float x, float y, float z, float w);

    FQuaternion(const FQuaternion& other);
    FQuaternion& operator =(const FQuaternion& other);

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
    FQuaternion NormalizeInvert() const;

    float3 Transform(const float3& value) const;
    float3 InvertTransform(const float3& value) const;

    bool operator ==(const FQuaternion& other) const { return (data == other.data); }
    bool operator !=(const FQuaternion& other) const { return !operator ==(other); }

    FQuaternion& operator +=(const FQuaternion& other);
    FQuaternion& operator -=(const FQuaternion& other);

    FQuaternion operator +(const FQuaternion& other) const;
    FQuaternion operator -(const FQuaternion& other) const;

    FQuaternion& operator *=(float scale);
    FQuaternion operator *(float scale) const;

    FQuaternion& operator *=(const FQuaternion& other);
    FQuaternion operator *(const FQuaternion& other) const;

    static const FQuaternion Identity;
    static const FQuaternion One;
    static const FQuaternion Zero;

    union {
        float4 data;
        struct {
            // to have FQuaternion.x
            float x, y, z, w;
        };
    };
};
//----------------------------------------------------------------------------
FQuaternion operator *(float scale, const FQuaternion& quaternion);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Quaternions are considered as pods
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FQuaternion)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/Quaternion-inl.h"
