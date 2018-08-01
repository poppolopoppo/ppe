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
class FTimeline;

namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ICamera;
typedef TRefPtr<ICamera> PCamera;
struct FVariabilitySeed;
FWD_REFPTR(World);
//----------------------------------------------------------------------------
class FScene : public FRefCountable, Meta::FThreadResource {
public:
    FScene(  const char *name,
            ICamera *camera,
            const Engine::FWorld *world,
            const Engine::FMaterialDatabase *materialDatabase );
    ~FScene();

    const FString& FName() const { THIS_THREADRESOURCE_CHECKACCESS(); return _name; }
    ESceneStatus Status() const { THIS_THREADRESOURCE_CHECKACCESS(); return _status; }

    const ICamera *FCamera() const { THIS_THREADRESOURCE_CHECKACCESS(); return _camera.get(); }
    const Engine::FWorld *FWorld() const { THIS_THREADRESOURCE_CHECKACCESS(); return _world.get(); }

    Engine::FMaterialDatabase *FMaterialDatabase() { THIS_THREADRESOURCE_CHECKACCESS(); return &_materialDatabase; }
    const Engine::FMaterialDatabase *FMaterialDatabase() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_materialDatabase; }

    Engine::FRenderTree *FRenderTree() { THIS_THREADRESOURCE_CHECKACCESS(); return &_renderTree; }
    const Engine::FRenderTree *FRenderTree() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_renderTree; }

    Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() { THIS_THREADRESOURCE_CHECKACCESS(); return &_sharedConstantBufferFactory; }
    const Engine::FSharedConstantBufferFactory *FSharedConstantBufferFactory() const { THIS_THREADRESOURCE_CHECKACCESS(); return &_sharedConstantBufferFactory; }

    void Initialize(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void Update(const FTimeline& timeline);
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, FVariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContext *context);
    
    FSceneEvent& OnBeforeInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeInitialize; }
    FSceneEvent& OnAfterInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterInitialize; }
    FSceneEvent& OnBeforeUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeUpdate; }
    FSceneEvent& OnAfterUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterUpdate; }
    FSceneEvent& OnBeforePrepare() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforePrepare; }
    FSceneEvent& OnAfterPrepare() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterPrepare; }
    FSceneEvent& OnBeforeRender() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeRender; }
    FSceneEvent& OnAfterRender() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterRender; }
    FSceneEvent& OnBeforeDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeDestroy; }
    FSceneEvent& OnAfterDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterDestroy; }

private:
    void ChangeStatus_(ESceneStatus value);

    FString _name;
    ESceneStatus _status;

    PCamera _camera;
    PCWorld _world;

    Engine::FMaterialDatabase _materialDatabase;
    Engine::FRenderTree _renderTree;
    Engine::FSharedConstantBufferFactory _sharedConstantBufferFactory;

    mutable FSceneEvent _onBeforeInitialize;
    mutable FSceneEvent _onAfterInitialize;
    mutable FSceneEvent _onBeforeUpdate;
    mutable FSceneEvent _onAfterUpdate;
    mutable FSceneEvent _onBeforePrepare;
    mutable FSceneEvent _onAfterPrepare;
    mutable FSceneEvent _onBeforeRender;
    mutable FSceneEvent _onAfterRender;
    mutable FSceneEvent _onBeforeDestroy;
    mutable FSceneEvent _onAfterDestroy;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
