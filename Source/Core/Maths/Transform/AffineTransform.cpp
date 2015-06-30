#include "stdafx.h"

#include "AffineTransform.h"

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

#include "Core/Maths/Transform/QuaternionHelpers.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AffineTransform::AffineTransform()
:   _position(0.0f)
,   _direction(float3::Forward())
,   _scale(1.0f) {}
//----------------------------------------------------------------------------
AffineTransform::~AffineTransform() {}
//----------------------------------------------------------------------------
AffineTransform::AffineTransform(
    const float3& position,
    const float3& direction,
    const float3& scale )
:   _position(position)
,   _direction(direction)
,   _scale(scale) {
    Assert(IsNormalized(_direction));
    Assert(_scale.AllGreaterThan(float3::Zero()));
}
//----------------------------------------------------------------------------
AffineTransform::AffineTransform(const AffineTransform& other)
:   _position(other._position)
,   _direction(other._direction)
,   _scale(other._scale) {}
//----------------------------------------------------------------------------
AffineTransform& AffineTransform::operator =(const AffineTransform& other) {
    _position = other._position;
    _direction = other._direction;
    _scale = other._scale;
    return *this;
}
//----------------------------------------------------------------------------
Quaternion AffineTransform::MakeQuaternion() const {
    return MakeAxisQuaternion(_direction);
}
//----------------------------------------------------------------------------
float4x4 AffineTransform::MakeTransformMatrix() const {
    return Make3DRotationMatrix(_direction);
}
//----------------------------------------------------------------------------
void AffineTransform::SetFromMatrix(const float4x4& parMatrix) {
    Quaternion rotation;
    Decompose(parMatrix, _scale, rotation, _position);
    _direction = rotation.Transform(float3::Forward());
}
//----------------------------------------------------------------------------
void AffineTransform::Transform(const Quaternion& rotation) {
    _direction = rotation.Transform(_direction);
}
//----------------------------------------------------------------------------
void AffineTransform::Transform(const float4x4& transform) {
    float4x4 t = MakeTransformMatrix();
    t = t.Multiply(transform);
    SetFromMatrix(t);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
