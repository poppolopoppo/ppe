#pragma once

#include "RHIVulkan_fwd.h"
#include "HAL/VulkanTargetRHI.h"

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

private:
    TUniquePtr<class FVulkanTargetRHI> _targetRHI;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
