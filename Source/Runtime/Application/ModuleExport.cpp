#include "stdafx.h"

#include "ModuleExport.h"

#include "ApplicationBase.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "HAL/PlatformApplicationMisc.h"

#include "Module-impl.h"

namespace PPE {
namespace Application {
POOL_TAG_DEF(Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationModule::FApplicationModule()
:   FModule("Runtime/Application")
{}
//----------------------------------------------------------------------------
FApplicationModule::~FApplicationModule()
{}
//----------------------------------------------------------------------------
void FApplicationModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    POOL_TAG(Application)::Start();

    FPlatformApplicationMisc::Start();
}
//----------------------------------------------------------------------------
void FApplicationModule::Shutdown() {
    FModule::Shutdown();

    FPlatformApplicationMisc::Shutdown();

    POOL_TAG(Application)::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    POOL_TAG(Application)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
