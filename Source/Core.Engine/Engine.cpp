#include "stdafx.h"

#include "Engine.h"

#include "Effect/Effect.h"
#include "Material/MaterialConstNames.h"
#include "Mesh/MeshName.h"
#include "World/World.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void EngineStartup::Start() {
    // 1 - Effect
    Effect::Start();
    // 2 - Material
    MaterialConstNames::Start();
    // 3 - Mesh
    MeshName::Start(256);
    // 4 - World
    World::Start();
}
//----------------------------------------------------------------------------
void EngineStartup::Shutdown() {
    // 4 - World
    World::Shutdown();
    // 3 - Mesh
    MeshName::Shutdown();
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
