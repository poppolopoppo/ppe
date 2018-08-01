#include "stdafx.h"

#include "Camera.h"

#include "ICameraView.h"
#include "ICameraProjection.h"

#include "Core/Maths/ScalarRectangle.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCamera::FCamera(float znear, float zfar)
:   _znear(znear)
,   _zfar(zfar) {
    Assert(0 <= znear);
    Assert(znear < zfar);
}
//----------------------------------------------------------------------------
FCamera::~FCamera() {}
//----------------------------------------------------------------------------
void FCamera::SetController(CameraController *controller) {
    _controller = controller;
}
//----------------------------------------------------------------------------
void FCamera::CurrentProjection(float4x4 *projection) const {
    Assert(projection);
    *projection = _projection;
}
//----------------------------------------------------------------------------
void FCamera::Update(const FTimeline& time) {
    UpdateImpl(&_projection, time);

    if (_controller)
        _controller->Update(time);

    _model.Update(this, _controller);
}
//----------------------------------------------------------------------------
void FCamera::OnResize(const ViewportF& viewport) {
    OnResizeImpl(viewport);
}
//----------------------------------------------------------------------------
void FCamera::SetZNear(float value) {
    Assert(0 <= value);
    _znear = value;
}
//----------------------------------------------------------------------------
void FCamera::SetZFar(float value) {
    Assert(0 < value);
    _zfar = value;
}
//----------------------------------------------------------------------------
void FCamera::SetView(ICameraView *view) {
    _view.reset(view);
}
//----------------------------------------------------------------------------
void FCamera::SetProjection(ICameraProjection *projection) {
    _projection.reset(projection);
}
//----------------------------------------------------------------------------
void FCamera::Update(const FTimeline& time, const ViewportF& viewport) {
    const float4x4 view = (_view)
        ? _view->ViewMatrix(time)
        : float4x4:Identity();

    const float4x4 projection = (_projection)
        ? _projection->ProjectionMatrix(time, _znear, _zfar, viewport)
        : float4x4:Identity();

    FCameraModel tmp;
    _currentState.CopyTo(&tmp);
    _currentState.Update(view, projection);

    if (false == _currentState.Equals(tmp))
        tmp.CopyTo(_previousState);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
