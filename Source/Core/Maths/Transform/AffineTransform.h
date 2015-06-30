#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix_fwd.h"

namespace Core {
class Quaternion;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AffineTransform {
public:
    AffineTransform();
    ~AffineTransform();

    AffineTransform(const float3& position,
                    const float3& direction,
                    const float3& scale );

    AffineTransform(const AffineTransform& other);
    AffineTransform& operator =(const AffineTransform& other);

    const float3& Position() const { return _position; }
    const float3& Direction() const { return _direction; }
    const float3& Scale() const { return _scale; }

    void SetPosition(const float3& value) { _position = value; }
    void SetDirection(const float3& value) { _direction = value; }
    void SetScale(const float3& value) { _scale = value; }

    Quaternion MakeQuaternion() const;
    float4x4 MakeTransformMatrix() const;
    void SetFromMatrix(const float4x4& parMatrix);

    void Transform(const Quaternion& rotation);
    void Transform(const float4x4& transform);

    bool operator ==(const AffineTransform& other) const {
        return  _position == other._position &&
                _direction == other._direction &&
                _scale == other._scale; 
    }

    bool operator !=(const AffineTransform& other) const { return !operator ==(other); }

private:
    float3 _position;
    float3 _direction;
    float3 _scale;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
