#pragma once

#include "HAL/TargetRHI.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanTargetRHI final : public ITargetRHI {
public:
    static const FVulkanTargetRHI& Get();

    virtual ETargetRHI RHI() const override {
        return ETargetRHI::Vulkan;
    }

    virtual FString DisplayName() const override;
    virtual FString FullName() const override;
    virtual FString ShortName() const override;

    virtual bool RequiresFeature(ERHIFeature feature) const override;
    virtual bool SupportsFeature(ERHIFeature feature) const override;

private:
    FVulkanTargetRHI() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
