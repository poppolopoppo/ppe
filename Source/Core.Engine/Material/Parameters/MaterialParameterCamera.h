#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_CAMERA(_Macro) \
    _Macro(MaterialVariability::Scene, float3,   EyePosition) \
    _Macro(MaterialVariability::Scene, float3,   EyeDirection) \
    _Macro(MaterialVariability::Scene, float3,   EyeUp) \
    \
    _Macro(MaterialVariability::Scene, float4x4, View) \
    _Macro(MaterialVariability::Scene, float4x4, InvertView) \
    \
    _Macro(MaterialVariability::Scene, float4x4, Projection) \
    _Macro(MaterialVariability::Scene, float4x4, InvertProjection) \
    \
    _Macro(MaterialVariability::Scene, float4x4, ViewProjection) \
    _Macro(MaterialVariability::Scene, float4x4, InvertViewProjection) \
    \
    _Macro(MaterialVariability::Scene, float4x4, FrustumRays) \
    _Macro(MaterialVariability::Scene, float4x4, FarCorners) \
    _Macro(MaterialVariability::Scene, float4x4, NearCorners) \
    _Macro(MaterialVariability::Scene, float2,   NearFarZ)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterCamera {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_CAMERA(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterCamera
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
