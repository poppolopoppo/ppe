#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Meta/Delegate.h"
#include "Core/Meta/Event.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWorld;
//----------------------------------------------------------------------------
enum class EWorldStatus : size_t {
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
using FWorldDelegate = TDelegate<void (*)(FWorld *)>;
//----------------------------------------------------------------------------
using FWorldEvent = TEvent<FWorldDelegate>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
