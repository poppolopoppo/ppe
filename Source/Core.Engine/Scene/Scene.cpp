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
FScene::FScene(
    const char *name,
    ICamera *camera,
    const Engine::FWorld *world,
    const Engine::FMaterialDatabase *materialDatabase
    )
:   _name(name)
,   _status(ESceneStatus::Invalid)
,   _camera(camera)
,   _world(world)
,   _materialDatabase(materialDatabase)
,   _renderTree(this, world->ServiceProvider()) {
    Assert(name);
    Assert(camera);
    Assert(world);
}
//----------------------------------------------------------------------------
FScene::~FScene() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FScene::Initialize(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[FScene] Initialize scene \"{0}\" ...", _name.c_str());

    ChangeStatus_(ESceneStatus::BeforeInitialize);
    {
        ChangeStatus_(ESceneStatus::Update);
        /**********************************************************************/

        _sharedConstantBufferFactory.Start(device);

        /**********************************************************************/
    }
    ChangeStatus_(ESceneStatus::AfterInitialize);
}
//----------------------------------------------------------------------------
void FScene::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();

    LOG(Info, L"[FScene] Destroy scene \"{0}\" ...", _name.c_str());

    ChangeStatus_(ESceneStatus::BeforeDestroy);
    {
        ChangeStatus_(ESceneStatus::Destroy);
        /**********************************************************************/

        _renderTree.Destroy(device);
        _sharedConstantBufferFactory.Shutdown(device);

        /**********************************************************************/
    }
    ChangeStatus_(ESceneStatus::AfterDestroy);
}
//----------------------------------------------------------------------------
void FScene::Update(const FTimeline& timeline) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(ESceneStatus::BeforeUpdate);
    {
        ChangeStatus_(ESceneStatus::Update);
        /**********************************************************************/

        _camera->Update(timeline);

        /**********************************************************************/
    }
    ChangeStatus_(ESceneStatus::AfterUpdate);
}
//----------------------------------------------------------------------------
void FScene::Prepare(Graphics::IDeviceAPIEncapsulator *device, FVariabilitySeed *seeds) {
    Assert(device);
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(ESceneStatus::BeforePrepare);
    {
        ChangeStatus_(ESceneStatus::Prepare);
        /**********************************************************************/

        seeds[size_t(EMaterialVariability::FScene)].Next();

        _renderTree.Prepare(device, &_materialDatabase, seeds);

        /**********************************************************************/
    }
    ChangeStatus_(ESceneStatus::AfterPrepare);
}
//----------------------------------------------------------------------------
void FScene::Render(Graphics::IDeviceAPIContext *context) {
    Assert(context);
    THIS_THREADRESOURCE_CHECKACCESS();

    ChangeStatus_(ESceneStatus::BeforeRender);
    {
        ChangeStatus_(ESceneStatus::Render);
        /**********************************************************************/

        _renderTree.Render(context);

        /**********************************************************************/
    }
    ChangeStatus_(ESceneStatus::AfterRender);
}
//----------------------------------------------------------------------------
void FScene::ChangeStatus_(ESceneStatus value) {
    _status = value;
    switch (value)
    {
    case ESceneStatus::BeforeInitialize: _onBeforeInitialize(this); break;
    case ESceneStatus::AfterInitialize: _onAfterInitialize(this); break;
    case ESceneStatus::BeforeUpdate: _onBeforeUpdate(this); break;
    case ESceneStatus::AfterUpdate: _onAfterUpdate(this); break;
    case ESceneStatus::BeforePrepare: _onBeforePrepare(this); break;
    case ESceneStatus::AfterPrepare: _onAfterPrepare(this); break;
    case ESceneStatus::BeforeRender: _onBeforeRender(this); break;
    case ESceneStatus::AfterRender: _onAfterRender(this); break;
    case ESceneStatus::BeforeDestroy: _onBeforeDestroy(this); break;
    case ESceneStatus::AfterDestroy: _onAfterDestroy(this); break;
    default:
        break;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
