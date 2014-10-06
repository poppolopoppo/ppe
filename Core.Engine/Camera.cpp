#include "stdafx.h"

#include "Camera.h"

#include "Core/ScalarMatrix.h"
#include "Core/ScalarMatrixHelpers.h"
#include "Core/ScalarRectangle.h"

#include "CameraController.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ICamera::ICamera()
:   _projection(float4x4::Identity())
,   _controller(nullptr) {}
//----------------------------------------------------------------------------
ICamera::~ICamera() {}
//----------------------------------------------------------------------------
void ICamera::SetController(ICameraController *controller) {
    _controller = controller;
}
//----------------------------------------------------------------------------
void ICamera::CurrentProjection(float4x4 *projection) const {
    Assert(projection);
    *projection = _projection;
}
//----------------------------------------------------------------------------
void ICamera::Update(const Timeline& time) {
    UpdateImpl(&_projection, time);

    if (_controller)
        _controller->Update(time);

    _model.Update(this, _controller);
}
//----------------------------------------------------------------------------
void ICamera::OnResize(const ViewportF& viewport) {
    OnResizeImpl(viewport);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PerspectiveCamera::PerspectiveCamera(float fov, float near, float far, const ViewportF& viewport)
:   _fov(fov), _near(near), _far(far)
,   _aspectRatio(viewport.AspectRatio()) {
    Assert(0 < fov && F_2PI >= fov);
    Assert(0 <= near);
    Assert(near < far);
    Assert(0 < _aspectRatio);
}
//----------------------------------------------------------------------------
PerspectiveCamera::~PerspectiveCamera() {}
//----------------------------------------------------------------------------
void PerspectiveCamera::UpdateImpl(float4x4 *projection, const Timeline& time) {
    Assert(projection);

    *projection = MakePerspectiveFovLHMatrix(_fov, _aspectRatio, _near, _far);
}
//----------------------------------------------------------------------------
void PerspectiveCamera::OnResizeImpl(const ViewportF& viewport) {
    _aspectRatio = viewport.AspectRatio();
    Assert(0 < _aspectRatio);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
OrthographicOffCenterCamera::OrthographicOffCenterCamera(float near, float far, const ViewportF& viewport)
:   _near(near), _far(far)
,   _left(viewport.Left())
,   _right(viewport.Right())
,   _bottom(viewport.Bottom())
,   _top(viewport.Top()) {
    Assert(0 <= near);
    Assert(near < far);
    Assert(_left < _right);
    Assert(_bottom < _top);
}
//----------------------------------------------------------------------------
OrthographicOffCenterCamera::~OrthographicOffCenterCamera() {}
//----------------------------------------------------------------------------
void OrthographicOffCenterCamera::UpdateImpl(float4x4 *projection, const Timeline& time) {
    Assert(projection);

    *projection = MakeOrthographicOffCenterLHMatrix(_left, _right, _bottom, _top, _near, _far);
}
//----------------------------------------------------------------------------
void OrthographicOffCenterCamera::OnResizeImpl(const ViewportF& viewport) {
    _left = viewport.Left();
    _right = viewport.Right();
    Assert(_left < _right);

    _bottom = viewport.Bottom();
    _top = viewport.Top();
    Assert(_bottom < _top);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
