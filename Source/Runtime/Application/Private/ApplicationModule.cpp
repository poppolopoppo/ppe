#include "stdafx.h"

#include "ApplicationModule.h"

#include "HAL/PlatformApplicationMisc.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Application {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_APPLICATION_API, Application)
//----------------------------------------------------------------------------
} //!namespace Application
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FApplicationModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FApplicationModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FApplicationModule& FApplicationModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FApplicationModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FApplicationModule::FApplicationModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FApplicationModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace Application;

    FPlatformApplicationMisc::Start();
}
//----------------------------------------------------------------------------
void FApplicationModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace Application;

    FPlatformApplicationMisc::Shutdown();
}
//----------------------------------------------------------------------------
void FApplicationModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FApplicationModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
