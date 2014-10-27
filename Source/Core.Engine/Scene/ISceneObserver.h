#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Observer.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Scene;
//----------------------------------------------------------------------------
namespace SceneEvent {
    enum Type           : size_t {
    BeforeInitialize    = 1<<0,
    AfterInitialize     = 1<<1,

    BeforeUpdate        = 1<<2,
    AfterUpdate         = 1<<3,

    BeforePrepare       = 1<<4,
    AfterPrepare        = 1<<5,

    BeforeRender        = 1<<6,
    AfterRender         = 1<<7,

    BeforeDestroy       = 1<<8,
    AfterDestroy        = 1<<9,
    };
}
//----------------------------------------------------------------------------
enum class SceneStatus : size_t {
    BeforeInitialize    = 0,
    Initialize          ,
    AfterInitialize     ,
    BeforeUpdate        ,
    Update              ,
    AfterUpdate         ,
    BeforePrepare       ,
    Prepare             ,
    AfterPrepare        ,
    BeforeRender        ,
    Render              ,
    AfterRender         ,
    BeforeDestroy       ,
    Destroy             ,
    AfterDestroy        ,
};
//----------------------------------------------------------------------------
using ISceneObserver = IObserver<SceneEvent::Type, Scene *>;
//----------------------------------------------------------------------------
using SceneObserverContainer = VECTOR(Scene, ISceneObserver::entry_type);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
