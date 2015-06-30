#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_TIME(_Macro) \
    _Macro(MaterialVariability::World, float,   WorldElapsedSeconds) \
    _Macro(MaterialVariability::World, float,   WorldTotalSeconds) \
    \
    _Macro(MaterialVariability::Frame, float,   ProcessTotalSeconds)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTime {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TIME(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterTime
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
