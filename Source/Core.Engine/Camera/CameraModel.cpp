#include "stdafx.h"

#include "CameraModel.h"

#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

#include "Camera.h"
#include "CameraController.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CameraModel::CameraModel() {
    _view = _projection = _viewProjection = float4x4::Identity();
    _invertView = _invertProjection = _invertViewProjection = float4x4::Identity();

    _frustum.GetCorners(_frustumCorners);
    _frustum.GetCameraParams(_parameters);
}
//----------------------------------------------------------------------------
CameraModel::~CameraModel() {}
//----------------------------------------------------------------------------
void CameraModel::GetFrustumRays(const MemoryView<float3>& rays) const {
    Assert(4 == rays.size());

    rays[size_t(CameraRay::LeftTop)]        = _frustumCorners[size_t(FrustumCorner::Far_LeftTop)]       - _frustumCorners[size_t(FrustumCorner::Near_LeftTop)];
    rays[size_t(CameraRay::LeftBottom)]     = _frustumCorners[size_t(FrustumCorner::Far_LeftBottom)]    - _frustumCorners[size_t(FrustumCorner::Near_LeftBottom)];
    rays[size_t(CameraRay::RightBottom)]    = _frustumCorners[size_t(FrustumCorner::Far_RightBottom)]   - _frustumCorners[size_t(FrustumCorner::Near_RightBottom)];
    rays[size_t(CameraRay::RightTop)]       = _frustumCorners[size_t(FrustumCorner::Far_RightTop)]      - _frustumCorners[size_t(FrustumCorner::Near_RightTop)];
}
//----------------------------------------------------------------------------
void CameraModel::Update(const ICamera *camera, const ICameraController *controller) {
    Assert(camera);
    Assert(controller);

    controller->CurrentView(&_view);
    camera->CurrentProjection(&_projection);

    _viewProjection = _view.Multiply(_projection);

    _invertView = Invert(_view);
    _invertProjection = Invert(_projection);
    _invertViewProjection = Invert(_viewProjection);

    _frustum.SetMatrix(_viewProjection);
    _frustum.GetCorners(_frustumCorners);
    _frustum.GetCameraParams(_parameters);
}
//----------------------------------------------------------------------------
void CameraModel::CopyTo(CameraModel *dst) const {
    Assert(dst);

    dst->_view = _view;
    dst->_projection = _projection;
    dst->_viewProjection = _viewProjection;

    dst->_invertView = _invertView;
    dst->_invertProjection = _invertProjection;
    dst->_invertViewProjection = _invertViewProjection;

    dst->_frustum = _frustum;
    for (size_t i = 0; i < lengthof(_frustumCorners); ++i)
        dst->_frustumCorners[i] = _frustumCorners[i];

    dst->_parameters = _parameters;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
