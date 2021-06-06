#include "stdafx.h"

#include "HAL/VulkanTargetRHI.h"

#include "HAL/VulkanRHIService.h"

#include "IO/String.h"
#include "Modular/ModularDomain.h"

namespace PPE {
namespace RHI {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::DisplayName() const NOEXCEPT {
    return "Vulkan";
}
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::FullName() const NOEXCEPT {
    return "VulkanSDK v" STRINGIZE(VK_HEADER_VERSION_COMPLETE);
}
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::ShortName() const NOEXCEPT {
    return "Vk";
}
//----------------------------------------------------------------------------
ERHIFeature FVulkanTargetRHI::RecommendedFeatures() const NOEXCEPT {
    return ERHIFeature::Recommended;
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::RequiresFeature(ERHIFeature feature) const NOEXCEPT {
    switch (feature) {
    case ERHIFeature::Graphics:
    case ERHIFeature::Compute:
    case ERHIFeature::AsyncCompute:
        return true;
    case ERHIFeature::Headless:
    case ERHIFeature::Discrete:
    case ERHIFeature::Raytracing:
    case ERHIFeature::MeshDraw:
    case ERHIFeature::SamplerFeedback:
    case ERHIFeature::TextureSpaceShading:
    case ERHIFeature::VariableShadingRate:
    case ERHIFeature::ConservativeDepth:
    case ERHIFeature::HighDynamicRange:
    case ERHIFeature::VSync:
        return false;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::SupportsFeature(ERHIFeature feature) const NOEXCEPT {
    switch (feature) {
    case ERHIFeature::Headless:
    case ERHIFeature::Discrete:
    case ERHIFeature::Graphics:
    case ERHIFeature::Compute:
    case ERHIFeature::AsyncCompute:
    case ERHIFeature::Raytracing:
    case ERHIFeature::MeshDraw:
    case ERHIFeature::SamplerFeedback:
    case ERHIFeature::TextureSpaceShading:
    case ERHIFeature::VariableShadingRate:
    case ERHIFeature::ConservativeDepth:
    case ERHIFeature::HighDynamicRange:
    case ERHIFeature::VSync:
        return true;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::CreateService(
    URHIService* pRHIService,
    const FModularDomain& domain,
    const FRHISurfaceCreateInfo* pOptionalWindow,
    ERHIFeature features,
    FStringView deviceName ) const {
    Assert(pRHIService);

    if (pRHIService
            ->reset<FVulkanRHIService>(*this, features)
            ->Construct(domain.Name(), pOptionalWindow, deviceName) )
        return true;

    pRHIService->reset();
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
