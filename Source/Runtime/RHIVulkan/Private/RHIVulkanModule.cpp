#include "stdafx.h"

#include "RHIVulkanModule.h"

#include "HAL/VulkanTargetRHI.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "BuildModules.generated.h"
#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRHIVulkanModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRHIVulkanModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::System,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRHIVulkanModule& FRHIVulkanModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FRHIVulkanModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FRHIVulkanModule::FRHIVulkanModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRHIVulkanModule::Start(FModularDomain& domain) {
    Assert_NoAssume(not _targetRHI.valid());

    IModuleInterface::Start(domain);

    using namespace RHI;
    FRHIModule& rhi = FRHIModule::Get(domain);

    _targetRHI = MakeUnique<FVulkanTargetRHI>();

    rhi.RegisterTarget(ETargetRHI::Vulkan, _targetRHI.get());
}
//----------------------------------------------------------------------------
void FRHIVulkanModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    FRHIModule& rhi = FRHIModule::Get(domain);

    rhi.UnregisterTarget(ETargetRHI::Vulkan);
}
//----------------------------------------------------------------------------
void FRHIVulkanModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRHIVulkanModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
void FRHIVulkanModule::DeviceCreated(RHI::FVulkanDeviceInfo& device) {
    Assert(device.vkInstance);
    Assert(device.vkPhysicalDevice);
    Assert(device.vkDevice);

    _OnDeviceCreated.Invoke(device);
}
//----------------------------------------------------------------------------
void FRHIVulkanModule::DeviceTearDown(RHI::FVulkanDeviceInfo& device) {
    Assert(device.vkInstance);
    Assert(device.vkPhysicalDevice);
    Assert(device.vkDevice);

    _OnDeviceTearDown.Invoke(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
