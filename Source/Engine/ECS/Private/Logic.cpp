// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Logic.h"

#include "Entity/Entity.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "RTTI_Tag-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Logic {
POOL_TAG_DEF(Logic);
RTTI_TAG_DEF(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLogicModule::Start() {
    PPE_MODULE_START(Logic);

    POOL_TAG(Logic)::Start();
    RTTI_TAG(Logic)::Start();
    FEntity::Start();
}
//----------------------------------------------------------------------------
void FLogicModule::Shutdown() {
    PPE_MODULE_SHUTDOWN(Logic);

    FEntity::Shutdown();
    RTTI_TAG(Logic)::Shutdown();
    POOL_TAG(Logic)::Shutdown();
}
//----------------------------------------------------------------------------
void FLogicModule::ClearAll_UnusedMemory() {
    PPE_MODULE_CLEARALL(Logic);

    POOL_TAG(Logic)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
