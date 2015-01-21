#include "stdafx.h"

#include "MaterialEffect.h"

#include "Effect.h"
#include "EffectConstantBuffer.h"
#include "EffectProgram.h"
#include "Material/Material.h"
#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Material/MaterialVariability.h"
#include "Material/Parameters/AbstractMaterialParameter.h"
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
static MaterialEffect::TextureSlot TextureSlot_(const Graphics::BindName& name, bool isCubeMap) {
    Assert(!name.empty());

    const char *cstr = name.cstr();
    Assert(cstr);

    static const char AnisotropicClamp[] = "uniAnisotropicClamp_";
    static const char AnisotropicWrap[] = "uniAnisotropicWrap_";
    static const char LinearClamp[] = "uniLinearClamp_";
    static const char LinearWrap[] = "uniLinearWrap_";
    static const char PointClamp[] = "uniPointClamp_";
    static const char PointWrap[] = "uniPointWrap_";
    static const char SRGB[] = "uniSRGB_";

    const char *textureName = cstr;
    const Graphics::SamplerState *samplerState = Graphics::SamplerState::LinearClamp;
    bool useSRGB = false;

    do {
        if (StartsWith(textureName, AnisotropicClamp)) {
            textureName = &textureName[lengthof(AnisotropicClamp) - 1];
            samplerState = Graphics::SamplerState::AnisotropicClamp;
        }
        else if (StartsWith(textureName, AnisotropicWrap)) {
            textureName = &textureName[lengthof(AnisotropicWrap) - 1];
            samplerState = Graphics::SamplerState::AnisotropicWrap;
        }
        else if (StartsWith(textureName, LinearClamp)) {
            textureName = &textureName[lengthof(LinearClamp) - 1];
            samplerState = Graphics::SamplerState::LinearClamp;
        }
        else if (StartsWith(textureName, LinearWrap)) {
            textureName = &textureName[lengthof(LinearWrap) - 1];
            samplerState = Graphics::SamplerState::LinearWrap;
        }
        else if (StartsWith(textureName, PointClamp)) {
            textureName = &textureName[lengthof(PointClamp) - 1];
            samplerState = Graphics::SamplerState::PointClamp;
        }
        else if (StartsWith(textureName, PointWrap)) {
            textureName = &textureName[lengthof(PointWrap) - 1];
            samplerState = Graphics::SamplerState::PointWrap;
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

    return MaterialEffect::TextureSlot(textureName, samplerState, useSRGB, isCubeMap);
}
//----------------------------------------------------------------------------
static void PrepareTexture_(Filename *filename,
                            MaterialEffect::TextureSlot& slot,
                            const Material *material,
                            const MaterialDatabase *materialDatabase,
                            TextureCache *textureCache,
                            RenderSurfaceManager *renderSurfaceManager ) {
    const ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename)& materialTextures = material->Textures();

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
static void FetchTexture_(MaterialEffect::TextureBinding& binding,
                            const MaterialEffect::TextureSlot& slot,
                            TextureCache *textureCache,
                            RenderSurfaceManager *renderSurfaceManager,
                            Graphics::IDeviceAPIEncapsulator *device ) {
    const Filename& filename = binding.Filename;

    if (slot.IsVirtualTexture) {
        AbstractRenderSurface *renderSurface = renderSurfaceManager->Unalias(filename.Dirpath());
        Assert(renderSurface->InUse()); // else the render target has never been filled !

        renderSurface->Prepare(device, binding.SurfaceLock);

        const Graphics::RenderTarget *renderTarget = nullptr;
        const Graphics::DepthStencil *depthStencil = nullptr;
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
static void DestroyTexture2D_(  MaterialEffect::TextureBinding& binding,
                                const MaterialEffect::TextureSlot& slot,
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
MaterialEffect::TextureSlot::TextureSlot(
    const Graphics::BindName& name,
    const Graphics::SamplerState *sampler,
    bool useSRGB,
    bool isCubeMap,
    bool isVirtuaTexture/* = false */)
:   Name(name)
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
SINGLETON_POOL_ALLOCATED_DEF(MaterialEffect, );
//----------------------------------------------------------------------------
MaterialEffect::MaterialEffect(const Engine::Effect *effect, const Engine::Material *material)
:   _effect(effect)
,   _material(material) {
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());
    Assert(material);
}
//----------------------------------------------------------------------------
MaterialEffect::~MaterialEffect() {}
//----------------------------------------------------------------------------
void MaterialEffect::Create(Graphics::IDeviceAPIEncapsulator *device, const Scene *scene) {
    Assert(_constants.empty());
    Assert(_textureSlots.empty());
    Assert(_textureBindings.empty());

    for (const Graphics::ShaderProgramType stage : Graphics::EachShaderProgramType()) {
        const EffectProgram *program = _effect->StageProgram(stage);
        if (!program)
            continue;

        const auto layouts = program->Constants();

        _constants.reserve(_constants.size() + program->Constants().size());
        for (const Pair<Graphics::BindName, Graphics::PCConstantBufferLayout>& layout : layouts) {
            PEffectConstantBuffer cbuffer;
            // Tries to merge constant buffers inside the same effect
            for (const PEffectConstantBuffer& c : _constants)
                if (c->Match(layout.first, *layout.second))
                    cbuffer = c;

            if (!cbuffer) {
                cbuffer = new EffectConstantBuffer(layout.first, layout.second);
                cbuffer->Create(device);
            }

            _constants.emplace_back(cbuffer);
        }

        _textureSlots.reserve(_textureSlots.size() + program->Textures().size());
        for (const Graphics::ShaderProgramTexture& texture : program->Textures())
            _textureSlots.emplace_back(TextureSlot_(texture.Name, texture.IsCubeMap));
    }

    _textureBindings.resize(_textureSlots.size());

    const MaterialDatabase *materialDatabase = scene->MaterialDatabase();
    Assert(materialDatabase);
    TextureCache *const textureCache = scene->RenderTree()->TextureCache();
    Assert(textureCache);
    RenderSurfaceManager *const renderSurfaceManager = scene->RenderTree()->RenderSurfaceManager();
    Assert(renderSurfaceManager);

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        PrepareTexture_(&_textureBindings[i].Filename, _textureSlots[i], _material, 
                        materialDatabase, textureCache, renderSurfaceManager );

    for (const PEffectConstantBuffer& cbuffer : _constants)
        cbuffer->Prepare(device, _material, scene);
}
//----------------------------------------------------------------------------
void MaterialEffect::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    for (PEffectConstantBuffer& cbuffer : _constants)
        if (1 == cbuffer->RefCount()) {
            cbuffer->Destroy(device);
            RemoveRef_AssertReachZero(cbuffer);
        }
        else {
            Assert(1 < cbuffer->RefCount());
            cbuffer.reset(nullptr);
        }

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        DestroyTexture2D_(_textureBindings[i], _textureSlots[i], device);

    _constants.clear();
    _textureSlots.clear();
    _textureBindings.clear();
}
//----------------------------------------------------------------------------
void MaterialEffect::Prepare(Graphics::IDeviceAPIEncapsulator *device, const Scene *scene, const VariabilitySeed *seeds) {
    TextureCache *const textureCache = scene->RenderTree()->TextureCache();
    Assert(textureCache);
    RenderSurfaceManager *const renderSurfaceManager = scene->RenderTree()->RenderSurfaceManager();
    Assert(renderSurfaceManager);

    const size_t textureCount = _textureSlots.size();
    for (size_t i = 0; i < textureCount; ++i)
        FetchTexture_(_textureBindings[i], _textureSlots[i], textureCache, renderSurfaceManager, device);

    const MaterialContext context = { scene, this, MemoryView<const VariabilitySeed>(seeds, VariabilitySeed::Count) };
    for (const PEffectConstantBuffer& cbuffer : _constants)
        cbuffer->Eval(device, context);
}
//----------------------------------------------------------------------------
void MaterialEffect::Set(Graphics::IDeviceAPIContextEncapsulator *deviceContext) {
    size_t constantOffset = 0;
    size_t textureOffset = 0;

    for (const Graphics::ShaderProgramType stage : Graphics::EachShaderProgramType()) {
        const EffectProgram *program = _effect->StageProgram(stage);
        if (!program)
            continue;

        const size_t constantCount = program->Constants().size();
        for (size_t i = 0; i < constantCount; ++i)
            deviceContext->SetConstantBuffer(stage, i, _constants[constantOffset + i]);

        const size_t textureCount = program->Textures().size();
        for (size_t i = 0; i < textureCount; ++i) {
            const TextureSlot& slot = _textureSlots[textureOffset + i];
            const TextureBinding& binding = _textureBindings[textureOffset + i];

            // achtung, texture can be null as it's a weak reference pointing to the texture cache
            // so cleaning the texture cache will reset that value !
            const Graphics::WCTexture& texture = binding.Texture;

            if (binding.SurfaceLock) {
                Assert(texture);
                binding.SurfaceLock->Bind(stage, i); // tracks usage of RTs as texture to unbind them before using them as RT
            }

            deviceContext->SetTexture(stage, i, texture);
            deviceContext->SetSamplerState(stage, i, slot.Sampler);
        }

        constantOffset += constantCount;
        textureOffset += textureCount;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
