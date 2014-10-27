#include "stdafx.h"

#include "World.h"

#include "Lighting/LightingEnvironment.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
World::World(const char *name, IServiceProvider *serviceProvider)
:   _name(name)
,   _status(WorldStatus::BeforeInitialize)
,   _serviceProvider(serviceProvider) {
    Assert(name);
    Assert(serviceProvider);
}
//----------------------------------------------------------------------------
World::~World() {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(WorldStatus::BeforeInitialize == _status);
    Assert(_observers.empty());
}
//----------------------------------------------------------------------------
void World::Pause() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _time.SetSpeed(0);
}
//----------------------------------------------------------------------------
bool World::IsPaused() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _time.Speed() == 0;
}
//----------------------------------------------------------------------------
void World::TogglePause() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (IsPaused())
        SetSpeed(1.0f);
    else
        Pause();
}
//----------------------------------------------------------------------------
void World::SetSpeed(float value) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(value > 0 /* use Pause() for == 0 */);

    _time.SetSpeed(value);
}
//----------------------------------------------------------------------------
void World::SetLighting(LightingEnvironment *lighting) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(lighting);

    _lighting.reset(lighting);
}
//----------------------------------------------------------------------------
void World::Initialize() {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Information, L"[World] Initialize world \"{0}\" ...", _name.c_str());

    NotifyObservers(_observers, WorldEvent::BeforeInitialize, this);
    Move_AssertEquals(&_status, WorldStatus::Initialize, WorldStatus::BeforeInitialize);
    {
        /**********************************************************************/

        _time.Reset();
        _lighting = new LightingEnvironment();

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, WorldStatus::AfterInitialize, WorldStatus::Initialize);
    NotifyObservers(_observers, WorldEvent::AfterInitialize, this);

    Move_AssertEquals(&_status, WorldStatus::BeforeUpdate, WorldStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void World::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Information, L"[World] Destroy world \"{0}\" ...", _name.c_str());

    Move_AssertEquals(&_status, WorldStatus::BeforeDestroy, WorldStatus::BeforeUpdate);

    NotifyObservers(_observers, WorldEvent::BeforeDestroy, this);
    Move_AssertEquals(&_status, WorldStatus::Destroy, WorldStatus::BeforeDestroy);
    {
        /**********************************************************************/

        RemoveRef_AssertReachZero(_lighting);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, WorldStatus::AfterDestroy, WorldStatus::Destroy);
    NotifyObservers(_observers, WorldEvent::AfterDestroy, this);

    Move_AssertEquals(&_status, WorldStatus::BeforeInitialize, WorldStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void World::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    NotifyObservers(_observers, WorldEvent::BeforeUpdate, this);
    Move_AssertEquals(&_status, WorldStatus::Update, WorldStatus::BeforeUpdate);
    {
        /**********************************************************************/

        _time.Update(timeline);

        if (_lighting)
            _lighting->Update(this);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, WorldStatus::AfterUpdate, WorldStatus::Update);
    NotifyObservers(_observers, WorldEvent::AfterUpdate, this);

    Move_AssertEquals(&_status, WorldStatus::BeforeUpdate, WorldStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void World::RegisterObserver(WorldEvent::Type eventFlags, IWorldObserver *observer) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Core::RegisterObserver(_observers, observer, eventFlags);
}
//----------------------------------------------------------------------------
void World::UnregisterObserver(WorldEvent::Type eventFlags, IWorldObserver *observer) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Core::UnregisterObserver(_observers, observer, eventFlags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
