#include "stdafx.h"

#include "HAL/VulkanTargetRHI.h"

#include "Vulkan/Common/VulkanAPI.h"

#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FVulkanTargetRHI& FVulkanTargetRHI::Get() {
    ONE_TIME_DEFAULT_INITIALIZE(const FVulkanTargetRHI, GTargetRHI);
    return GTargetRHI;
}
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::DisplayName() const {
    return "Vulkan";
}
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::FullName() const {
    return "VulkanSDK v" STRINGIZE(VK_HEADER_VERSION_COMPLETE);
}
//----------------------------------------------------------------------------
FString FVulkanTargetRHI::ShortName() const {
    return "Vk";
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::RequiresFeature(ERHIFeature feature) const {
    switch (feature) {
    case ERHIFeature::Graphics:
    case ERHIFeature::Compute:
    case ERHIFeature::AsyncCompute:
        return true;
    case ERHIFeature::Headless:
    case ERHIFeature::Discrete:
    case ERHIFeature::Raytracing:
    case ERHIFeature::Meshlet:
    case ERHIFeature::SamplerFeedback:
    case ERHIFeature::TextureSpaceShading:
    case ERHIFeature::VariableShadingRate:
    case ERHIFeature::ConservativeDepth:
        return false;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::SupportsFeature(ERHIFeature feature) const {
    switch (feature) {
    case ERHIFeature::Headless:
    case ERHIFeature::Discrete:
    case ERHIFeature::Graphics:
    case ERHIFeature::Compute:
    case ERHIFeature::AsyncCompute:
    case ERHIFeature::Raytracing:
    case ERHIFeature::Meshlet:
    case ERHIFeature::SamplerFeedback:
    case ERHIFeature::TextureSpaceShading:
    case ERHIFeature::VariableShadingRate:
    case ERHIFeature::ConservativeDepth:
        return true;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
