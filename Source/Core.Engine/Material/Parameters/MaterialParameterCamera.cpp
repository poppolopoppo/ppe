#include "stdafx.h"

#include "MaterialParameterCamera.h"

#include "Camera/Camera.h"
#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterCamera_EyePosition::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3 eyePosition = context.Scene->Camera()->Model().Parameters().Position;

    const bool changed = (eyePosition != *cached);
    *cached = eyePosition;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_EyeDirection::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3 eyeDirection = context.Scene->Camera()->Model().Parameters().LookAtDir;

    const bool changed = (eyeDirection != *cached);
    *cached = eyeDirection;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_EyeUp::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3 eyeUp = context.Scene->Camera()->Model().Parameters().UpDir;

    const bool changed = (eyeUp != *cached);
    *cached = eyeUp;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_View::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& view = context.Scene->Camera()->Model().View();

    const bool changed = (view != *cached);
    *cached = view;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_InvertView::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& invertView = context.Scene->Camera()->Model().InvertView();

    const bool changed = (invertView != *cached);
    *cached = invertView;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_Projection::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& projection = context.Scene->Camera()->Model().Projection();

    const bool changed = (projection != *cached);
    *cached = projection;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_InvertProjection::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& invertProjection = context.Scene->Camera()->Model().InvertProjection();

    const bool changed = (invertProjection != *cached);
    *cached = invertProjection;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_ViewProjection::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& viewProjection = context.Scene->Camera()->Model().ViewProjection();

    const bool changed = (viewProjection != *cached);
    *cached = viewProjection;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_InvertViewProjection::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const float4x4& invertViewProjection = context.Scene->Camera()->Model().InvertViewProjection();

    const bool changed = (invertViewProjection != *cached);
    *cached = invertViewProjection;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_FrustumRays::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    float3 cameraRays[4];
    context.Scene->Camera()->Model().GetFrustumRays(cameraRays);

    float4x4 raysAsMatrix(0.0f);
    raysAsMatrix.SetColumn(0, cameraRays[size_t(CameraRay::LeftTop)].ZeroExtend());
    raysAsMatrix.SetColumn(1, cameraRays[size_t(CameraRay::LeftBottom)].ZeroExtend());
    raysAsMatrix.SetColumn(2, cameraRays[size_t(CameraRay::RightBottom)].ZeroExtend());
    raysAsMatrix.SetColumn(3, cameraRays[size_t(CameraRay::RightTop)].ZeroExtend());

    const bool changed = (raysAsMatrix != *cached);
    *cached = raysAsMatrix;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_NearFarZ::Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext& context) {
    const CameraModel& cameraModel = context.Scene->Camera()->Model();

    const float2 nearFarZ(cameraModel.Parameters().ZNear, cameraModel.Parameters().ZFar);

    const bool changed = (nearFarZ != *cached);
    *cached = nearFarZ;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_NearCorners::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const MemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    float4x4 pointsAsMatrix(0.0f);
    pointsAsMatrix.SetColumn(0, frustumCorners[size_t(FrustumCorner::Near_LeftTop)].ZeroExtend());
    pointsAsMatrix.SetColumn(1, frustumCorners[size_t(FrustumCorner::Near_LeftBottom)].ZeroExtend());
    pointsAsMatrix.SetColumn(2, frustumCorners[size_t(FrustumCorner::Near_RightBottom)].ZeroExtend());
    pointsAsMatrix.SetColumn(3, frustumCorners[size_t(FrustumCorner::Near_RightTop)].ZeroExtend());

    const bool changed = (pointsAsMatrix != *cached);
    *cached = pointsAsMatrix;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterCamera_FarCorners::Memoize_ReturnIfChanged_(float4x4 *cached, const MaterialContext& context) {
    const MemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    float4x4 pointsAsMatrix(0.0f);
    pointsAsMatrix.SetColumn(0, frustumCorners[size_t(FrustumCorner::Far_LeftTop)].ZeroExtend());
    pointsAsMatrix.SetColumn(1, frustumCorners[size_t(FrustumCorner::Far_LeftBottom)].ZeroExtend());
    pointsAsMatrix.SetColumn(2, frustumCorners[size_t(FrustumCorner::Far_RightBottom)].ZeroExtend());
    pointsAsMatrix.SetColumn(3, frustumCorners[size_t(FrustumCorner::Far_RightTop)].ZeroExtend());

    const bool changed = (pointsAsMatrix != *cached);
    *cached = pointsAsMatrix;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterCameraMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniEyeDirection",          new MaterialParameterCamera_EyeDirection() );
    database->BindParameter("uniEyePosition",           new MaterialParameterCamera_EyePosition() );
    database->BindParameter("uniEyeUp",                 new MaterialParameterCamera_EyeUp() );
    database->BindParameter("uniView",                  new MaterialParameterCamera_View() );
    database->BindParameter("uniInvertView",            new MaterialParameterCamera_InvertView() );
    database->BindParameter("uniProjection",            new MaterialParameterCamera_Projection() );
    database->BindParameter("uniInvertProjection",      new MaterialParameterCamera_InvertProjection() );
    database->BindParameter("uniViewProjection",        new MaterialParameterCamera_InvertView() );
    database->BindParameter("uniInvertViewProjection",  new MaterialParameterCamera_InvertViewProjection() );
    database->BindParameter("uniFrustumRays",           new MaterialParameterCamera_FrustumRays() );
    database->BindParameter("uniNearFarZ",              new MaterialParameterCamera_NearFarZ() );
    database->BindParameter("uniNearCorners",           new MaterialParameterCamera_NearCorners() );
    database->BindParameter("uniFarCorners",            new MaterialParameterCamera_FarCorners() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
