#include "stdafx.h"

#include "ModuleExport.h"

#include "RTTI.h"

#include "RTTI/Namespace.h"
#include "RTTI/Namespace-impl.h"

#include "MetaDatabase.h"

#include "Allocator/PoolAllocatorTag-impl.h"

#include "Module-impl.h"

namespace PPE {
namespace RTTI {
POOL_TAG_DEF(RTTI);
RTTI_NAMESPACE_DEF(PPE_RTTI_API, RTTI, MetaObject);
#if USE_PPE_RTTI_CHECKS
extern void RTTI_UnitTests();
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRTTIModule::FRTTIModule()
:   FModule("Runtime/RTTI")
{}
//----------------------------------------------------------------------------
FRTTIModule::~FRTTIModule()
{}
//----------------------------------------------------------------------------
void FRTTIModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    POOL_TAG(RTTI)::Start();

    FName::Start();

    FMetaDatabase::Create();

    RTTI_NAMESPACE(RTTI).Start();

#if USE_PPE_RTTI_CHECKS
    RTTI_UnitTests();
#endif
}
//----------------------------------------------------------------------------
void FRTTIModule::Shutdown() {
    FModule::Shutdown();

    RTTI_NAMESPACE(RTTI).Shutdown();

    FMetaDatabase::Destroy();

    FName::Shutdown();

    POOL_TAG(RTTI)::Shutdown();
}
//----------------------------------------------------------------------------
void FRTTIModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    POOL_TAG(RTTI)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
