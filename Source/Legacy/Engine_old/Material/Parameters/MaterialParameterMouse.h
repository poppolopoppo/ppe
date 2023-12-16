#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_MOUSE(_Macro) \
    _Macro(EMaterialVariability::FFrame, float4,  MousePosition) \
    _Macro(EMaterialVariability::FFrame, float4,  MouseButtons)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMouse {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MOUSE(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterMouse
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
