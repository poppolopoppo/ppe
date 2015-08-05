#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/CameraModel.h"

#include "Core/Maths/Geometry/ScalarRectangle_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Engine {
FWD_INTERFACE_REFPTR(CameraProjection);
FWD_INTERFACE_REFPTR(CameraView);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Camera);
class Camera : public RefCountable {
public:
    Camera(float znear, float zfar);

    Camera(const ICamera& ) = delete;
    Camera& operator =(const ICamera& ) = delete;

    const CameraModel& CurrentState() const { return _currentState; }
    const CameraModel& PreviousState() const { return _previousState; }

    float ZNear() const { return _znear; }
    float ZFar() const { return _zfar; }

    const ICameraView *View() const { return _view.get(); }
    const ICameraProjection *Projection() const { return _projection.get(); }

    void SetZNear(float value);
    void SetZFar(float value);

    void SetView(ICameraView *view);
    void SetProjection(ICameraProjection *projection);

    void Update(const Timeline& time, const ViewportF& viewport);

private:
    CameraModel _currentState;

    float _znear;
    float _zfar;

    PCameraView _view;
    PCameraProjection _projection;

    CameraModel _previousState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
