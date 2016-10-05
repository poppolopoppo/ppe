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
void FEngineStartup::Start() {
    // 0 - Pool allocator tag
    POOL_TAG(Engine)::Start();
    // 1 - FEffect
    FEffect::Start();
    // 2 - FMaterial
    FMaterialConstNames::Start();
    // 3 - Mesh
    FMeshName::Start(256);
    // 4 - FWorld
    FWorld::Start();
}
//----------------------------------------------------------------------------
void FEngineStartup::Shutdown() {
    // 4 - FWorld
    FWorld::Shutdown();
    // 3 - Mesh
    FMeshName::Shutdown();
    // 2 - FMaterial
    FMaterialConstNames::Shutdown();
    // 1 - FEffect
    FEffect::Shutdown();
    // 0 - Pool allocator tag
    POOL_TAG(Engine)::Shutdown();
}
//----------------------------------------------------------------------------
void FEngineStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Engine)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
