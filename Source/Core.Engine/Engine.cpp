#include "stdafx.h"

#include "Engine.h"

#include "Effect/Effect.h"
#include "Material/MaterialConstNames.h"
#include "Mesh/MeshName.h"
#include "World/World.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Engine {
POOL_TAG_DEF(Engine);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void EngineStartup::Start() {
    // 0 - Pool allocator tag
    POOL_TAG(Engine)::Start();
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
    POOL_TAG(Engine)::Shutdown();
}
//----------------------------------------------------------------------------
void EngineStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Engine)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
