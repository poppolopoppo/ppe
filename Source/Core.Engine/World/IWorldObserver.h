#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Observer.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class World;
//----------------------------------------------------------------------------
namespace WorldEvent {
    enum Type           : size_t {
    BeforeInitialize    = 1<<0,
    AfterInitialize     = 1<<1,

    BeforeUpdate        = 1<<2,
    AfterUpdate         = 1<<3,

    BeforeDestroy       = 1<<4,
    AfterDestroy        = 1<<5,
    };
}
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
using IWorldObserver = IObserver<WorldEvent::Type, World *>;
//----------------------------------------------------------------------------
using WorldObserverContainer = VECTOR(World, IWorldObserver::entry_type);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
