#include "stdafx.h"

#include "Logic.h"

#include "Entity/Entity.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core.RTTI/RTTI_Tag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Logic {
POOL_TAG_DEF(Logic);
RTTI_TAG_DEF(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LogicStartup::Start() {
    POOL_TAG(Logic)::Start();
    RTTI_TAG(Logic)::Start();
    Entity::Start();
}
//----------------------------------------------------------------------------
void LogicStartup::Shutdown() {
    Entity::Shutdown();
    RTTI_TAG(Logic)::Shutdown();
    POOL_TAG(Logic)::Shutdown();
}
//----------------------------------------------------------------------------
void LogicStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Logic)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
