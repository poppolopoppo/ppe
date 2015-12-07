#include "stdafx.h"

#include "Logic.h"

#include "Entity/Entity.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

namespace Core {
namespace Logic {
POOLTAG_DEF(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LogicStartup::Start() {
    POOLTAG(Logic)::Start();
    Entity::Start();
}
//----------------------------------------------------------------------------
void LogicStartup::Shutdown() {
    Entity::Shutdown();
    POOLTAG(Logic)::Shutdown();
}
//----------------------------------------------------------------------------
void LogicStartup::ClearAll_UnusedMemory() {
    POOLTAG(Logic)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
