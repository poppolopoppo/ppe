#include "stdafx.h"

#include "AffineTransform.h"

#include "Core/Maths/ScalarVectorHelpers.h"

#include "Core/Maths/QuaternionHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAffineTransform::FAffineTransform()
:   _position(0.0f)
,   _direction(float3::Forward())
,   _scale(1.0f) {}
//----------------------------------------------------------------------------
FAffineTransform::~FAffineTransform() {}
//----------------------------------------------------------------------------
FAffineTransform::FAffineTransform(
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
FAffineTransform::FAffineTransform(const FAffineTransform& other)
:   _position(other._position)
,   _direction(other._direction)
,   _scale(other._scale) {}
//----------------------------------------------------------------------------
FAffineTransform& FAffineTransform::operator =(const FAffineTransform& other) {
    _position = other._position;
    _direction = other._direction;
    _scale = other._scale;
    return *this;
}
//----------------------------------------------------------------------------
FQuaternion FAffineTransform::MakeQuaternion() const {
    return MakeAxisQuaternion(_direction);
}
//----------------------------------------------------------------------------
float4x4 FAffineTransform::MakeTransformMatrix() const {
    return Make3DRotationMatrix(_direction);
}
//----------------------------------------------------------------------------
void FAffineTransform::SetFromMatrix(const float4x4& parMatrix) {
    FQuaternion rotation;
    Decompose(parMatrix, _scale, rotation, _position);
    _direction = rotation.Transform(float3::Forward());
}
//----------------------------------------------------------------------------
void FAffineTransform::Transform(const FQuaternion& rotation) {
    _direction = rotation.Transform(_direction);
}
//----------------------------------------------------------------------------
void FAffineTransform::Transform(const float4x4& transform) {
    float4x4 t = MakeTransformMatrix();
    t = t.Multiply(transform);
    SetFromMatrix(t);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
