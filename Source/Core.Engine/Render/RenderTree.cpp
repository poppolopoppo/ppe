#include "stdafx.h"

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
RenderTree::RenderTree(const Engine::Scene *scene, IServiceProvider *serviceProvider)
:   _scene(scene)
,   _renderSurfaceService(ENGINESERVICE_PTR(IRenderSurfaceService, serviceProvider))
,   _effectCompilerService(ENGINESERVICE_PTR(IEffectCompilerService, serviceProvider))
,   _textureCacheService(ENGINESERVICE_PTR(ITextureCacheService, serviceProvider)) {
    Assert(scene);
}
//----------------------------------------------------------------------------
RenderTree::~RenderTree() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
Engine::RenderSurfaceManager *RenderTree::RenderSurfaceManager() const {
    return _renderSurfaceService->Manager();
}
//----------------------------------------------------------------------------
Engine::EffectCompiler *RenderTree::EffectCompiler() const {
    return _effectCompilerService->EffectCompiler();
}
//----------------------------------------------------------------------------
TextureCache *RenderTree::TextureCache() const {
    return _textureCacheService->TextureCache();
}
//----------------------------------------------------------------------------
void RenderTree::Add(AbstractRenderLayer *layer) {
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
void RenderTree::Remove(AbstractRenderLayer *layer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(layer);

    const auto it = FindFirstOf(_layers, PAbstractRenderLayer(layer));
    Assert(_layers.end() != it);

    _layers.erase(it);
}
//----------------------------------------------------------------------------
bool RenderTree::TryGet(const char *name, AbstractRenderLayer **layer) const {
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
bool RenderTree::TryGet(const char *name, PAbstractRenderLayer& player) const {
    AbstractRenderLayer *layer = nullptr;
    const bool result = TryGet(name, &layer);

    player = layer;
    return result;
}
//----------------------------------------------------------------------------
void RenderTree::Prepare(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase,  VariabilitySeed *seeds) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);

    for (PAbstractRenderLayer& layer : _layers)
        layer->Prepare(device, materialDatabase, this, seeds);

    // tries to fetch possibly loaded textures after each prepare :
    TextureCache()->Update();
}
//----------------------------------------------------------------------------
void RenderTree::Render(Graphics::IDeviceAPIContext *context) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(context);

    GRAPHICS_DIAGNOSTICS_BEGINEVENT(context->Diagnostics(), _scene->Name().c_str());

    for (PAbstractRenderLayer& layer : _layers)
        layer->Render(context);

    GRAPHICS_DIAGNOSTICS_ENDEVENT(context->Diagnostics());
}
//----------------------------------------------------------------------------
void RenderTree::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
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
