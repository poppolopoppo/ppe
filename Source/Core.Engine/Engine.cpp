#include "stdafx.h"

#include "Engine.h"

#include "Effect/Effect.h"
#include "Material/MaterialConstNames.h"
#include "Mesh/MeshName.h"
#include "World/World.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
POOLTAG_DEF(Engine);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void EngineStartup::Start() {
    // 0 - Pool allocator tag
    POOLTAG(Engine)::Start();
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
    // 0 - Pool allocator tag
    POOLTAG(Engine)::Shutdown();
}
//----------------------------------------------------------------------------
void EngineStartup::ClearAll_UnusedMemory() {
    POOLTAG(Engine)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
