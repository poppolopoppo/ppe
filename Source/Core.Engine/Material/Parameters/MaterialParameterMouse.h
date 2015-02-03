#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
FWD_REFPTR(MaterialDatabase);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterMouse_Position : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterMouse_Position() : AbstractMaterialParameterMemoizer(MaterialVariability::Frame) {}
    virtual ~MaterialParameterMouse_Position() {}
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterMouse_Buttons : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterMouse_Buttons() : AbstractMaterialParameterMemoizer(MaterialVariability::Frame) {}
    virtual ~MaterialParameterMouse_Buttons() {}
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterMouseMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
