#include "stdafx.h"

#include "RHIModule.h"

#include "Modular/ModuleRegistration.h"
#include "Diagnostic/Logger.h"

#include "HAL/RHIDevice.h"
#include "HAL/RHIInstance.h"

#include "BuildModules.generated.h"

namespace PPE {
LOG_CATEGORY(PPE_RHI_API, RHI)
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
        STRINGIZE(BUILD_TARGET_DEPS) )
};

FRHIDevice* GRHIDevice = nullptr; // #TODO: remove this after initial support for Vulkan
//----------------------------------------------------------------------------
FRHIModule::FRHIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRHIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    FRHIInstance::Start();

    auto deviceFlags = FRHIInstance::EPhysicalDeviceFlags::Default;
    FRHIInstance::FPhysicalDevice physicalDevice = FRHIInstance::PickPhysicalDevice(deviceFlags);
    GRHIDevice = FRHIInstance::CreateLogicalDevice(physicalDevice, deviceFlags);
}
//----------------------------------------------------------------------------
void FRHIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    FRHIInstance::DestroyLogicalDevice(&GRHIDevice);

    FRHIInstance::Shutdown();
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
