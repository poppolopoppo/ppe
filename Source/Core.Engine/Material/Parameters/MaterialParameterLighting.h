#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class MaterialDatabase;
FWD_REFPTR(LightingEnvironment);

#define EACH_MATERIALPARAMETER_LIGHTING(_Macro) \
    _Macro(MaterialVariability::World, float3,  SunColor) \
    _Macro(MaterialVariability::World, float3,  SunDirection) \
    _Macro(MaterialVariability::World, float,   SunIntensity) \
    \
    _Macro(MaterialVariability::World, float,   Exposure) \
    _Macro(MaterialVariability::World, float,   WhitePoint)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterLighting {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_LIGHTING(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterLighting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
