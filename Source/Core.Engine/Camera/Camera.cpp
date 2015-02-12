#include "stdafx.h"

#include "Camera.h"

#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

#include "CameraController.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ICamera::ICamera(float znear, float zfar)
:   _projection(float4x4::Identity())
,   _controller(nullptr)
,   _znear(znear)
,   _zfar(zfar) {
    Assert(0 <= znear);
    Assert(znear < zfar);
}
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
PerspectiveCamera::PerspectiveCamera(float fov, float znear, float zfar, const ViewportF& viewport)
:   ICamera(znear, zfar)
,   _fov(fov)
,   _aspectRatio(viewport.AspectRatio()) {
    Assert(0 < fov && F_2PI >= fov);
    Assert(0 < _aspectRatio);
}
//----------------------------------------------------------------------------
PerspectiveCamera::~PerspectiveCamera() {}
//----------------------------------------------------------------------------
void PerspectiveCamera::UpdateImpl(float4x4 *projection, const Timeline& time) {
    Assert(projection);

    *projection = MakePerspectiveFovLHMatrix(_fov, _aspectRatio, _znear, _zfar);
}
//----------------------------------------------------------------------------
void PerspectiveCamera::OnResizeImpl(const ViewportF& viewport) {
    _aspectRatio = viewport.AspectRatio();
    Assert(0 < _aspectRatio);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
OrthographicOffCenterCamera::OrthographicOffCenterCamera(float znear, float zfar, const ViewportF& viewport)
:   ICamera(znear, zfar)
,   _left(viewport.Left())
,   _right(viewport.Right())
,   _bottom(viewport.Bottom())
,   _top(viewport.Top()) {
    Assert(_left < _right);
    Assert(_bottom < _top);
}
//----------------------------------------------------------------------------
OrthographicOffCenterCamera::~OrthographicOffCenterCamera() {}
//----------------------------------------------------------------------------
void OrthographicOffCenterCamera::UpdateImpl(float4x4 *projection, const Timeline& time) {
    Assert(projection);

    *projection = MakeOrthographicOffCenterLHMatrix(_left, _right, _bottom, _top, _znear, _zfar);
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
