#include "stdafx.h"

#include "Scene.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/Time/Timeline.h"

#include "Camera/Camera.h"
#include "Material/MaterialVariability.h"
#include "Service/IServiceProvider.h"
#include "World/World.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Scene::Scene(
    const char *name,
    ICamera *camera,
    const Engine::World *world,
    const Engine::MaterialDatabase *materialDatabase
    )
:   _name(name)
,   _status(SceneStatus::BeforeInitialize)
,   _camera(camera)
,   _world(world)
,   _materialDatabase(materialDatabase)
,   _renderTree(this, world->ServiceProvider()) {
    Assert(name);
    Assert(camera);
    Assert(world);
}
//----------------------------------------------------------------------------
Scene::~Scene() {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(SceneStatus::BeforeInitialize == _status);
    Assert(_observers.empty());
}
//----------------------------------------------------------------------------
void Scene::Initialize(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Information, L"[Scene] Initialize scene \"{0}\" ...", _name.c_str());

    NotifyObservers(_observers, SceneEvent::BeforeInitialize, this);
    Move_AssertEquals(&_status, SceneStatus::Initialize, SceneStatus::BeforeInitialize);
    {
        /**********************************************************************/

        NOOP;

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, SceneStatus::AfterInitialize, SceneStatus::Initialize);
    NotifyObservers(_observers, SceneEvent::AfterInitialize, this);

    Move_AssertEquals(&_status, SceneStatus::BeforeUpdate, SceneStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void Scene::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Information, L"[Scene] Destroy scene \"{0}\" ...", _name.c_str());

    Move_AssertEquals(&_status, SceneStatus::BeforeDestroy, SceneStatus::BeforeUpdate);

    NotifyObservers(_observers, SceneEvent::BeforeDestroy, this);
    Move_AssertEquals(&_status, SceneStatus::Destroy, SceneStatus::BeforeDestroy);
    {
        /**********************************************************************/

        _renderTree.Destroy(device);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, SceneStatus::AfterDestroy, SceneStatus::Destroy);
    NotifyObservers(_observers, SceneEvent::AfterDestroy, this);

    Move_AssertEquals(&_status, SceneStatus::BeforeInitialize, SceneStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void Scene::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    NotifyObservers(_observers, SceneEvent::BeforeUpdate, this);
    Move_AssertEquals(&_status, SceneStatus::Update, SceneStatus::BeforeUpdate);
    {
        /**********************************************************************/

        _camera->Update(timeline);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, SceneStatus::AfterUpdate, SceneStatus::Update);
    NotifyObservers(_observers, SceneEvent::AfterUpdate, this);

    Move_AssertEquals(&_status, SceneStatus::BeforePrepare, SceneStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void Scene::Prepare(Graphics::IDeviceAPIEncapsulator *device, VariabilitySeed *seeds) {
    Assert(device);
    THIS_THREADRESOURCE_CHECKACCESS();

    NotifyObservers(_observers, SceneEvent::BeforePrepare, this);
    Move_AssertEquals(&_status, SceneStatus::Prepare, SceneStatus::BeforePrepare);
    {
        /**********************************************************************/

        seeds[size_t(MaterialVariability::Scene)].Next();

        _renderTree.Prepare(device, &_materialDatabase, seeds);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, SceneStatus::AfterPrepare, SceneStatus::Prepare);
    NotifyObservers(_observers, SceneEvent::AfterPrepare, this);

    Move_AssertEquals(&_status, SceneStatus::BeforeRender, SceneStatus::AfterPrepare);
}
//----------------------------------------------------------------------------
void Scene::Render(Graphics::IDeviceAPIContextEncapsulator *context) {
    Assert(context);
    THIS_THREADRESOURCE_CHECKACCESS();

    NotifyObservers(_observers, SceneEvent::BeforeRender, this);
    Move_AssertEquals(&_status, SceneStatus::Render, SceneStatus::BeforeRender);
    {
        /**********************************************************************/

        _renderTree.Render(context);

        /**********************************************************************/
    }
    Move_AssertEquals(&_status, SceneStatus::AfterRender, SceneStatus::Render);
    NotifyObservers(_observers, SceneEvent::AfterRender, this);

    Move_AssertEquals(&_status, SceneStatus::BeforeUpdate, SceneStatus::AfterRender);
}
//----------------------------------------------------------------------------
void Scene::RegisterObserver(SceneEvent::Type eventFlags, const SceneObserver& observer) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Core::RegisterObserver(_observers, observer, eventFlags);
}
//----------------------------------------------------------------------------
void Scene::UnregisterObserver(SceneEvent::Type eventFlags, const SceneObserver& observer) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Core::UnregisterObserver(_observers, observer, eventFlags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
