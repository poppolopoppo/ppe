#include "stdafx.h"

#include "World.h"

#include "Lighting/LightingEnvironment.h"

#include "Core/Diagnostic/Logger.h"

#include "Core.Logic/EntityManager.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
World::World(const char *name, IServiceProvider *serviceProvider)
:   _name(name)
,   _status(WorldStatus::BeforeInitialize)
,   _serviceProvider(serviceProvider)
,   _revision(0)
,   _timespeed(1.0f) {
    Assert(name);
    Assert(serviceProvider);
}
//----------------------------------------------------------------------------
World::~World() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void World::Pause() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _timespeed = 0;
}
//----------------------------------------------------------------------------
bool World::IsPaused() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return 0 == _timespeed;
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

    _timespeed = value;
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

    LOG(Info, L"[World] Initialize world \"{0}\" ...", _name.c_str());

    ChangeStatus_(WorldStatus::BeforeInitialize);
    {
        ChangeStatus_(WorldStatus::Initialize);
        /**********************************************************************/

        _timeline.Tick();
        _lighting = new LightingEnvironment();

        Assert(!_logic);
        _logic.reset(new Logic::EntityManager());
        _logic->Initialize();

        /**********************************************************************/
    }
    ChangeStatus_(WorldStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void World::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[World] Destroy world \"{0}\" ...", _name.c_str());

    ChangeStatus_(WorldStatus::BeforeDestroy);
    {
        ChangeStatus_(WorldStatus::Destroy);
        /**********************************************************************/

        Assert(_logic);
        _logic->Destroy();
        _logic.reset();

        RemoveRef_AssertReachZero(_lighting);

        /**********************************************************************/
    }
    ChangeStatus_(WorldStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void World::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(WorldStatus::BeforeUpdate);
    {
        ChangeStatus_(WorldStatus::Update);
        /**********************************************************************/

        ++_revision;

        _timeline.Tick(timeline, _timespeed);
        _logic->Update(_timeline);

        /**********************************************************************/
    }
    ChangeStatus_(WorldStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void World::ChangeStatus_(WorldStatus value) {
    _status = value;
    switch (value)
    {
    case WorldStatus::BeforeInitialize: _onBeforeInitialize(this); break;
    case WorldStatus::AfterInitialize: _onAfterInitialize(this); break;
    case WorldStatus::BeforeUpdate: _onBeforeUpdate(this); break;
    case WorldStatus::AfterUpdate: _onAfterUpdate(this); break;
    case WorldStatus::BeforeDestroy: _onBeforeDestroy(this); break;
    case WorldStatus::AfterDestroy: _onAfterDestroy(this); break;
    default:
        break;
    } 
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void World::Start() {
    NOOP;
}
//----------------------------------------------------------------------------
void World::Shutdown() {
    NOOP;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
