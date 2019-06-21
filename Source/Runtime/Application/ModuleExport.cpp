#include "stdafx.h"

#include "ModuleExport.h"

#include "ApplicationBase.h"

#include "HAL/PlatformApplicationMisc.h"

#include "Module-impl.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FApplicationModule::FApplicationModule()
:   FModule("Runtime/Application")
{}
//----------------------------------------------------------------------------
FApplicationModule::~FApplicationModule() = default;
//----------------------------------------------------------------------------
void FApplicationModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    FPlatformApplicationMisc::Start();
}
//----------------------------------------------------------------------------
void FApplicationModule::Shutdown() {
    FModule::Shutdown();

    FPlatformApplicationMisc::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationModule::ReleaseMemory() {
    FModule::ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
