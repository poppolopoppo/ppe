#pragma once

#include "Core.Engine/Engine.h"

#include "Camera/ICameraView.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FreeLookView);
class FreeLookView : public ICameraView {
public:
    FreeLookView(const float3& position, float heading/* rad */, float pitch/* rad */);
    virtual ~FreeLookView();

    const float3& Position() const { return _position; }
    float Heading() const { return _heading; }
    float Pitch() const { return _pitch; }

    const float3& Up() const { return _up; }
    const float3& Forward() const { return _forward; }
    const float3& Right() const { return _right; }

    void LookAt(const float3& position, float heading/* rad */, float pitch/* rad */);

    virtual float4x4 ViewMatrix(const Timeline& time) override;

private:
    float3 _position;
    float _heading;
    float _pitch;

    float3 _up;
    float3 _forward;
    float3 _right;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
