#pragma once

#include "Engine.h"

#include "Core/RefPtr.h"

#include "MaterialDatabase.h"
#include "MaterialVariability.h"

namespace Core {
class Timeline;

namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IServiceProvider;
FWD_REFPTR(DefaultEffectCompilerService);
FWD_REFPTR(DefaultTextureCacheService);
FWD_REFPTR(DefaultRenderSurfaceService);
FWD_REFPTR(Scene);
FWD_REFPTR(World);
//----------------------------------------------------------------------------
class RenderContext : public RefCountable {
public:
    RenderContext(IServiceProvider *services, size_t textureCacheSizeInBytes);
    ~RenderContext();

    Engine::MaterialDatabase *MaterialDatabase() { return &_materialDatabase; }
    const Engine::MaterialDatabase *MaterialDatabase() const { return &_materialDatabase; }

    DefaultRenderSurfaceService *RenderSurfaceService() { return _renderSurfaceService.get(); }
    const DefaultRenderSurfaceService *RenderSurfaceService() const { return _renderSurfaceService.get(); }

    DefaultEffectCompilerService *EffectCompilerService() { return _effectCompilerService.get(); }
    const DefaultEffectCompilerService *EffectCompilerService() const { return _effectCompilerService.get(); }

    DefaultTextureCacheService *TextureCacheService() { return _textureCacheService.get(); }
    const DefaultTextureCacheService *TextureCacheService() const { return _textureCacheService.get(); }

    VariabilitySeed *VariabilitySeeds() { return _variabilitySeeds; }
    const VariabilitySeed *VariabilitySeeds() const { return _variabilitySeeds; }

    void FrameTick() { _variabilitySeeds[size_t(MaterialVariability::Frame)].Next(); }

    void UpdateAndPrepare(  Graphics::IDeviceAPIEncapsulator *device,
                            const Timeline& timeline,
                            World *const world,
                            const MemoryView<Scene *const>& scenes );

    void Render(Graphics::IDeviceAPIContextEncapsulator *context,
                const MemoryView<Scene *const>& scenes );

    void Clear();

private:
    IServiceProvider *_services;
    VariabilitySeed _variabilitySeeds[VariabilitySeed::Count];
    Engine::MaterialDatabase _materialDatabase;

    PDefaultRenderSurfaceService _renderSurfaceService;
    PDefaultEffectCompilerService _effectCompilerService;
    PDefaultTextureCacheService _textureCacheService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
