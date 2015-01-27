#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Vector.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderLayer);
class EffectCompiler;
FWD_REFPTR(MaterialDatabase);
class RenderSurfaceManager;
FWD_REFPTR(Scene);
class TextureCache;
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderTree : public Meta::ThreadResource {
public:
    RenderTree( const Engine::Scene *scene, IServiceProvider *serviceProvider);
    ~RenderTree();

    const Engine::Scene *Scene() const { return _scene; }
    Engine::RenderSurfaceManager *RenderSurfaceManager() const;
    Engine::EffectCompiler *EffectCompiler() const;
    Engine::TextureCache *TextureCache() const;
    const VECTOR(Render, PAbstractRenderLayer)& Layers() const { return _layers; }

    void Add(AbstractRenderLayer *layer);
    void Remove(AbstractRenderLayer *layer);
    bool TryGet(const char *name, AbstractRenderLayer **layer) const;
    bool TryGet(const char *name, PAbstractRenderLayer& player) const;

    // (1) create the resources and sort if necessary
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase, VariabilitySeed *seeds);
    // (2) change device context and execute draw calls
    void Render(Graphics::IDeviceAPIContextEncapsulator *context);
    // (3) called before death or disabling to destroy possibly created resources
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

private:
    const Engine::Scene *_scene;
    Engine::SRenderSurfaceService _renderSurfaceService;
    Engine::SEffectCompilerService _effectCompilerService;
    Engine::STextureCacheService _textureCacheService;
    VECTOR(Render, PAbstractRenderLayer) _layers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
