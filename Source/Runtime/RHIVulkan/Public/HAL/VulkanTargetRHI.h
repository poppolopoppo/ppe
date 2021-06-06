#pragma once

#include "RHIVulkan_fwd.h"

#include "HAL/TargetRHI.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanTargetRHI final : public ITargetRHI {
public:
    FVulkanTargetRHI() = default;

    virtual ETargetRHI RHI() const NOEXCEPT override {
        return ETargetRHI::Vulkan;
    }

    virtual FString DisplayName() const NOEXCEPT override;
    virtual FString FullName() const NOEXCEPT override;
    virtual FString ShortName() const NOEXCEPT override;

    virtual ERHIFeature RecommendedFeatures() const NOEXCEPT override;
    virtual bool RequiresFeature(ERHIFeature feature) const NOEXCEPT override;
    virtual bool SupportsFeature(ERHIFeature feature) const NOEXCEPT override;

    virtual bool CreateService(
        URHIService* pRHIService,
        const FModularDomain& domain,
        const FRHISurfaceCreateInfo* pOptionalWindow,
        ERHIFeature features,
        FStringView deviceName ) const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
