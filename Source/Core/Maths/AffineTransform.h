#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix_fwd.h"

namespace Core {
class FQuaternion;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAffineTransform {
public:
    FAffineTransform();
    ~FAffineTransform();

    FAffineTransform(const float3& position,
                    const float3& direction,
                    const float3& scale );

    FAffineTransform(const FAffineTransform& other);
    FAffineTransform& operator =(const FAffineTransform& other);

    const float3& Position() const { return _position; }
    const float3& Direction() const { return _direction; }
    const float3& Scale() const { return _scale; }

    void SetPosition(const float3& value) { _position = value; }
    void SetDirection(const float3& value) { _direction = value; }
    void SetScale(const float3& value) { _scale = value; }

    FQuaternion MakeQuaternion() const;
    float4x4 MakeTransformMatrix() const;
    void SetFromMatrix(const float4x4& parMatrix);

    void Transform(const FQuaternion& rotation);
    void Transform(const float4x4& transform);

    bool operator ==(const FAffineTransform& other) const {
        return  _position == other._position &&
                _direction == other._direction &&
                _scale == other._scale; 
    }

    bool operator !=(const FAffineTransform& other) const { return !operator ==(other); }

private:
    float3 _position;
    float3 _direction;
    float3 _scale;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
