#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_RANDOM(_Macro) \
    _Macro(EMaterialVariability::FFrame, float,   UnitRand) \
    _Macro(EMaterialVariability::FFrame, float2,  UnitRand2) \
    _Macro(EMaterialVariability::FFrame, float3,  UnitRand3) \
    _Macro(EMaterialVariability::FFrame, float4,  UnitRand4) \
    _Macro(EMaterialVariability::FFrame, float4,  UnitRand4) \
    _Macro(EMaterialVariability::FFrame, float3,  HemisphereRand) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterRandom {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_RANDOM(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterRandom
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
