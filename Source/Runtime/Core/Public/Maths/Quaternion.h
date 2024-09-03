#pragma once

#include "Core.h"

#include "Maths/ScalarMatrix.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FQuaternion {
public:
    CONSTEXPR FQuaternion() NOEXCEPT : vec{} {}
    FORCE_INLINE FQuaternion(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR FQuaternion(Meta::FForceInit) NOEXCEPT;
    CONSTEXPR explicit FQuaternion(float broadcast) NOEXCEPT;
    CONSTEXPR explicit FQuaternion(const float4& value) NOEXCEPT;
    CONSTEXPR FQuaternion(const float3& value, float w) NOEXCEPT;
    CONSTEXPR FQuaternion(const float2& xy, float z, float w) NOEXCEPT;
    CONSTEXPR FQuaternion(float x, float y, float z, float w) NOEXCEPT;

    CONSTEXPR FQuaternion(const FQuaternion& other) NOEXCEPT;
    CONSTEXPR FQuaternion& operator =(const FQuaternion& other) NOEXCEPT;

    PPE_CORE_API NODISCARD bool IsIdentity() const;
    PPE_CORE_API NODISCARD bool IsNormalized() const;

    PPE_CORE_API NODISCARD float Angle() const;
    PPE_CORE_API NODISCARD float3 Axis() const;

    NODISCARD float Length() const;
    NODISCARD float LengthSq() const;

    NODISCARD FQuaternion Conjugate() const;
    PPE_CORE_API NODISCARD FQuaternion Exponential() const;
    PPE_CORE_API NODISCARD FQuaternion Logarithm() const;
    PPE_CORE_API NODISCARD FQuaternion Invert() const;
    NODISCARD FQuaternion Negate() const;
    PPE_CORE_API NODISCARD FQuaternion Normalize() const;
    PPE_CORE_API NODISCARD FQuaternion NormalizeInvert() const;

    PPE_CORE_API NODISCARD float3 Transform(const float3& value) const;
    PPE_CORE_API NODISCARD float3 InvertTransform(const float3& value) const;

    NODISCARD bool operator ==(const FQuaternion& other) const { return (vec == other.vec); }
    NODISCARD bool operator !=(const FQuaternion& other) const { return !operator ==(other); }

    FQuaternion& operator +=(const FQuaternion& other);
    FQuaternion& operator -=(const FQuaternion& other);

    NODISCARD FQuaternion operator +(const FQuaternion& other) const;
    NODISCARD FQuaternion operator -(const FQuaternion& other) const;

    FQuaternion& operator *=(float scale);
    NODISCARD FQuaternion operator *(float scale) const;

    FQuaternion& operator *=(const FQuaternion& other);
    NODISCARD FQuaternion operator *(const FQuaternion& other) const;

    static const FQuaternion Identity;
    static const FQuaternion One;
    static const FQuaternion Zero;

    union {
        float4 vec;
        struct {
            // to have FQuaternion.x
            float x, y, z, w;
        };
    };
};
//----------------------------------------------------------------------------
NODISCARD FQuaternion operator *(float scale, const FQuaternion& quaternion) NOEXCEPT;
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
