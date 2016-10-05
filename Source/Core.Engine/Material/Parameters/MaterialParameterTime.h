#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_TIME(_Macro) \
    _Macro(EMaterialVariability::FWorld, float,   WorldElapsedSeconds) \
    _Macro(EMaterialVariability::FWorld, float,   WorldTotalSeconds) \
    \
    _Macro(EMaterialVariability::FFrame, float,   ProcessTotalSeconds)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTime {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TIME(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterTime
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
