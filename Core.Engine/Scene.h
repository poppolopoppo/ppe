#pragma once

#include "Engine.h"

#include "Core/RefPtr.h"
#include "Core/String.h"
#include "Core/ThreadResource.h"

#include "ISceneObserver.h"
#include "MaterialDatabase.h"
#include "RenderTree.h"

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

    const SceneObserverContainer& Observers() const { THIS_THREADRESOURCE_CHECKACCESS(); return _observers; }

    void Initialize(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void Update(const Timeline& timeline);
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, VariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContextEncapsulator *context);

    void RegisterObserver(SceneEvent::Type eventFlags, ISceneObserver *observer);
    void UnregisterObserver(SceneEvent::Type eventFlags, ISceneObserver *observer);

private:
    String _name;
    SceneStatus _status;

    PCamera _camera;
    PCWorld _world;

    Engine::MaterialDatabase _materialDatabase;
    Engine::RenderTree _renderTree;
    SceneObserverContainer _observers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
