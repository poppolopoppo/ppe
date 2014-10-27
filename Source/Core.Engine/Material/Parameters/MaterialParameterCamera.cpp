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
    const MemoryView<const float3> corners = context.Scene->Camera()->Model().FrustumCorners();

    float4x4 rays(0.0f);
    rays.SetAxisX(corners[4] - corners[0]);
    rays.SetAxisX(corners[5] - corners[1]);
    rays.SetAxisX(corners[6] - corners[2]);
    rays.SetAxisX(corners[7] - corners[3]);

    const bool changed = (rays != *cached);
    *cached = rays;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterCameraMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniEyeDirection",          new MaterialParameterCamera_EyeDirection() );
    database->BindParameter("uniEyePosition",           new MaterialParameterCamera_EyePosition() );
    database->BindParameter("uniView",                  new MaterialParameterCamera_View() );
    database->BindParameter("uniInvertView",            new MaterialParameterCamera_InvertView() );
    database->BindParameter("uniProjection",            new MaterialParameterCamera_Projection() );
    database->BindParameter("uniInvertProjection",      new MaterialParameterCamera_InvertProjection() );
    database->BindParameter("uniViewProjection",        new MaterialParameterCamera_InvertView() );
    database->BindParameter("uniInvertViewProjection",  new MaterialParameterCamera_InvertViewProjection() );
    database->BindParameter("uniFrustumRays",           new MaterialParameterCamera_FrustumRays() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
