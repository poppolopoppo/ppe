#include "stdafx.h"

#include "RenderContext.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core/Maths/Units.h"

#include "Material/Parameters/AbstractMaterialParameter.h"
#include "Scene/Scene.h"
#include "Service/EffectCompilerService.h"
#include "Service/IServiceProvider.h"
#include "Service/RenderSurfaceService.h"
#include "Service/Service_fwd.h"
#include "Service/TextureCacheService.h"
#include "World/World.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderContext::RenderContext(IServiceProvider *services, size_t textureCacheSizeInBytes)
:   _services(services)
,   _renderSurfaceService(new DefaultRenderSurfaceService())
,   _effectCompilerService(new DefaultEffectCompilerService())
,   _textureCacheService(new DefaultTextureCacheService(textureCacheSizeInBytes)) {
    Assert(services);

    for (size_t i = 0; i < VariabilitySeed::Count; ++i)
        _variabilitySeeds[i].Reset();

    RegisterDefaultMaterialParameters(&_materialDatabase);

    ENGINESERVICE_REGISTER(IRenderSurfaceService, _services, _renderSurfaceService.get());
    ENGINESERVICE_REGISTER(ITextureCacheService, _services, _textureCacheService.get());
    ENGINESERVICE_REGISTER(IEffectCompilerService, _services, _effectCompilerService.get());
}
//----------------------------------------------------------------------------
RenderContext::~RenderContext() {
    ENGINESERVICE_UNREGISTER(IEffectCompilerService, _services, _effectCompilerService.get());
    ENGINESERVICE_UNREGISTER(ITextureCacheService, _services, _textureCacheService.get());
    ENGINESERVICE_UNREGISTER(IRenderSurfaceService, _services, _renderSurfaceService.get());

    RemoveRef_AssertReachZero(_effectCompilerService);
    RemoveRef_AssertReachZero(_textureCacheService);
    RemoveRef_AssertReachZero(_renderSurfaceService);
}
//----------------------------------------------------------------------------
void RenderContext::UpdateAndPrepare(
    Graphics::IDeviceAPIEncapsulator *device,
    const Timeline& timeline,
    World *const world,
    const MemoryView<Scene *const>& scenes ) {
    Assert(device);
    Assert(world);

    _variabilitySeeds[size_t(MaterialVariability::World)].Next();

    world->Update(timeline);

    for (Scene *scene : scenes) {
        scene->Update(timeline);
    }

    for (Scene *scene : scenes) {
        _variabilitySeeds[size_t(MaterialVariability::Scene)].Next();
        scene->Prepare(device, _variabilitySeeds);
    }
}
//----------------------------------------------------------------------------
void RenderContext::Render(
    Graphics::IDeviceAPIContextEncapsulator *context,
    const MemoryView<Scene *const>& scenes ) {
    Assert(context);

    for (Scene *scene : scenes)
        scene->Render(context);
}
//----------------------------------------------------------------------------
void RenderContext::Clear() {
    _effectCompilerService->EffectCompiler()->Clear();
    _textureCacheService->TextureCache()->Clear();
    _renderSurfaceService->Manager()->Clear();
    _materialDatabase.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
