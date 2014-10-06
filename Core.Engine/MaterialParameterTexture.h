#pragma once

#include "Engine.h"

#include "MaterialParameterMemoizer.h"

#include "Core.Graphics/BindName.h"

#include "Core/PoolAllocator.h"
#include "Core/ScalarMatrix.h"
#include "Core/ScalarVector.h"

namespace Core {
namespace Engine {
FWD_REFPTR(Material);
FWD_REFPTR(MaterialDatabase);
class Scene;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialParameterTexture_DuDv : public AbstractMaterialParameterMemoizer<float2> {
public:
    MaterialParameterTexture_DuDv(const Graphics::BindName& textureName);
    virtual ~MaterialParameterTexture_DuDv() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterTexture_DuDv);
protected:
    bool Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext& context) override;
private:
    const Graphics::BindName _textureName;
};
//----------------------------------------------------------------------------
class MaterialParameterTexture_DuDvDimensions : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterTexture_DuDvDimensions(const Graphics::BindName& textureName);
    virtual ~MaterialParameterTexture_DuDvDimensions() {}
    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterTexture_DuDvDimensions);
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
private:
    const Graphics::BindName _textureName;
};
//----------------------------------------------------------------------------
class MaterialParameterTexture_RenderTargetDuDvDimensions : public AbstractMaterialParameterMemoizer<float4> {
public:
    MaterialParameterTexture_RenderTargetDuDvDimensions() : AbstractMaterialParameterMemoizer(MaterialVariability::Material) {}
    virtual ~MaterialParameterTexture_RenderTargetDuDvDimensions() {}
protected:
    bool Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTextureMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
bool TryCreateTextureMaterialParameter(
    AbstractMaterialParameter **param,
    const Material *material,
    const Scene *scene,
    const Graphics::BindName& name );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
