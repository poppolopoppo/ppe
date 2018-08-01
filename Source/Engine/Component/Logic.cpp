#include "stdafx.h"

#include "Logic.h"

#include "Entity/Entity.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core.RTTI/RTTI_Tag-impl.h"

PRAGMA_INITSEG_LIB

namespace Core {
namespace Logic {
POOL_TAG_DEF(Logic);
RTTI_TAG_DEF(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogicModule::Start() {
    CORE_MODULE_START(Logic);

    POOL_TAG(Logic)::Start();
    RTTI_TAG(Logic)::Start();
    FEntity::Start();
}
//----------------------------------------------------------------------------
void FLogicModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Logic);

    FEntity::Shutdown();
    RTTI_TAG(Logic)::Shutdown();
    POOL_TAG(Logic)::Shutdown();
}
//----------------------------------------------------------------------------
void FLogicModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Logic);

    POOL_TAG(Logic)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
