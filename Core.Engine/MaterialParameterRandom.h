#pragma once

#include "Engine.h"

#include "MaterialParameterMemoizer.h"

#include "Core/RandomGenerator.h"

namespace Core {
namespace Engine {
FWD_REFPTR(MaterialDatabase);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterRandom_Unit : public AbstractMaterialParameterMemoizer<float> {
public:
    MaterialParameterRandom_Unit()
    :   AbstractMaterialParameterMemoizer(MaterialVariability::Frame)
    ,   _rand(RandomGenerator::RandomSeedTag()) {}
    virtual ~MaterialParameterRandom_Unit() {}
protected:
    bool Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) override;
private:
    RandomGenerator _rand;
};
//----------------------------------------------------------------------------
class MaterialParameterRandom_Unit2 : public AbstractMaterialParameterMemoizer<float2> {
public:
    MaterialParameterRandom_Unit2()
    :   AbstractMaterialParameterMemoizer(MaterialVariability::Frame)
    ,   _rand(RandomGenerator::RandomSeedTag()) {}
    virtual ~MaterialParameterRandom_Unit2() {}
protected:
    bool Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext& context) override;
private:
    RandomGenerator _rand;
};
//----------------------------------------------------------------------------
class MaterialParameterRandom_Unit3 : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterRandom_Unit3()
    :   AbstractMaterialParameterMemoizer(MaterialVariability::Frame)
    ,   _rand(RandomGenerator::RandomSeedTag()) {}
    virtual ~MaterialParameterRandom_Unit3() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
private:
    RandomGenerator _rand;
};
//----------------------------------------------------------------------------
class MaterialParameterRandom_Unit4 : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterRandom_Unit4()
    :   AbstractMaterialParameterMemoizer(MaterialVariability::Frame)
    ,   _rand(RandomGenerator::RandomSeedTag()) {}
    virtual ~MaterialParameterRandom_Unit4() {}
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
private:
    RandomGenerator _rand;
};
//----------------------------------------------------------------------------
class MaterialParameterRandom_Hemisphere : public AbstractMaterialParameterMemoizer<float3> {
public:
    MaterialParameterRandom_Hemisphere()
    :   AbstractMaterialParameterMemoizer(MaterialVariability::Frame)
    ,   _rand(RandomGenerator::RandomSeedTag()) {}
    virtual ~MaterialParameterRandom_Hemisphere() {}
protected:
    bool Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) override;
private:
    RandomGenerator _rand;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterRandomMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
