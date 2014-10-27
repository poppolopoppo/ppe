#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
FWD_REFPTR(MaterialDatabase);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterTime_WorldElapsedSeconds : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterTime_WorldElapsedSeconds() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterTime_WorldElapsedSeconds() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterTime_WorldTotalSeconds : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterTime_WorldTotalSeconds() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterTime_WorldTotalSeconds() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterTime_ProcessTotalSeconds : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterTime_ProcessTotalSeconds() : AbstractMaterialParameterMemoizer(MaterialVariability::Frame) {}
    virtual ~MaterialParameterTime_ProcessTotalSeconds() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTimeMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
