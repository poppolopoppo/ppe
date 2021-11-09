#pragma once

#include "RHIVulkan_fwd.h"
#include "HAL/VulkanTargetRHI.h"

#include "Misc/Event.h"
#include "Modular/ModuleInterface.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FRHIVulkanModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    FRHIVulkanModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    static FRHIVulkanModule& Get(const FModularDomain& domain);

    const FVulkanTargetRHI& TargetRHI() const { return (*_targetRHI); }

    // --- FVulkanDevice ---

    using FVulkanDeviceEvent = TFunction<void(const RHI::FVulkanDeviceInfo&)>;

    THREADSAFE_EVENT(OnDeviceCreated, FVulkanDeviceEvent);
    THREADSAFE_EVENT(OnDeviceTearDown, FVulkanDeviceEvent);

    void DeviceCreated(RHI::FVulkanDeviceInfo& device);
    void DeviceTearDown(RHI::FVulkanDeviceInfo& device);

private:
    TUniquePtr<class FVulkanTargetRHI> _targetRHI;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
