#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Vector.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderLayer);
class FEffectCompiler;
class FMaterialDatabase;
class FRenderSurfaceManager;
FWD_REFPTR(Scene);
class FTextureCache;
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FRenderTree : public Meta::FThreadResource {
public:
    FRenderTree( const Engine::FScene *scene, IServiceProvider *serviceProvider);
    ~FRenderTree();

    const Engine::FScene *FScene() const { return _scene; }
    Engine::FRenderSurfaceManager *FRenderSurfaceManager() const;
    Engine::FEffectCompiler *FEffectCompiler() const;
    Engine::FTextureCache *FTextureCache() const;
    const VECTOR(Render, PAbstractRenderLayer)& Layers() const { return _layers; }

    void Add(FAbstractRenderLayer *layer);
    void Remove(FAbstractRenderLayer *layer);
    bool TryGet(const char *name, FAbstractRenderLayer **layer) const;
    bool TryGet(const char *name, PAbstractRenderLayer& player) const;

    // (1) create the resources and sort if necessary
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, FVariabilitySeed *seeds);
    // (2) change device context and execute draw calls
    void Render(Graphics::IDeviceAPIContext *context);
    // (3) called before death or disabling to destroy possibly created resources
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

private:
    const Engine::FScene *_scene;
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
