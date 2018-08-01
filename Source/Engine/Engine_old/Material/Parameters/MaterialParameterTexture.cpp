#include "stdafx.h"

#include "MaterialParameterTexture.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool RetrieveTexture2DFromMaterialIFP_(
    Graphics::SCTexture2D *pTexture2D,
    const FMaterialParameterContext& context,
    const Graphics::FBindName& textureName ) {
    Assert(pTexture2D);

    const Graphics::FTexture *texture = nullptr;
    const size_t textureCount = context.MaterialEffect->TextureSlots().size();

    for (size_t i = 0; i < textureCount; ++i)
        if (context.MaterialEffect->TextureSlots()[i].Name == textureName) {
            texture = context.MaterialEffect->TextureBindings()[i].Texture;
            pTexture2D->reset(checked_cast<const Graphics::FTexture2D *>(texture));
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
static IMaterialParameter *CreateTextureParam_(const Graphics::FBindName& textureName, bool hasDimensions) {
    if (hasDimensions)
        return new MaterialParameterTexture::Memoizer_DuDvDimensions(textureName);
    else
        return new MaterialParameterTexture::Memoizer_DuDv(textureName);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_TEXTURE(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_TEXTURE(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
void RenderTargetDuDvDimensions(const FMaterialParameterContext& context, float4& dst) {
    AssertNotImplemented(); // TODO: access current render layer from scene
    dst = float4(0);
}
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
// FDuDv
//----------------------------------------------------------------------------
FDuDv::FDuDv(const Graphics::FBindName& textureName)
:   TextureName(textureName) {
    Assert(!textureName.empty());
}
//----------------------------------------------------------------------------
void FDuDv::TypedEval(const FMaterialParameterContext& context, float2& dst) {
    Graphics::SCTexture2D texture;
    if (RetrieveTexture2DFromMaterialIFP_(&texture, context, TextureName))
        dst = texture->DuDvDimensions().xy();
    else
        dst.Broadcast(0.0f);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<FDuDv>;
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
// FDuDvDimensions
//----------------------------------------------------------------------------
FDuDvDimensions::FDuDvDimensions(const Graphics::FBindName& textureName)
:   TextureName(textureName) {
    Assert(!textureName.empty());
}
//----------------------------------------------------------------------------
void FDuDvDimensions::TypedEval(const FMaterialParameterContext& context, float4& dst) {
    Graphics::SCTexture2D texture;
    if (RetrieveTexture2DFromMaterialIFP_(&texture, context, TextureName))
        dst = texture->DuDvDimensions();
    else
        dst.Broadcast(0.0f);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<FDuDvDimensions>;
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterTexture {
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field ) {
    Assert(param);
    Assert(context.MaterialEffect);
    Assert(context.Database);
    Assert(context.Scene);
    Assert(!name.empty());

    const char *cstr = name.c_str();

    static const char uniDuDvDimensions[] = "uniDuDvDimensions_";
    static const char uniDuDv[] = "uniDuDv_";

    bool hasDimensions = false;
    Graphics::FBindName textureName;

    if (StartsWith(cstr, uniDuDvDimensions)) {
        hasDimensions = true;
        textureName = &cstr[lengthof(uniDuDvDimensions) - 1];
        Assert(Graphics::EConstantFieldType::Float4 == field.Type());
    }
    else if (StartsWith(cstr, uniDuDv)) {
        Assert(!hasDimensions);
        textureName = &cstr[lengthof(uniDuDv) - 1];
        Assert(Graphics::EConstantFieldType::Float2 == field.Type());
    }
    else {
        Assert(!*param);
        return false;
    }

    Assert(!textureName.empty());

    FFilename filename;
    // Local texture path search :
    if (context.MaterialEffect->Material()->Textures().TryGet(textureName, &filename)) {
        Assert(!filename.empty());
        *param = CreateTextureParam_(textureName, hasDimensions);
        context.MaterialEffect->BindParameter(name, *param); // TODO : bind in local context.MaterialParameterDatabase
    }
    // Global texture path search :
    else if (context.Database->TryGetTexture(textureName, &filename)) {
        Assert(!filename.empty());
        *param = CreateTextureParam_(textureName, hasDimensions);
        context.Database->BindParameter(name, *param); // TODO : bind in local context.MaterialParameterDatabase
    }
    else {
        return false;
    }

    Assert(*param);
    return true;
}
//----------------------------------------------------------------------------
} //!MaterialParameterTexture
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
