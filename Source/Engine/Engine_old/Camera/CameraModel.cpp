// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "CameraModel.h"

#include "Core/Maths/ScalarMatrixHelpers.h"

#include "Camera.h"
#include "CameraController.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCameraModel::FCameraModel() {
    _view = _projection = _viewProjection = float4x4::Identity();
    _invertView = _invertProjection = _invertViewProjection = float4x4::Identity();
    _frustum.GetCorners(_frustumCorners);
    _frustum.GetCameraParams(_parameters);
}
//----------------------------------------------------------------------------
FCameraModel::~FCameraModel() {}
//----------------------------------------------------------------------------
void FCameraModel::GetFrustumRays(const TMemoryView<float3>& rays) const {
    Assert(4 == rays.size());

    rays[size_t(ECameraRay::LeftTop)]        = _frustumCorners[size_t(EFrustumCorner::Far_LeftTop)]       - _frustumCorners[size_t(EFrustumCorner::Near_LeftTop)];
    rays[size_t(ECameraRay::LeftBottom)]     = _frustumCorners[size_t(EFrustumCorner::Far_LeftBottom)]    - _frustumCorners[size_t(EFrustumCorner::Near_LeftBottom)];
    rays[size_t(ECameraRay::RightBottom)]    = _frustumCorners[size_t(EFrustumCorner::Far_RightBottom)]   - _frustumCorners[size_t(EFrustumCorner::Near_RightBottom)];
    rays[size_t(ECameraRay::RightTop)]       = _frustumCorners[size_t(EFrustumCorner::Far_RightTop)]      - _frustumCorners[size_t(EFrustumCorner::Near_RightTop)];
}
//----------------------------------------------------------------------------
void FCameraModel::Update(const float4x4& view, const float4x4& projection) {
    _view = view;
    _projection = projection;
    _viewProjection = _view.Multiply(_projection);

    _invertView = Invert(_view);
    _invertProjection = Invert(_projection);
    _invertViewProjection = Invert(_viewProjection);

    _frustum.SetMatrix(_viewProjection);
    _frustum.GetCorners(_frustumCorners);
    _frustum.GetCameraParams(_parameters);
}
//----------------------------------------------------------------------------
void FCameraModel::CopyTo(FCameraModel *dst) const {
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
bool FCameraModel::Equals(const FCameraModel& other) const {
    const bool result = _view == other->_view &&
                        _projection == other->_projection;
#ifdef WITH_CORE_ASSERT
    if (result) {
        Assert(_viewProjection == other->_viewProjection);
        Assert(_invertView == other->_invertView);
        Assert(_invertProjection == other->_invertProjection);
        Assert(_invertViewProjection == other->_invertViewProjection);
        Assert(_frustum == other._frustum);
    }
#endif
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
