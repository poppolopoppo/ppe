#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Meta/Delegate.h"
#include "Core/Meta/Event.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Scene;
//----------------------------------------------------------------------------
enum class SceneStatus : size_t {
    Invalid             = 0,
    BeforeInitialize    ,
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
using SceneDelegate = Delegate<void (*)(Scene *)>;
//----------------------------------------------------------------------------
using SceneEvent = Event<SceneDelegate>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
