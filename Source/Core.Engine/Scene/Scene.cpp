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
,   _status(SceneStatus::Invalid)
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
}
//----------------------------------------------------------------------------
void Scene::Initialize(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[Scene] Initialize scene \"{0}\" ...", _name.c_str());

    ChangeStatus_(SceneStatus::BeforeInitialize);
    {
        ChangeStatus_(SceneStatus::Update);
        /**********************************************************************/

        _sharedConstantBufferFactory.Start(device);

        /**********************************************************************/
    }
    ChangeStatus_(SceneStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void Scene::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[Scene] Destroy scene \"{0}\" ...", _name.c_str());

    ChangeStatus_(SceneStatus::BeforeDestroy);
    {
        ChangeStatus_(SceneStatus::Destroy);
        /**********************************************************************/

        _renderTree.Destroy(device);
        _sharedConstantBufferFactory.Shutdown(device);

        /**********************************************************************/
    }
    ChangeStatus_(SceneStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void Scene::Update(const Timeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(SceneStatus::BeforeUpdate);
    {
        ChangeStatus_(SceneStatus::Update);
        /**********************************************************************/

        _camera->Update(timeline);

        /**********************************************************************/
    }
    ChangeStatus_(SceneStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void Scene::Prepare(Graphics::IDeviceAPIEncapsulator *device, VariabilitySeed *seeds) {
    Assert(device);
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(SceneStatus::BeforePrepare);
    {
        ChangeStatus_(SceneStatus::Prepare);
        /**********************************************************************/

        seeds[size_t(MaterialVariability::Scene)].Next();

        _renderTree.Prepare(device, &_materialDatabase, seeds);

        /**********************************************************************/
    }
    ChangeStatus_(SceneStatus::AfterPrepare);
}
//----------------------------------------------------------------------------
void Scene::Render(Graphics::IDeviceAPIContext *context) {
    Assert(context);
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(SceneStatus::BeforeRender);
    {
        ChangeStatus_(SceneStatus::Render);
        /**********************************************************************/

        _renderTree.Render(context);

        /**********************************************************************/
    }
    ChangeStatus_(SceneStatus::AfterRender);
}
//----------------------------------------------------------------------------
void Scene::ChangeStatus_(SceneStatus value) {
    _status = value;
    switch (value)
    {
    case SceneStatus::BeforeInitialize: _onBeforeInitialize(this); break;
    case SceneStatus::AfterInitialize: _onAfterInitialize(this); break;
    case SceneStatus::BeforeUpdate: _onBeforeUpdate(this); break;
    case SceneStatus::AfterUpdate: _onAfterUpdate(this); break;
    case SceneStatus::BeforePrepare: _onBeforePrepare(this); break;
    case SceneStatus::AfterPrepare: _onAfterPrepare(this); break;
    case SceneStatus::BeforeRender: _onBeforeRender(this); break;
    case SceneStatus::AfterRender: _onAfterRender(this); break;
    case SceneStatus::BeforeDestroy: _onBeforeDestroy(this); break;
    case SceneStatus::AfterDestroy: _onAfterDestroy(this); break;
    default:
        break;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
