#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_CAMERA(_Macro) \
    _Macro(EMaterialVariability::FScene, float3,   EyePosition) \
    _Macro(EMaterialVariability::FScene, float3,   EyeDirection) \
    _Macro(EMaterialVariability::FScene, float3,   EyeUp) \
    \
    _Macro(EMaterialVariability::FScene, float4x4, View) \
    _Macro(EMaterialVariability::FScene, float4x4, InvertView) \
    \
    _Macro(EMaterialVariability::FScene, float4x4, Projection) \
    _Macro(EMaterialVariability::FScene, float4x4, InvertProjection) \
    \
    _Macro(EMaterialVariability::FScene, float4x4, ViewProjection) \
    _Macro(EMaterialVariability::FScene, float4x4, InvertViewProjection) \
    \
    _Macro(EMaterialVariability::FScene, float4x4, FrustumRays) \
    _Macro(EMaterialVariability::FScene, float4x4, FarCorners) \
    _Macro(EMaterialVariability::FScene, float4x4, NearCorners) \
    _Macro(EMaterialVariability::FScene, float2,   NearFarZ)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_CAMERA(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
