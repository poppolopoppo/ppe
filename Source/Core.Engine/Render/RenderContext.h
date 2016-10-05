#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Material/MaterialDatabase.h"
#include "Core.Engine/Material/MaterialVariability.h"

namespace Core {
class FTimeline;

namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IServiceProvider;
FWD_REFPTR(DefaultEffectCompilerService);
FWD_REFPTR(DefaultRenderSurfaceService);
FWD_REFPTR(DefaultSharedConstantBufferFactoryService);
FWD_REFPTR(DefaultTextureCacheService);
FWD_REFPTR(Scene);
FWD_REFPTR(World);
//----------------------------------------------------------------------------
class FRenderContext : public FRefCountable {
public:
    FRenderContext(IServiceProvider *services, size_t textureCacheSizeInBytes);
    ~FRenderContext();

    Engine::FMaterialDatabase *FMaterialDatabase() { return &_materialDatabase; }
    const Engine::FMaterialDatabase *FMaterialDatabase() const { return &_materialDatabase; }


    FDefaultRenderSurfaceService *RenderSurfaceService() { return _renderSurfaceService.get(); }
    const FDefaultRenderSurfaceService *RenderSurfaceService() const { return _renderSurfaceService.get(); }
    
    FDefaultTextureCacheService *TextureCacheService() { return _textureCacheService.get(); }
    const FDefaultTextureCacheService *TextureCacheService() const { return _textureCacheService.get(); }

    FDefaultSharedConstantBufferFactoryService *SharedConstantBufferFactoryService() { return _sharedConstantBufferFactoryService.get(); }
    const FDefaultSharedConstantBufferFactoryService *SharedConstantBufferFactoryService() const { return _sharedConstantBufferFactoryService.get(); }

    FDefaultEffectCompilerService *EffectCompilerService() { return _effectCompilerService.get(); }
    const FDefaultEffectCompilerService *EffectCompilerService() const { return _effectCompilerService.get(); }



    FVariabilitySeed *VariabilitySeeds() { return _variabilitySeeds; }
    const FVariabilitySeed *VariabilitySeeds() const { return _variabilitySeeds; }

    void FrameTick() { _variabilitySeeds[size_t(EMaterialVariability::FFrame)].Next(); }

    void UpdateAndPrepare(  Graphics::IDeviceAPIEncapsulator *device,
                            const FTimeline& timeline,
                            FWorld *const world,
                            const TMemoryView<FScene *const>& scenes );

    void Render(Graphics::IDeviceAPIContext *context,
                const TMemoryView<FScene *const>& scenes );

    void Clear();

private:
    IServiceProvider *_services;
    FVariabilitySeed _variabilitySeeds[FVariabilitySeed::Count];
    Engine::FMaterialDatabase _materialDatabase;

    PDefaultRenderSurfaceService _renderSurfaceService;
    PDefaultTextureCacheService _textureCacheService;
    PDefaultSharedConstantBufferFactoryService _sharedConstantBufferFactoryService;
    PDefaultEffectCompilerService _effectCompilerService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
