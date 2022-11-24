// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderTree.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceDiagnostics.h"

#include "Layers/AbstractRenderLayer.h"
#include "Scene/Scene.h"
#include "Service/EffectCompilerService.h"
#include "Service/IServiceProvider.h"
#include "Service/RenderSurfaceService.h"
#include "Service/TextureCacheService.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderTree::FRenderTree(const Engine::FScene *scene, IServiceProvider *serviceProvider)
:   _scene(scene)
,   _renderSurfaceService(ENGINESERVICE_PTR(IRenderSurfaceService, serviceProvider))
,   _effectCompilerService(ENGINESERVICE_PTR(IEffectCompilerService, serviceProvider))
,   _textureCacheService(ENGINESERVICE_PTR(ITextureCacheService, serviceProvider)) {
    Assert(scene);
}
//----------------------------------------------------------------------------
FRenderTree::~FRenderTree() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
Engine::FRenderSurfaceManager *FRenderTree::FRenderSurfaceManager() const {
    return _renderSurfaceService->Manager();
}
//----------------------------------------------------------------------------
Engine::FEffectCompiler *FRenderTree::FEffectCompiler() const {
    return _effectCompilerService->EffectCompiler();
}
//----------------------------------------------------------------------------
FTextureCache *FRenderTree::FTextureCache() const {
    return _textureCacheService->TextureCache();
}
//----------------------------------------------------------------------------
void FRenderTree::Add(FAbstractRenderLayer *layer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(layer);

#ifdef _DEBUG
    if (layer->Exported())
        for (const PAbstractRenderLayer& l : _layers)
            if (l->Exported())
                Assert(l->Name() != layer->Name());
#endif

    _layers.emplace_back(layer);
}
//----------------------------------------------------------------------------
void FRenderTree::Remove(FAbstractRenderLayer *layer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(layer);

    const auto it = FindFirstOf(_layers, PAbstractRenderLayer(layer));
    Assert(_layers.end() != it);

    _layers.erase(it);
}
//----------------------------------------------------------------------------
bool FRenderTree::TryGet(const char *name, FAbstractRenderLayer **layer) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(layer);

    for (const PAbstractRenderLayer& l : _layers)
        if (l->Exported() && l->Name() == name) {
            *layer = l.get();
            return true;
        }

    return false;
}
//----------------------------------------------------------------------------
bool FRenderTree::TryGet(const char *name, PAbstractRenderLayer& player) const {
    FAbstractRenderLayer *layer = nullptr;
    const bool result = TryGet(name, &layer);

    player = layer;
    return result;
}
//----------------------------------------------------------------------------
void FRenderTree::Prepare(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase,  FVariabilitySeed *seeds) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);

    for (PAbstractRenderLayer& layer : _layers)
        layer->Prepare(device, materialDatabase, this, seeds);

    // tries to fetch possibly loaded textures after each prepare :
    FTextureCache()->Update();
}
//----------------------------------------------------------------------------
void FRenderTree::Render(Graphics::IDeviceAPIContext *context) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(context);

    GRAPHICS_DIAGNOSTICS_BEGINEVENT(context->Diagnostics(), _scene->Name().c_str());

    for (PAbstractRenderLayer& layer : _layers)
        layer->Render(context);

    GRAPHICS_DIAGNOSTICS_ENDEVENT(context->Diagnostics());
}
//----------------------------------------------------------------------------
void FRenderTree::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);

    for (PAbstractRenderLayer& layer : _layers)
        layer->Destroy(device, this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
