#pragma once

#include "RHIVulkan_fwd.h"

#include "HAL/TargetRHI_fwd.h"

#include "Vulkan/Vulkan_fwd.h"

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

    bool VulkanAvailable() const;
    RHI::FVulkanInstance& VulkanInstance() const;

    bool CreatePipelineCompiler(RHI::PPipelineCompiler* pcompiler) const;
    bool CreateFrameGraph(RHI::PFrameGraph* pfg, ERHIFeature features, RHI::FWindowHandle windowSurface) const;

private:
    TUniquePtr<RHI::FVulkanInstance> _instance;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
