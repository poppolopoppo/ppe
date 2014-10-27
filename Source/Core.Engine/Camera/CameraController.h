#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ICameraController : public RefCountable {
public:
    ICameraController();
    virtual ~ICameraController();

    ICameraController(const ICameraController& ) = delete;
    ICameraController& operator =(const ICameraController& ) = delete;

    void CurrentView(float4x4 *view) const;
    void Update(const Timeline& time);

protected:
    virtual void UpdateImpl(float4x4 *view, const Timeline& time) = 0;

private:
    float4x4 _view;
};
//----------------------------------------------------------------------------
typedef RefPtr<ICameraController> PCameraController;
typedef RefPtr<const ICameraController> PCCameraController;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FreeLookCameraController);
class FreeLookCameraController : public ICameraController {
public:
    FreeLookCameraController(const float3& position, float heading/* rad */, float pitch/* rad */);
    virtual ~FreeLookCameraController();

    const float3& Position() const { return _position; }
    float Heading() const { return _heading; }
    float Pitch() const { return _pitch; }

    const float3& Up() const { return _up; }
    const float3& Forward() const { return _forward; }
    const float3& Right() const { return _right; }

    void LookAt(const float3& position, float heading/* rad */, float pitch/* rad */);

protected:
    virtual void UpdateImpl(float4x4 *view, const Timeline& time) override;

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
