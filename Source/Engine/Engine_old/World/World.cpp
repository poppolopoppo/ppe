// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "World.h"

#include "Lighting/LightingEnvironment.h"

#include "Core/Diagnostic/Logger.h"

#include "Core.Logic/EntityManager.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWorld::FWorld(const char *name, IServiceProvider *serviceProvider)
:   _name(name)
,   _status(EWorldStatus::BeforeInitialize)
,   _serviceProvider(serviceProvider)
,   _revision(0)
,   _timespeed(1.0f) {
    Assert(name);
    Assert(serviceProvider);
}
//----------------------------------------------------------------------------
FWorld::~FWorld() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FWorld::Pause() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _timespeed = 0;
}
//----------------------------------------------------------------------------
bool FWorld::IsPaused() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return 0 == _timespeed;
}
//----------------------------------------------------------------------------
void FWorld::TogglePause() {
    THIS_THREADRESOURCE_CHECKACCESS();

    if (IsPaused())
        SetSpeed(1.0f);
    else
        Pause();
}
//----------------------------------------------------------------------------
void FWorld::SetSpeed(float value) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(value > 0 /* use Pause() for == 0 */);

    _timespeed = value;
}
//----------------------------------------------------------------------------
void FWorld::SetLighting(FLightingEnvironment *lighting) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(lighting);

    _lighting.reset(lighting);
}
//----------------------------------------------------------------------------
void FWorld::Initialize() {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[FWorld] Initialize world \"{0}\" ...", _name.c_str());

    ChangeStatus_(EWorldStatus::BeforeInitialize);
    {
        ChangeStatus_(EWorldStatus::Initialize);
        /**********************************************************************/

        _timeline.Tick();
        _lighting = new FLightingEnvironment();

        Assert(!_logic);
        _logic.reset(new Logic::FEntityManager());
        _logic->Initialize();

        /**********************************************************************/
    }
    ChangeStatus_(EWorldStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void FWorld::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[FWorld] Destroy world \"{0}\" ...", _name.c_str());

    ChangeStatus_(EWorldStatus::BeforeDestroy);
    {
        ChangeStatus_(EWorldStatus::Destroy);
        /**********************************************************************/

        Assert(_logic);
        _logic->Destroy();
        _logic.reset();

        RemoveRef_AssertReachZero(_lighting);

        /**********************************************************************/
    }
    ChangeStatus_(EWorldStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void FWorld::Update(const FTimeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(EWorldStatus::BeforeUpdate);
    {
        ChangeStatus_(EWorldStatus::Update);
        /**********************************************************************/

        ++_revision;

        _timeline.Tick(timeline, _timespeed);
        _logic->Update(_timeline);

        /**********************************************************************/
    }
    ChangeStatus_(EWorldStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void FWorld::ChangeStatus_(EWorldStatus value) {
    _status = value;
    switch (value)
    {
    case EWorldStatus::BeforeInitialize: _onBeforeInitialize(this); break;
    case EWorldStatus::AfterInitialize: _onAfterInitialize(this); break;
    case EWorldStatus::BeforeUpdate: _onBeforeUpdate(this); break;
    case EWorldStatus::AfterUpdate: _onAfterUpdate(this); break;
    case EWorldStatus::BeforeDestroy: _onBeforeDestroy(this); break;
    case EWorldStatus::AfterDestroy: _onAfterDestroy(this); break;
    default:
        break;
    } 
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWorld::Start() {
    NOOP;
}
//----------------------------------------------------------------------------
void FWorld::Shutdown() {
    NOOP;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
