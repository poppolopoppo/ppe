#include "stdafx.h"

#include "Engine.h"

#include "Effect/Effect.h"
#include "Material/MaterialConstNames.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void EngineStartup::Start() {
    // 1 - Effect
    Effect::Start();
    // 2 - Material
    MaterialConstNames::Startup();
}
//----------------------------------------------------------------------------
void EngineStartup::Shutdown() {
    // 2 - Material
    MaterialConstNames::Shutdown();
    // 1 - Effect
    Effect::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
