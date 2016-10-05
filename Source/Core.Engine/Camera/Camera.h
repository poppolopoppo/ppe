#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/CameraModel.h"

#include "Core/Maths/ScalarRectangle_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FTimeline;

namespace Engine {
FWD_INTERFACE_REFPTR(CameraProjection);
FWD_INTERFACE_REFPTR(CameraView);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Camera);
class FCamera : public FRefCountable {
public:
    FCamera(float znear, float zfar);

    FCamera(const ICamera& ) = delete;
    FCamera& operator =(const ICamera& ) = delete;

    const FCameraModel& CurrentState() const { return _currentState; }
    const FCameraModel& PreviousState() const { return _previousState; }

    float ZNear() const { return _znear; }
    float ZFar() const { return _zfar; }

    const ICameraView *View() const { return _view.get(); }
    const ICameraProjection *Projection() const { return _projection.get(); }

    void SetZNear(float value);
    void SetZFar(float value);

    void SetView(ICameraView *view);
    void SetProjection(ICameraProjection *projection);

    void Update(const FTimeline& time, const ViewportF& viewport);

private:
    FCameraModel _currentState;

    float _znear;
    float _zfar;

    PCameraView _view;
    PCameraProjection _projection;

    FCameraModel _previousState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
