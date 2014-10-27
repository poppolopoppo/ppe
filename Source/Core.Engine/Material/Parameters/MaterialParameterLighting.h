#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
FWD_REFPTR(MaterialDatabase);
FWD_REFPTR(LightingEnvironment);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterLighting_SunColor : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterLighting_SunColor() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterLighting_SunColor() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterLighting_SunDirection : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterLighting_SunDirection() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterLighting_SunDirection() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterLighting_Exposure : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterLighting_Exposure() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterLighting_Exposure() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
class MaterialParameterLighting_WhitePoint : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterLighting_WhitePoint() : AbstractMaterialParameterMemoizer(MaterialVariability::World) {}
    virtual ~MaterialParameterLighting_WhitePoint() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterLightingMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
