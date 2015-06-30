#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_RANDOM(_Macro) \
    _Macro(MaterialVariability::Frame, float,   UnitRand) \
    _Macro(MaterialVariability::Frame, float2,  UnitRand2) \
    _Macro(MaterialVariability::Frame, float3,  UnitRand3) \
    _Macro(MaterialVariability::Frame, float4,  UnitRand4) \
    _Macro(MaterialVariability::Frame, float4,  UnitRand4) \
    _Macro(MaterialVariability::Frame, float3,  HemisphereRand) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterRandom {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_RANDOM(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterRandom
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
