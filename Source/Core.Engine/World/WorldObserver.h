#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Meta/Delegate.h"
#include "Core/Meta/Event.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class World;
//----------------------------------------------------------------------------
enum class WorldStatus : size_t {
    BeforeInitialize    = 0,
    Initialize          ,
    AfterInitialize     ,
    BeforeUpdate        ,
    Update              ,
    AfterUpdate         ,
    BeforeDestroy       ,
    Destroy             ,
    AfterDestroy        ,
};
//----------------------------------------------------------------------------
using WorldDelegate = Delegate<void (*)(World *)>;
//----------------------------------------------------------------------------
using WorldEvent = Event<WorldDelegate>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
