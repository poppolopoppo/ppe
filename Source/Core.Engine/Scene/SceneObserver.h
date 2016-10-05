#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Meta/Delegate.h"
#include "Core/Meta/Event.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FScene;
//----------------------------------------------------------------------------
enum class ESceneStatus : size_t {
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
using FSceneDelegate = TDelegate<void (*)(FScene *)>;
//----------------------------------------------------------------------------
using FSceneEvent = TEvent<FSceneDelegate>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
