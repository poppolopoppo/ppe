#pragma once

#include "Core.Engine/Engine.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/World/WorldObserver.h"
#include "Core.Engine/World/WorldTime.h"

namespace Core {
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

    WorldTime& Time() { THIS_THREADRESOURCE_CHECKACCESS(); return _time; }
    const WorldTime& Time() const { THIS_THREADRESOURCE_CHECKACCESS(); return _time; }

    WorldStatus Status() const { THIS_THREADRESOURCE_CHECKACCESS(); return _status; }

    LightingEnvironment *Lighting() { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    const LightingEnvironment *Lighting() const { THIS_THREADRESOURCE_CHECKACCESS(); return _lighting.get(); }
    void SetLighting(LightingEnvironment *lighting);

    const WorldObserverContainer& Observers() const { THIS_THREADRESOURCE_CHECKACCESS(); return _observers; }

    IServiceProvider *ServiceProvider() const { return _serviceProvider; }

    void Pause();
    bool IsPaused() const;
    void TogglePause();
    void SetSpeed(float value);

    void Initialize();
    void Destroy();

    void Update(const Timeline& timeline);

    void RegisterObserver(WorldEvent::Type eventFlags, const WorldObserver& observer);
    void UnregisterObserver(WorldEvent::Type eventFlags, const WorldObserver& observer);

private:
    String _name;
    WorldTime _time;
    WorldStatus _status;

    PLightingEnvironment _lighting;
    WorldObserverContainer _observers;
    IServiceProvider *const _serviceProvider;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
