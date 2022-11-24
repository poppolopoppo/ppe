// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Engine.h"

#include "Effect/Effect.h"
#include "Material/MaterialConstNames.h"
#include "Mesh/MeshName.h"
#include "World/World.h"

#include "Core/Allocator/PoolAllocator-impl.h"

PRAGMA_INITSEG_LIB

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
