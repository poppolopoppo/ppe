#pragma once

#include "Core.h"

#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector.h"
#include "Maths/Quaternion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FTransform {
public:
    FORCE_INLINE FTransform()
#if USE_PPE_ASSERT
        : _rotation(NAN, NAN, NAN, NAN)
        , _translation(NAN)
        , _scale(NAN)
#endif
    {}

    explicit FTransform(Meta::FForceInit)
        : _rotation(0.f, 0.f, 0.f, 1.f)
        , _translation(0.f)
        , _scale(1.f)
    {}

    explicit FTransform(const float3& translation)
        : FTransform(FQuaternion(0.f, 0.f, 0.f, 1.f), translation, float3(1.f))
    {}
    explicit FTransform(const FQuaternion& rotation)
        : FTransform(rotation, float3(0.f), float3(1.f))
    {}
    FTransform(const FQuaternion& rotation, const float3& translation, float scale = 1.f)
        : FTransform(rotation, translation, float3(scale))
    {}
    FTransform(const FQuaternion& rotation, const float3& translation, const float3& scale = float3(1.f))
        : _rotation(rotation)
        , _translation(translation)
        , _scale(scale)
    {}

    const FQuaternion& Rotation() const { return _rotation; }
    void SetRotation(const FQuaternion& rotation) { _rotation = rotation; }

    const float3& Translation() const { return _translation; }
    void SetTranslation(const float3& translation) { _translation = translation; }

    const float3& Scale() const { return _scale; }
    void SetScale(const float3& scale) { _scale = scale; }

    static const FTransform Identity;
    bool IsIdentity() const { return Equals(Identity); }

    float Determinant() const { return (_scale.x * _scale.y * _scale.z); }
    float Sign() const { return (Determinant() < 0 ? -1.f : 1.f); }

    float3 TransformPosition(const float3& p) const;
    float3 TransformPositionNoScale(const float3& p) const;
    float3 TransformVector(const float3& v) const;
    float3 TransformVectorNoScale(const float3& v) const;
    float4 Transform(const float4& v) const;
    float4 TransformNoScale(const float4& v) const;

    FTransform Invert() const;

    float3 InvertTransformPosition(const float3& p) const;
    float3 InvertTransformPositionNoScale(const float3& p) const;
    float3 InvertTransformVector(const float3& v) const;
    float3 InvertTransformVectorNoScale(const float3& v) const;
    float4 InvertTransform(const float4& v) const;
    float4 InvertTransformNoScale(const float4& v) const;

    float4x4 ToMatrixNoScale() const;
    float4x4 ToMatrixWithScale() const;
    float4x4 ToInvertMatrixWithScale() const;

    bool Equals(const FTransform& other, float espilon = F_Epsilon) const;

    FTransform& Accumulate(const FTransform& delta);
    FTransform& Accumulate(const FTransform& delta, float blendWeight) { return Accumulate(Multiply(delta, blendWeight)); }
    FTransform& AccumulateWithShortestRotation(const FTransform& delta);
    FTransform& AccumulateWithShortestRotation(const FTransform& delta, float blendWeight) { return AccumulateWithShortestRotation(Multiply(delta, blendWeight)); }
    FTransform& AccumulateWithAdditiveScale(const FTransform& delta);
    FTransform& AccumulateWithAdditiveScale(const FTransform& delta, float blendWeight) { return AccumulateWithAdditiveScale(Multiply(delta, blendWeight)); }

    static FTransform Blend(const FTransform& lhs, const FTransform& rhs, float alpha);
    static FTransform RelativeTransform(const FTransform& from, const FTransform& to);
    static FTransform FromMatrix(const float4x4& transform);

    static FTransform Multiply(const FTransform& transform, float scale);
    static FTransform Multiply(const FTransform& lhs, const FTransform& rhs);

protected:
    FQuaternion _rotation;
    float3 _translation;
    float3 _scale;
};
//----------------------------------------------------------------------------
bool PPE_CORE_API IsINF(const FTransform& transform);
bool PPE_CORE_API IsNAN(const FTransform& transform);
bool PPE_CORE_API IsNANorINF(const FTransform& transform);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All FTransform are considered as pods
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FTransform)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
