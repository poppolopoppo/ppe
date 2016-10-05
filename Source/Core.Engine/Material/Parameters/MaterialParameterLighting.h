#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;
FWD_REFPTR(LightingEnvironment);

#define EACH_MATERIALPARAMETER_LIGHTING(_Macro) \
    _Macro(EMaterialVariability::FWorld, float3,  SunColor) \
    _Macro(EMaterialVariability::FWorld, float3,  SunDirection) \
    _Macro(EMaterialVariability::FWorld, float,   SunIntensity) \
    \
    _Macro(EMaterialVariability::FWorld, float,   Exposure) \
    _Macro(EMaterialVariability::FWorld, float,   WhitePoint)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterLighting {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_LIGHTING(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterLighting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
