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
class FEntityManager;
typedef TUniquePtr<FEntityManager> UEntityManager;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IServiceProvider;
FWD_REFPTR(LightingEnvironment);
//----------------------------------------------------------------------------
class FWorld : public FRefCountable, Meta::FThreadResource {
public:
    FWorld(const char *name, IServiceProvider *serviceProvider);
    ~FWorld();

    const FString& FName() const { THIS_THREADRESOURCE_CHECKACCESS(); return _name; }

    EWorldStatus Status() const { THIS_THREADRESOURCE_CHECKACCESS(); return _status; }

    size_t Revision() const { return _revision; }

    float Speed() const { return _timespeed; }
    const FTimeline& Time() const { return _timeline; }

    FLightingEnvironment *Lighting() { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    const FLightingEnvironment *Lighting() const { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    void SetLighting(FLightingEnvironment *lighting);

    Logic::FEntityManager& FEntityManager() { THIS_THREADRESOURCE_CHECKACCESS(); return *_logic; }
    const Logic::FEntityManager& FEntityManager() const { THIS_THREADRESOURCE_CHECKACCESS(); return *_logic; }

    IServiceProvider *ServiceProvider() const { return _serviceProvider; }

    void Pause();
    bool IsPaused() const;
    void TogglePause();
    void SetSpeed(float value);

    void Initialize();
    void Destroy();

    void Update(const FTimeline& timeline);

    FWorldEvent& OnBeforeInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeInitialize; }
    FWorldEvent& OnAfterInitialize() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterInitialize; }
    FWorldEvent& OnBeforeUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeUpdate; }
    FWorldEvent& OnAfterUpdate() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterUpdate; }
    FWorldEvent& OnBeforeDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onBeforeDestroy; }
    FWorldEvent& OnAfterDestroy() const { THIS_THREADRESOURCE_CHECKACCESS(); return _onAfterDestroy; }

    static void Start();
    static void Shutdown();

private:
    void ChangeStatus_(EWorldStatus value);

    FString _name;
    EWorldStatus _status;
    size_t _revision;

    float _timespeed;
    FTimeline _timeline;

    Logic::UEntityManager _logic;
    PLightingEnvironment _lighting;
    IServiceProvider *const _serviceProvider;

    mutable FWorldEvent _onBeforeInitialize;
    mutable FWorldEvent _onAfterInitialize;
    mutable FWorldEvent _onBeforeUpdate;
    mutable FWorldEvent _onAfterUpdate;
    mutable FWorldEvent _onBeforeDestroy;
    mutable FWorldEvent _onAfterDestroy;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
