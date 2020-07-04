#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIPipelineLayout.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanDescriptorSetLayout : FGenericDescriptorSetLayout {

};
//----------------------------------------------------------------------------
struct FVulkanPipelineLayout /*: FGenericPipelineLayout*/ {
    VECTORINSITU(RHIState, FVulkanDescriptorSetLayoutHandle, 2) SetLayouts;
    VECTORINSITU(RHIState, FVulkanPushConstantRange, 2) PushConstantRanges;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
