#include "stdafx.h"

#include "Maths/Transform.h"

#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/QuaternionHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FTransform FTransform::Identity(
    FQuaternion::Identity,
    float3::Zero,
    float3::One );
//----------------------------------------------------------------------------
float3 FTransform::TransformPosition(const float3& p) const {
    Assert(not IsNAN(*this));
    return _rotation.Transform(p * _scale) + _translation;
}
//----------------------------------------------------------------------------
float3 FTransform::TransformPositionNoScale(const float3& p) const {
    Assert(not IsNAN(*this));
    return _rotation.Transform(p) + _translation;
}
//----------------------------------------------------------------------------
float3 FTransform::TransformVector(const float3& v) const {
    Assert(not IsNAN(*this));
    return _rotation.Transform(v * _scale);
}
//----------------------------------------------------------------------------
float3 FTransform::TransformVectorNoScale(const float3& v) const {
    Assert(not IsNAN(*this));
    return _rotation.Transform(v);
}
//----------------------------------------------------------------------------
float4 FTransform::Transform(const float4& v) const {
    Assert(not IsNAN(*this));
    Assert(v.w == 0 || v.w == 1); // or it won't work

    return ((v.w == 1.f)
        ? float4(TransformPosition(v.xyz), 1.f)
        : float4(TransformVector(v.xyz), 0.f) );
}
//----------------------------------------------------------------------------
float4 FTransform::TransformNoScale(const float4& v) const {
    Assert(not IsNAN(*this));
    Assert(v.w == 0 || v.w == 1); // or it won't work

    return ((v.w == 1.f)
        ? float4(TransformPositionNoScale(v.xyz), 1.f)
        : float4(TransformVectorNoScale(v.xyz), 0.f));
}
//----------------------------------------------------------------------------
FTransform FTransform::Invert() const {
    Assert(not IsNAN(*this));

    FTransform result;
    result._rotation = _rotation.Invert();
    result._scale = Rcp(_scale);
    result._translation = result._rotation.Transform(result._scale * -_translation);
    return result;
}
//----------------------------------------------------------------------------
float3 FTransform::InvertTransformPosition(const float3& p) const {
    Assert(not IsNAN(*this));

    return _rotation.InvertTransform(p - _translation) * Rcp(_scale);
}
//----------------------------------------------------------------------------
float3 FTransform::InvertTransformPositionNoScale(const float3& p) const {
    Assert(not IsNAN(*this));

    return _rotation.InvertTransform(p - _translation);
}
//----------------------------------------------------------------------------
float3 FTransform::InvertTransformVector(const float3& v) const {
    Assert(not IsNAN(*this));

    return _rotation.InvertTransform(v) * Rcp(_scale);
}
//----------------------------------------------------------------------------
float3 FTransform::InvertTransformVectorNoScale(const float3& v) const {
    Assert(not IsNAN(*this));

    return _rotation.InvertTransform(v);
}
//----------------------------------------------------------------------------
float4 FTransform::InvertTransform(const float4& v) const {
    Assert(not IsNAN(*this));
    Assert(v.w == 0 || v.w == 1); // or it won't work

    return ((v.w == 1.f)
        ? float4(InvertTransformPosition(v.xyz), 1.f)
        : float4(InvertTransformVector(v.xyz), 0.f) );
}
//----------------------------------------------------------------------------
float4 FTransform::InvertTransformNoScale(const float4& v) const {
    Assert(not IsNAN(*this));
    Assert(v.w == 0 || v.w == 1); // or it won't work

    return ((v.w == 1.f)
        ? float4(InvertTransformPositionNoScale(v.xyz), 1.f)
        : float4(InvertTransformVectorNoScale(v.xyz), 0.f) );
}
//----------------------------------------------------------------------------
float4x4 FTransform::ToMatrixNoScale() const {
    Assert(not IsNAN(*this));

    return Make3DTransformMatrix(_translation, 1.f, _rotation);
}
//----------------------------------------------------------------------------
float4x4 FTransform::ToMatrixWithScale() const {
    Assert(not IsNAN(*this));

    return Make3DTransformMatrix(_translation, _scale, _rotation);
}
//----------------------------------------------------------------------------
float4x4 FTransform::ToInvertMatrixWithScale() const {
    Assert(not IsNAN(*this));

    return Invert().ToMatrixWithScale();
}
//----------------------------------------------------------------------------
bool FTransform::Equals(const FTransform& other, float espilon/* = F_Epsilon */) const {
    return (NearlyEquals(_rotation.data, other._rotation.data, espilon) &&
            NearlyEquals(_translation, other._translation, espilon) &&
            NearlyEquals(_scale, other._scale, espilon) );
}
//----------------------------------------------------------------------------
FTransform& FTransform::Accumulate(const FTransform& delta) {
    Assert(not IsNAN(*this));

    if (Sqr(delta._rotation.w) < 1.f - F_Delta * F_Delta)
        _rotation = delta._rotation * _rotation;

    _translation += delta._translation;
    _scale *= delta._scale;

    return (*this);
}
//----------------------------------------------------------------------------
FTransform& FTransform::AccumulateWithShortestRotation(const FTransform& delta) {
    Assert(not IsNAN(*this));

    if (Dot(_rotation.data, delta._rotation.data) < 0.f) {
        _rotation.x -= delta._rotation.x;
        _rotation.y -= delta._rotation.y;
        _rotation.z -= delta._rotation.z;
        _rotation.w -= delta._rotation.w;
    }
    else {
        _rotation.x += delta._rotation.x;
        _rotation.y += delta._rotation.y;
        _rotation.z += delta._rotation.z;
        _rotation.w += delta._rotation.w;
    }

    _translation += delta._translation;
    _scale *= delta._scale;

    return (*this);
}
//----------------------------------------------------------------------------
FTransform& FTransform::AccumulateWithAdditiveScale(const FTransform& delta) {
    Assert(not IsNAN(*this));

    if (Sqr(delta._rotation.w) < 1.f - F_Delta * F_Delta)
        _rotation = delta._rotation * _rotation;

    _translation += delta._translation;
    _scale *= (1.f + delta._scale);

    return (*this);
}
//----------------------------------------------------------------------------
FTransform FTransform::Blend(const FTransform& lhs, const FTransform& rhs, float alpha) {
    Assert(lhs._rotation.IsNormalized());
    Assert(rhs._rotation.IsNormalized());

    if (alpha <= F_LargeEpsilon) {
        return lhs;
    }
    else if (alpha >= 1.f - F_LargeEpsilon) {
        return rhs;
    }
    else {
        return FTransform(
            Lerp(lhs._rotation, rhs._rotation, alpha),
            Lerp(lhs._translation, rhs._translation, alpha),
            Lerp(lhs._scale, rhs._scale, alpha) );
    }
}
//----------------------------------------------------------------------------
FTransform FTransform::FromMatrix(const float4x4& transform) {
    FTransform result;
    Decompose(transform, result._scale, result._rotation, result._translation);
    return result;
}
//----------------------------------------------------------------------------
FTransform FTransform::RelativeTransform(const FTransform& from, const FTransform& to) {
    Assert(not IsNAN(from));
    Assert(not IsNAN(to));
    Assert(from._rotation.IsNormalized());
    Assert(to._rotation.IsNormalized());

    if (from.Sign() < 0 || to.Sign() < 0)
        return FromMatrix(from.ToMatrixWithScale().Multiply(to.ToInvertMatrixWithScale()));

    const float3 invToScale = Rcp(to._scale);
    const FQuaternion invToRotation = to._rotation.Invert();

    FTransform result;
    result._scale = from._scale * invToScale;
    result._rotation = invToRotation * from._rotation;
    result._translation = invToRotation.Transform(from._translation - to._translation) * invToScale;
    return result;
}
//----------------------------------------------------------------------------
FTransform FTransform::Multiply(const FTransform& transform, float scale) {
    Assert(not IsNAN(transform));

    return FTransform(
        transform._rotation * scale,
        transform._translation * scale,
        transform._scale *  scale);
}
//----------------------------------------------------------------------------
FTransform FTransform::Multiply(const FTransform& lhs, const FTransform& rhs) {
    Assert(not IsNAN(lhs));
    Assert(not IsNAN(rhs));
    Assert(lhs._rotation.IsNormalized());
    Assert(rhs._rotation.IsNormalized());

    if (lhs.Sign() < 0 || rhs.Sign() < 0)
        return FromMatrix(lhs.ToMatrixWithScale().Multiply(rhs.ToMatrixWithScale()));

    FTransform result;
    result._rotation = rhs._rotation * lhs._rotation;
    result._scale = lhs._scale * rhs._scale;
    result._translation = rhs._rotation.Transform(rhs._scale * lhs._translation) + rhs._translation;
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsINF(const FTransform& transform) {
    return (IsINF(transform.Rotation()) ||
            IsINF(transform.Translation()) ||
            IsINF(transform.Scale()) );
}
//----------------------------------------------------------------------------
bool IsNAN(const FTransform& transform) {
    return (IsNAN(transform.Rotation()) ||
            IsNAN(transform.Translation()) ||
            IsNAN(transform.Scale()) );
}
//----------------------------------------------------------------------------
bool IsNANorINF(const FTransform& transform) {
    return (IsNANorINF(transform.Rotation()) ||
            IsNANorINF(transform.Translation()) ||
            IsNANorINF(transform.Scale()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
