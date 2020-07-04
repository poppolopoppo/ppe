#include "stdafx.h"

#include "HAL/Vulkan/VulkanTargetRHI.h"

#include "IO/String.h"

namespace PPE {
namespace RHI {
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
    case ERHIFeature::HighEndGraphics: return true;
    case ERHIFeature::Compute: return true;
    case ERHIFeature::AsyncCompute: return true;
    case ERHIFeature::Raytracing: return true;
    case ERHIFeature::Meshlet: return true;
    case ERHIFeature::SamplerFeedback: return false;
    case ERHIFeature::TextureSpaceShading: return false;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FVulkanTargetRHI::SupportsFeature(ERHIFeature feature) const {
    switch (feature) {
    case ERHIFeature::HighEndGraphics: return true;
    case ERHIFeature::Compute: return true;
    case ERHIFeature::AsyncCompute: return true;
    case ERHIFeature::Raytracing: return true;
    case ERHIFeature::Meshlet: return true;
    case ERHIFeature::SamplerFeedback: return false;
    case ERHIFeature::TextureSpaceShading: return false;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
