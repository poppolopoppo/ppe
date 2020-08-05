#include "stdafx.h"

#include "RHIModule.h"

#include "Modular/ModuleRegistration.h"
#include "Diagnostic/Logger.h"

#include "HAL/RHIDevice.h"
#include "HAL/RHIInstance.h"

#include "BuildModules.generated.h"

namespace PPE {
namespace RHI {
LOG_CATEGORY(PPE_RHI_API, RHI)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRHIModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRHIModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRHIModule::FRHIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRHIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    RHI::FInstance::ParseOptions();
    RHI::FInstance::Start();
}
//----------------------------------------------------------------------------
void FRHIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    RHI::FInstance::Shutdown();
}
//----------------------------------------------------------------------------
void FRHIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRHIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
