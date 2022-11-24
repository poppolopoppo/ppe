// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderContext.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core/Maths/Units.h"

#include "Material/IMaterialParameter.h"
#include "Scene/Scene.h"
#include "Service/EffectCompilerService.h"
#include "Service/IServiceProvider.h"
#include "Service/RenderSurfaceService.h"
#include "Service/Service_fwd.h"
#include "Service/SharedConstantBufferFactoryService.h"
#include "Service/TextureCacheService.h"
#include "World/World.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderContext::FRenderContext(IServiceProvider *services, size_t textureCacheSizeInBytes)
:   _services(services)
,   _renderSurfaceService(new FDefaultRenderSurfaceService())
,   _textureCacheService(new FDefaultTextureCacheService(textureCacheSizeInBytes))
,   _sharedConstantBufferFactoryService(new FDefaultSharedConstantBufferFactoryService())
,   _effectCompilerService(new FDefaultEffectCompilerService()) {
    Assert(services);

    for (size_t i = 0; i < FVariabilitySeed::Count; ++i)
        _variabilitySeeds[i].Reset();

    RegisterDefaultMaterialParameters(&_materialDatabase);

    ENGINESERVICE_REGISTER(IRenderSurfaceService, _services, _renderSurfaceService.get());
    ENGINESERVICE_REGISTER(ITextureCacheService, _services, _textureCacheService.get());
    ENGINESERVICE_REGISTER(ISharedConstantBufferFactoryService, _services, _sharedConstantBufferFactoryService.get());
    ENGINESERVICE_REGISTER(IEffectCompilerService, _services, _effectCompilerService.get());
}
//----------------------------------------------------------------------------
FRenderContext::~FRenderContext() {
    ENGINESERVICE_UNREGISTER(IEffectCompilerService, _services, _effectCompilerService.get());
    ENGINESERVICE_UNREGISTER(ISharedConstantBufferFactoryService, _services, _sharedConstantBufferFactoryService.get());
    ENGINESERVICE_UNREGISTER(ITextureCacheService, _services, _textureCacheService.get());
    ENGINESERVICE_UNREGISTER(IRenderSurfaceService, _services, _renderSurfaceService.get());

    RemoveRef_AssertReachZero(_effectCompilerService);
    RemoveRef_AssertReachZero(_sharedConstantBufferFactoryService);
    RemoveRef_AssertReachZero(_textureCacheService);
    RemoveRef_AssertReachZero(_renderSurfaceService);
}
//----------------------------------------------------------------------------
void FRenderContext::UpdateAndPrepare(
    Graphics::IDeviceAPIEncapsulator *device,
    const FTimeline& timeline,
    FWorld *const world,
    const TMemoryView<FScene *const>& scenes ) {
    Assert(device);
    Assert(world);

    _variabilitySeeds[size_t(EMaterialVariability::FWorld)].Next();

    world->Update(timeline);

    for (FScene *scene : scenes) {
        scene->Update(timeline);
    }

    for (FScene *scene : scenes) {
        _variabilitySeeds[size_t(EMaterialVariability::FScene)].Next();
        scene->Prepare(device, _variabilitySeeds);
    }
}
//----------------------------------------------------------------------------
void FRenderContext::Render(
    Graphics::IDeviceAPIContext *context,
    const TMemoryView<FScene *const>& scenes ) {
    Assert(context);

    for (FScene *scene : scenes)
        scene->Render(context);
}
//----------------------------------------------------------------------------
void FRenderContext::Clear() {
    _effectCompilerService->EffectCompiler()->Clear();
    _sharedConstantBufferFactoryService->SharedConstantBufferFactory()->Clear();
    _textureCacheService->TextureCache()->Clear();
    _renderSurfaceService->Manager()->Clear();

    _materialDatabase.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
