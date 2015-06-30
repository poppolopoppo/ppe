#pragma once

#include "Core.Engine/Engine.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/Effect/SharedConstantBuffer.h"
#include "Core.Engine/Effect/SharedConstantBufferFactory.h"
#include "Core.Engine/Material/MaterialDatabase.h"
#include "Core.Engine/Render/RenderTree.h"
#include "Core.Engine/Scene/SceneObserver.h"

namespace Core {
class Timeline;

namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ICamera;
typedef RefPtr<ICamera> PCamera;
struct VariabilitySeed;
FWD_REFPTR(World);
//----------------------------------------------------------------------------
class Scene : public RefCountable, Meta::ThreadResource {
public:
    Scene(  const char *name,
            ICamera *camera,
            const Engine::World *world,
            const Engine::MaterialDatabase *materialDatabase );
    ~Scene();

    const String& Name() const { THIS_THREADRESOURCE_CHECKACCESS(); return _name; }
    SceneStatus Status() const { THIS_THREADRESOURCE_CHECKACCESS(); return _status; }

    const ICamera *Camera() const { THIS_THREADRESOURCE_CHECKACCESS(); return _camera.get(); }
    const Engine::World *World() const { THIS_THREADRESOURCE_CHECKACCESS(); return _world.get(); }

    Engine::MaterialDatabase *MaterialDatabase() { THIS_THREADRESOURCE_CHECKACCESS(); return &_materialDatabase; }
    const Engine::MaterialDatabase *MaterialDatabase() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_materialDatabase; }

    Engine::RenderTree *RenderTree() { THIS_THREADRESOURCE_CHECKACCESS(); return &_renderTree; }
    const Engine::RenderTree *RenderTree() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_renderTree; }

    Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() { THIS_THREADRESOURCE_CHECKACCESS(); return &_sharedConstantBufferFactory; }
    const Engine::SharedConstantBufferFactory *SharedConstantBufferFactory() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_sharedConstantBufferFactory; }

    void Initialize(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void Update(const Timeline& timeline);
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, VariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContextEncapsulator *context);
    
    SceneEvent& OnBeforeInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeInitialize; }
    SceneEvent& OnAfterInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterInitialize; }
    SceneEvent& OnBeforeUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeUpdate; }
    SceneEvent& OnAfterUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterUpdate; }
    SceneEvent& OnBeforePrepare() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforePrepare; }
    SceneEvent& OnAfterPrepare() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterPrepare; }
    SceneEvent& OnBeforeRender() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeRender; }
    SceneEvent& OnAfterRender() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterRender; }
    SceneEvent& OnBeforeDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeDestroy; }
    SceneEvent& OnAfterDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterDestroy; }

private:
    void ChangeStatus_(SceneStatus value);

    String _name;
    SceneStatus _status;

    PCamera _camera;
    PCWorld _world;

    Engine::MaterialDatabase _materialDatabase;
    Engine::RenderTree _renderTree;
    Engine::SharedConstantBufferFactory _sharedConstantBufferFactory;

    mutable SceneEvent _onBeforeInitialize;
    mutable SceneEvent _onAfterInitialize;
    mutable SceneEvent _onBeforeUpdate;
    mutable SceneEvent _onAfterUpdate;
    mutable SceneEvent _onBeforePrepare;
    mutable SceneEvent _onAfterPrepare;
    mutable SceneEvent _onBeforeRender;
    mutable SceneEvent _onAfterRender;
    mutable SceneEvent _onBeforeDestroy;
    mutable SceneEvent _onAfterDestroy;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
