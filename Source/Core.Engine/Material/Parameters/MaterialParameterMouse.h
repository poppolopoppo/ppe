#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_MOUSE(_Macro) \
    _Macro(MaterialVariability::Frame, float4,  MousePosition) \
    _Macro(MaterialVariability::Frame, float4,  MouseButtons)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMouse {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MOUSE(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterMouse
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
