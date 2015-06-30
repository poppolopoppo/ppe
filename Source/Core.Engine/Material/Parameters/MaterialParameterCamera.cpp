#include "stdafx.h"

#include "MaterialParameterCamera.h"

#include "Camera/Camera.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_CAMERA(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_CAMERA(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// Camera Model
//----------------------------------------------------------------------------
void EyePosition(const MaterialParameterContext& context, float3& dst) {
    dst = context.Scene->Camera()->Model().Parameters().Position;
}
//----------------------------------------------------------------------------
void EyeDirection(const MaterialParameterContext& context, float3& dst) {
    dst = context.Scene->Camera()->Model().Parameters().LookAtDir;
}
//----------------------------------------------------------------------------
void EyeUp(const MaterialParameterContext& context, float3& dst) {
    dst = context.Scene->Camera()->Model().Parameters().UpDir;
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// View
//----------------------------------------------------------------------------
void View(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().View();
}
//----------------------------------------------------------------------------
void InvertView(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().InvertView();
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// Projection
//----------------------------------------------------------------------------
void Projection(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().Projection();
}
//----------------------------------------------------------------------------
void InvertProjection(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().InvertProjection();
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// View Projection
//----------------------------------------------------------------------------
void ViewProjection(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().ViewProjection();
}
//----------------------------------------------------------------------------
void InvertViewProjection(const MaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().InvertViewProjection();
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// Frustum
//----------------------------------------------------------------------------
void FrustumRays(const MaterialParameterContext& context, float4x4& dst) {
    float3 cameraRays[4];
    context.Scene->Camera()->Model().GetFrustumRays(cameraRays);

    dst.SetRow(size_t(CameraRay::LeftTop),     cameraRays[size_t(CameraRay::LeftTop)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::LeftBottom),  cameraRays[size_t(CameraRay::LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightBottom), cameraRays[size_t(CameraRay::RightBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightTop),    cameraRays[size_t(CameraRay::RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void FarCorners(const MaterialParameterContext& context, float4x4& dst) {
    const MemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    dst.SetRow(size_t(CameraRay::LeftTop),     frustumCorners[size_t(FrustumCorner::Far_LeftTop)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::LeftBottom),  frustumCorners[size_t(FrustumCorner::Far_LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightBottom), frustumCorners[size_t(FrustumCorner::Far_RightBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightTop),    frustumCorners[size_t(FrustumCorner::Far_RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void NearCorners(const MaterialParameterContext& context, float4x4& dst) {
    const MemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    dst.SetRow(size_t(CameraRay::LeftTop),     frustumCorners[size_t(FrustumCorner::Near_LeftTop)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::LeftBottom),  frustumCorners[size_t(FrustumCorner::Near_LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightBottom), frustumCorners[size_t(FrustumCorner::Near_RightBottom)].ZeroExtend());
    dst.SetRow(size_t(CameraRay::RightTop),    frustumCorners[size_t(FrustumCorner::Near_RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void NearFarZ(const MaterialParameterContext& context, float2& dst) {
    const ICamera *const camera = context.Scene->Camera();

    dst.x() = camera->ZNear();
    dst.y() = camera->ZFar();
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
