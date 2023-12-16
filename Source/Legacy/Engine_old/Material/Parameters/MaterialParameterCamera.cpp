// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MaterialParameterCamera.h"

#include "Camera/Camera.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_CAMERA(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
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
// FCamera FModel
//----------------------------------------------------------------------------
void EyePosition(const FMaterialParameterContext& context, float3& dst) {
    dst = context.Scene->Camera()->Model().Parameters().Position;
}
//----------------------------------------------------------------------------
void EyeDirection(const FMaterialParameterContext& context, float3& dst) {
    dst = context.Scene->Camera()->Model().Parameters().LookAtDir;
}
//----------------------------------------------------------------------------
void EyeUp(const FMaterialParameterContext& context, float3& dst) {
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
void View(const FMaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().View();
}
//----------------------------------------------------------------------------
void InvertView(const FMaterialParameterContext& context, float4x4& dst) {
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
void Projection(const FMaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().Projection();
}
//----------------------------------------------------------------------------
void InvertProjection(const FMaterialParameterContext& context, float4x4& dst) {
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
void ViewProjection(const FMaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().ViewProjection();
}
//----------------------------------------------------------------------------
void InvertViewProjection(const FMaterialParameterContext& context, float4x4& dst) {
    dst = context.Scene->Camera()->Model().InvertViewProjection();
}
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
// FFrustum
//----------------------------------------------------------------------------
void FrustumRays(const FMaterialParameterContext& context, float4x4& dst) {
    float3 cameraRays[4];
    context.Scene->Camera()->Model().GetFrustumRays(cameraRays);

    dst.SetRow(size_t(ECameraRay::LeftTop),     cameraRays[size_t(ECameraRay::LeftTop)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::LeftBottom),  cameraRays[size_t(ECameraRay::LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightBottom), cameraRays[size_t(ECameraRay::RightBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightTop),    cameraRays[size_t(ECameraRay::RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void FarCorners(const FMaterialParameterContext& context, float4x4& dst) {
    const TMemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    dst.SetRow(size_t(ECameraRay::LeftTop),     frustumCorners[size_t(EFrustumCorner::Far_LeftTop)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::LeftBottom),  frustumCorners[size_t(EFrustumCorner::Far_LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightBottom), frustumCorners[size_t(EFrustumCorner::Far_RightBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightTop),    frustumCorners[size_t(EFrustumCorner::Far_RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void NearCorners(const FMaterialParameterContext& context, float4x4& dst) {
    const TMemoryView<const float3> frustumCorners = context.Scene->Camera()->Model().FrustumCorners();

    dst.SetRow(size_t(ECameraRay::LeftTop),     frustumCorners[size_t(EFrustumCorner::Near_LeftTop)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::LeftBottom),  frustumCorners[size_t(EFrustumCorner::Near_LeftBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightBottom), frustumCorners[size_t(EFrustumCorner::Near_RightBottom)].ZeroExtend());
    dst.SetRow(size_t(ECameraRay::RightTop),    frustumCorners[size_t(EFrustumCorner::Near_RightTop)].ZeroExtend());
}
//----------------------------------------------------------------------------
void NearFarZ(const FMaterialParameterContext& context, float2& dst) {
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
