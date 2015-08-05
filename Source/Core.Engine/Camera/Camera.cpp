#include "stdafx.h"

#include "Camera.h"

#include "ICameraView.h"
#include "ICameraProjection.h"

#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Camera::Camera(float znear, float zfar)
:   _znear(znear)
,   _zfar(zfar) {
    Assert(0 <= znear);
    Assert(znear < zfar);
}
//----------------------------------------------------------------------------
Camera::~Camera() {}
//----------------------------------------------------------------------------
void Camera::SetController(CameraController *controller) {
    _controller = controller;
}
//----------------------------------------------------------------------------
void Camera::CurrentProjection(float4x4 *projection) const {
    Assert(projection);
    *projection = _projection;
}
//----------------------------------------------------------------------------
void Camera::Update(const Timeline& time) {
    UpdateImpl(&_projection, time);

    if (_controller)
        _controller->Update(time);

    _model.Update(this, _controller);
}
//----------------------------------------------------------------------------
void Camera::OnResize(const ViewportF& viewport) {
    OnResizeImpl(viewport);
}
//----------------------------------------------------------------------------
void Camera::SetZNear(float value) {
    Assert(0 <= value);
    _znear = value;
}
//----------------------------------------------------------------------------
void Camera::SetZFar(float value) {
    Assert(0 < value);
    _zfar = value;
}
//----------------------------------------------------------------------------
void Camera::SetView(ICameraView *view) {
    _view.reset(view);
}
//----------------------------------------------------------------------------
void Camera::SetProjection(ICameraProjection *projection) {
    _projection.reset(projection);
}
//----------------------------------------------------------------------------
void Camera::Update(const Timeline& time, const ViewportF& viewport) {
    const float4x4 view = (_view)
        ? _view->ViewMatrix(time)
        : float4x4:Identity();

    const float4x4 projection = (_projection)
        ? _projection->ProjectionMatrix(time, _znear, _zfar, viewport)
        : float4x4:Identity();

    CameraModel tmp;
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
