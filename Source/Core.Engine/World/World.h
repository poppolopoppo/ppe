#pragma once

#include "Core.Engine/Engine.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Time/Timeline.h"

#include "Core.Engine/World/WorldObserver.h"

namespace Core {

namespace Logic {
class EntityManager;
typedef UniquePtr<EntityManager> UEntityManager;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IServiceProvider;
FWD_REFPTR(LightingEnvironment);
//----------------------------------------------------------------------------
class World : public RefCountable, Meta::ThreadResource {
public:
    World(const char *name, IServiceProvider *serviceProvider);
    ~World();

    const String& Name() const { THIS_THREADRESOURCE_CHECKACCESS(); return _name; }

    WorldStatus Status() const { THIS_THREADRESOURCE_CHECKACCESS(); return _status; }

    size_t Revision() const { return _revision; }

    float Speed() const { return _timespeed; }
    const Timeline& Time() const { return _timeline; }

    LightingEnvironment *Lighting() { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    const LightingEnvironment *Lighting() const { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    void SetLighting(LightingEnvironment *lighting);

    Logic::EntityManager& EntityManager() { THIS_THREADRESOURCE_CHECKACCESS(); return *_logic; }
    const Logic::EntityManager& EntityManager() const { THIS_THREADRESOURCE_CHECKACCESS(); return *_logic; }

    IServiceProvider *ServiceProvider() const { return _serviceProvider; }

    void Pause();
    bool IsPaused() const;
    void TogglePause();
    void SetSpeed(float value);

    void Initialize();
    void Destroy();

    void Update(const Timeline& timeline);

    WorldEvent& OnBeforeInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeInitialize; }
    WorldEvent& OnAfterInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterInitialize; }
    WorldEvent& OnBeforeUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeUpdate; }
    WorldEvent& OnAfterUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterUpdate; }
    WorldEvent& OnBeforeDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeDestroy; }
    WorldEvent& OnAfterDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterDestroy; }

    static void Start();
    static void Shutdown();

private:
    void ChangeStatus_(WorldStatus value);

    String _name;
    WorldStatus _status;
    size_t _revision;

    float _timespeed;
    Timeline _timeline;

    Logic::UEntityManager _logic;
    PLightingEnvironment _lighting;
    IServiceProvider *const _serviceProvider;

    mutable WorldEvent _onBeforeInitialize;
    mutable WorldEvent _onAfterInitialize;
    mutable WorldEvent _onBeforeUpdate;
    mutable WorldEvent _onAfterUpdate;
    mutable WorldEvent _onBeforeDestroy;
    mutable WorldEvent _onAfterDestroy;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
