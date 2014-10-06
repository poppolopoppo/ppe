#include "stdafx.h"

#include "MaterialParameterTexture.h"

#include "Material.h"
#include "MaterialContext.h"
#include "MaterialDatabase.h"
#include "MaterialEffect.h"
#include "Scene.h"
#include "TextureCache.h"

#include "Core.Graphics/Texture2D.h"

#include "Core/AssociativeVector.h"
#include "Core/Filename.h"
#include "Core/PoolAllocator-impl.h"
#include "Core/String.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool RetrieveTexture2DFromMaterialIFP_(
    const Graphics::Texture2D **pTexture2D,
    const MaterialContext& context,
    const Graphics::BindName& textureName ) {
    const Graphics::Texture *texture = nullptr;
    const size_t textureCount = context.MaterialEffect->TextureSlots().size();
    for (size_t i = 0; i < textureCount; ++i)
        if (context.MaterialEffect->TextureSlots()[i].Name == textureName) {
            texture = context.MaterialEffect->TextureBindings()[i].Texture;
            *pTexture2D = checked_cast<const Graphics::Texture2D *>(texture);
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterTexture_DuDv, );
//----------------------------------------------------------------------------
MaterialParameterTexture_DuDv::MaterialParameterTexture_DuDv(const Graphics::BindName& textureName)
:   AbstractMaterialParameterMemoizer(MaterialVariability::Material)
,   _textureName(textureName) {
    Assert(!textureName.empty());
}
//----------------------------------------------------------------------------
bool MaterialParameterTexture_DuDv::Memoize_ReturnIfChanged_(float2 *cached, const MaterialContext& context) {
    float2 dudv = 0;
    const Graphics::Texture2D *texture = nullptr;
    if (RetrieveTexture2DFromMaterialIFP_(&texture, context, _textureName))
        dudv = texture->DuDvDimensions().xy();

    const bool changed = (dudv != *cached);
    *cached = dudv;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterTexture_DuDvDimensions, );
//----------------------------------------------------------------------------
MaterialParameterTexture_DuDvDimensions::MaterialParameterTexture_DuDvDimensions(const Graphics::BindName& textureName)
:   AbstractMaterialParameterMemoizer(MaterialVariability::Material)
,   _textureName(textureName) {
    Assert(!textureName.empty());
}
//----------------------------------------------------------------------------
bool MaterialParameterTexture_DuDvDimensions::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) {
    float4 dudvdimensions = 0;
    const Graphics::Texture2D *texture = nullptr;
    if (RetrieveTexture2DFromMaterialIFP_(&texture, context, _textureName))
        dudvdimensions = texture->DuDvDimensions();

    const bool changed = (dudvdimensions != *cached);
    *cached = dudvdimensions;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterTexture_RenderTargetDuDvDimensions::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) {
    AssertNotImplemented(); // TODO : expose render tree in material context

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterTextureMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniRenderTargetDuDvDimensions", new MaterialParameterTexture_RenderTargetDuDvDimensions() );
}
//----------------------------------------------------------------------------
bool TryCreateTextureMaterialParameter(
    AbstractMaterialParameter **param,
    const Material *material,
    const Scene *scene,
    const Graphics::BindName& name ) {
    Assert(param);
    Assert(material);
    Assert(scene);
    Assert(!name.empty());

    const char *cstr = name.cstr();

    const char uniDuDvDimensions[] = "uniDuDvDimensions_";
    const char uniDuDv[] = "uniDuDv_";

    bool hasDimensions = false;
    Graphics::BindName textureName;

    if (StartsWith(cstr, uniDuDvDimensions)) {
        hasDimensions = true;
        textureName = &cstr[lengthof(uniDuDvDimensions) - 1];
    }
    else if (StartsWith(cstr, uniDuDv)) {
        Assert(!hasDimensions);
        textureName = &cstr[lengthof(uniDuDv) - 1];
    }
    else {
        Assert(!*param);
        return false;
    }

    Assert(!textureName.empty());

    Filename filename;
    // Local texture path search :
    if (!material->Textures().TryGet(textureName, &filename)) {
        /*
        // Global texture path search :
        if (!scene->MaterialDatabase()->TryGetTexture(textureName, &filename)) {
            AssertNotReached(); // failed to retrieve the texture path, not binded ?
            return false;
        }
        */
        AssertNotReached(); // 07/09/14: texture properties are only readable if the texture is binded
    }
    Assert(!filename.empty());

    if (hasDimensions)
        *param = new MaterialParameterTexture_DuDvDimensions(textureName);
    else
        *param = new MaterialParameterTexture_DuDv(textureName);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
