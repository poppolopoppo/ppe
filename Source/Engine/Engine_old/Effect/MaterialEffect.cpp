// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MaterialEffect.h"

#include "Effect.h"
#include "EffectConstantBuffer.h"
#include "EffectProgram.h"
#include "SharedConstantBuffer.h"

#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Material/MaterialVariability.h"
#include "Material/IMaterialParameter.h"
#include "Render/RenderSurfaceManager.h"
#include "Render/Surfaces/AbstractRenderSurface.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Shader/ConstantBuffer.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ShaderProgram.h"
#include "Core.Graphics/Device/State/SamplerState.h"
#include "Core.Graphics/Device/Texture/DepthStencil.h"
#include "Core.Graphics/Device/Texture/RenderTarget.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"
#include "Core.Graphics/Device/Texture/TextureCube.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/Format.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FMaterialEffect::FTextureSlot TextureSlot_(const Graphics::FBindName& name, bool isCubeMap) {
    Assert(!name.empty());

    const char *cstr = name.c_str();
    Assert(cstr);

    static const char AnisotropicClamp[] = "uniAnisotropicClamp_";
    static const char AnisotropicWrap[] = "uniAnisotropicWrap_";
    static const char LinearClamp[] = "uniLinearClamp_";
    static const char LinearWrap[] = "uniLinearWrap_";
    static const char PointClamp[] = "uniPointClamp_";
    static const char PointWrap[] = "uniPointWrap_";
    static const char SRGB[] = "uniSRGB_";

    const char *textureName = cstr;
    const Graphics::FSamplerState *samplerState = Graphics::FSamplerState::LinearClamp;
    bool useSRGB = false;

    do {
        if (StartsWith(textureName, AnisotropicClamp)) {
            textureName = &textureName[lengthof(AnisotropicClamp) - 1];
            samplerState = Graphics::FSamplerState::AnisotropicClamp;
        }
        else if (StartsWith(textureName, AnisotropicWrap)) {
            textureName = &textureName[lengthof(AnisotropicWrap) - 1];
            samplerState = Graphics::FSamplerState::AnisotropicWrap;
        }
        else if (StartsWith(textureName, LinearClamp)) {
            textureName = &textureName[lengthof(LinearClamp) - 1];
            samplerState = Graphics::FSamplerState::LinearClamp;
        }
        else if (StartsWith(textureName, LinearWrap)) {
            textureName = &textureName[lengthof(LinearWrap) - 1];
            samplerState = Graphics::FSamplerState::LinearWrap;
        }
        else if (StartsWith(textureName, PointClamp)) {
            textureName = &textureName[lengthof(PointClamp) - 1];
            samplerState = Graphics::FSamplerState::PointClamp;
        }
        else if (StartsWith(textureName, PointWrap)) {
            textureName = &textureName[lengthof(PointWrap) - 1];
            samplerState = Graphics::FSamplerState::PointWrap;
        }
        else if (StartsWith(textureName, SRGB)) {
            textureName = &textureName[lengthof(SRGB) - 1];
            useSRGB = true;
        }
        else {
            break;
        }
    }
    while (true);

    return FMaterialEffect::FTextureSlot(textureName, samplerState, useSRGB, isCubeMap);
}
//----------------------------------------------------------------------------
static void PrepareTexture_(FFilename *filename,
                            FMaterialEffect::FTextureSlot& slot,
                            const FMaterial *material,
                            const FMaterialDatabase *materialDatabase,
                            FTextureCache *textureCache,
                            FRenderSurfaceManager *renderSurfaceManager ) {
    const ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename)& materialTextures = material->Textures();

    const auto it = materialTextures.Find(slot.Name);
    if (materialTextures.end() == it) {
        if (!materialDatabase->TryGetTexture(slot.Name, filename))
            AssertNotReached();
    }
    else {
        *filename = it->second;
    }
    Assert(filename);
    Assert(!filename->empty());

    PAbstractRenderSurface renderSurface;
    if (renderSurfaceManager->TryUnalias(filename->Dirpath(), &renderSurface)) {
        Assert(renderSurface);
        slot.IsVirtualTexture = true;
        Assert( filename->Basename() == renderSurfaceManager->RenderTargetName() ||
                filename->Basename() == renderSurfaceManager->DepthStencilName() );
    }
    else {
        slot.IsVirtualTexture = false;
        textureCache->PrepareTexture(*filename, slot.UseSRGB);
    }
}
//----------------------------------------------------------------------------
static void FetchTexture_(  FMaterialEffect::FTextureBinding& binding,
                            const FMaterialEffect::FTextureSlot& slot,
                            FTextureCache *textureCache,
                            FRenderSurfaceManager *renderSurfaceManager,
                            Graphics::IDeviceAPIEncapsulator *device ) {
    const FFilename& filename = binding.Filename;

    if (slot.IsVirtualTexture) {
        FAbstractRenderSurface *renderSurface = renderSurfaceManager->Unalias(filename.Dirpath());
        Assert(renderSurface->InUse()); // else the render target has never been filled !

        renderSurface->Prepare(device, binding.SurfaceLock);

        const Graphics::FRenderTarget *renderTarget = nullptr;
        const Graphics::FDepthStencil *depthStencil = nullptr;
        binding.SurfaceLock->Acquire(&renderTarget, &depthStencil);

        if(filename.Basename() == renderSurfaceManager->RenderTargetName()) {
            Assert(renderTarget);
            binding.Texture = renderTarget;
        }
        else {
            Assert(filename.Basename() == renderSurfaceManager->DepthStencilName() );
            Assert(depthStencil);
            binding.Texture = depthStencil;
        }
    }
    else {
        if (slot.IsCubeMap)
            binding.Texture = textureCache->FetchTextureCube_Fallback(filename);
        else
            binding.Texture = textureCache->FetchTexture2D_Fallback(filename);
    }
    Assert(binding.Texture);
}
//----------------------------------------------------------------------------
static void DestroyTexture2D_(  FMaterialEffect::FTextureBinding& binding,
                                const FMaterialEffect::FTextureSlot& slot,
                                Graphics::IDeviceAPIEncapsulator *device ) {
    Assert(binding.Texture);
    binding.Texture.reset(nullptr);

    if (slot.IsVirtualTexture) {
        Assert(binding.SurfaceLock);
        binding.SurfaceLock->Release(device, binding.SurfaceLock);
        Assert(!binding.SurfaceLock);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMaterialEffect::FTextureSlot::FTextureSlot(
    const Graphics::FBindName& name,
    const Graphics::FSamplerState *sampler,
    bool useSRGB,
    bool isCubeMap,
    bool isVirtuaTexture/* = false */)
:   FName(name)
,   Sampler(sampler) 
,   UseSRGB(useSRGB) 
,   IsCubeMap(isCubeMap)
,   IsVirtualTexture(isVirtuaTexture) {
    Assert(!name.empty());
    Assert(sampler);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FMaterialEffect, );
//----------------------------------------------------------------------------
FMaterialEffect::FMaterialEffect(const Engine::FEffect *effect, const Engine::FMaterial *material)
:   _effect(effect)
,   _material(material) {
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());
    Assert(material);
}
//----------------------------------------------------------------------------
FMaterialEffect::~FMaterialEffect() {}
//----------------------------------------------------------------------------
void FMaterialEffect::BindParameter(const Graphics::FBindName& name, IMaterialParameter *parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters.Insert_AssertUnique(name, parameter);
}
//----------------------------------------------------------------------------
void FMaterialEffect::Create(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FScene *scene) {
    Assert(_constants.empty());
    Assert(_textureSlots.empty());
    Assert(_textureBindings.empty());

    // *** TEXTURES ***

    for (const Graphics::EShaderProgramType stage : Graphics::EachShaderProgramType()) {
        const FEffectProgram *program = _effect->StageProgram(stage);
        if (!program)
            continue;

        _textureSlots.reserve(_textureSlots.size() + program->Textures().size());
        for (const Graphics::FShaderProgramTexture& texture : program->Textures())
            _textureSlots.emplace_back(TextureSlot_(texture.Name, texture.IsCubeMap));
    }

    _textureBindings.resize(_textureSlots.size());

    FTextureCache *const textureCache = scene->RenderTree()->TextureCache();
    Assert(textureCache);
    FRenderSurfaceManager *const renderSurfaceManager = scene->RenderTree()->RenderSurfaceManager();
    Assert(renderSurfaceManager);

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        PrepareTexture_(&_textureBindings[i].Filename, _textureSlots[i], _material, 
                        materialDatabase, textureCache, renderSurfaceManager );

    // *** CONSTANT BUFFERS ***

    const VECTOR(FEffect, PSharedConstantBuffer)& sharedBuffers = _effect->SharedBuffers();
    const FMaterialParameterMutableContext paramContext{scene, this, materialDatabase};

    _constants.resize(sharedBuffers.size());
    forrange(i, 0, sharedBuffers.size()) {
        PEffectConstantBuffer cbuffer = new FEffectConstantBuffer(sharedBuffers[i].get());
        cbuffer->Prepare(paramContext);
        _constants[i] = std::move(cbuffer);
    }
}
//----------------------------------------------------------------------------
void FMaterialEffect::Destroy(Graphics::IDeviceAPIEncapsulator *device) {

    // *** CONSTANT BUFFERS ***

    for (PEffectConstantBuffer& cbuffer : _constants) {
        cbuffer->Clear();
        RemoveRef_AssertReachZero(cbuffer);
    }

    // *** TEXTURES ***

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        DestroyTexture2D_(_textureBindings[i], _textureSlots[i], device);

    _constants.clear();
    _textureSlots.clear();
    _textureBindings.clear();
}
//----------------------------------------------------------------------------
void FMaterialEffect::Prepare(Graphics::IDeviceAPIEncapsulator *device, const FScene *scene, const VariabilitySeeds& seeds) {
    FTextureCache *const textureCache = scene->RenderTree()->TextureCache();
    Assert(textureCache);
    FRenderSurfaceManager *const renderSurfaceManager = scene->RenderTree()->RenderSurfaceManager();
    Assert(renderSurfaceManager);

    // *** TEXTURES ***

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        FetchTexture_(_textureBindings[i], _textureSlots[i], textureCache, renderSurfaceManager, device);

    // *** CONSTANT BUFFERS ***

    const FMaterialParameterContext context = { scene };
    for (const PEffectConstantBuffer& cbuffer : _constants)
        cbuffer->Eval(context);
}
//----------------------------------------------------------------------------
void FMaterialEffect::Set(Graphics::IDeviceAPIContext *deviceContext) {

    // *** CONSTANT BUFFERS ***

    for (const PEffectConstantBuffer& cbuffer : _constants)
        cbuffer->SetDataIFN(deviceContext->Encapsulator()->Device());

    // *** TEXTURES ***

    const Graphics::FTexture *stageTextures[16];
    const Graphics::FSamplerState *stageSamplers[16];

    size_t textureOffset = 0;
    for (const Graphics::EShaderProgramType stage : Graphics::EachShaderProgramType()) {
        const FEffectProgram *program = _effect->StageProgram(stage);
        if (!program)
            continue;

        const size_t textureCount = program->Textures().size();
        for (size_t i = 0; i < textureCount; ++i) {
            const FTextureSlot& slot = _textureSlots[textureOffset + i];
            const FTextureBinding& binding = _textureBindings[textureOffset + i];

            // achtung, texture can be null as it's a weak reference pointing to the texture cache
            // so cleaning the texture cache will reset that value !
            const Graphics::WCTexture& texture = binding.Texture;

            if (binding.SurfaceLock) {
                Assert(texture);
                binding.SurfaceLock->Bind(stage, i); // tracks usage of RTs as texture to unbind them before using them as RT
            }

            stageTextures[i] = texture;
            stageSamplers[i] = slot.Sampler;
        }

        if (textureCount) {
            deviceContext->SetTextures(stage, MakeConstView(&stageTextures[0], &stageTextures[textureCount]));
            deviceContext->SetSamplerStates(stage, MakeConstView(&stageSamplers[0], &stageSamplers[textureCount]));
        }

        textureOffset += textureCount;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
