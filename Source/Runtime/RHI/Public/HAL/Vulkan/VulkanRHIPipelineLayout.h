#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIPipelineLayout.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanPushConstantRange {
    EVulkanShaderStageFlags StageFlags{ 0 };
    u32 Offset{ 0 };
    u32 Size{ 0 };
};
//----------------------------------------------------------------------------
enum class EVulkanDescriptorFlags : u32 {
    None = 0,
    UpdateAfterBind = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
    UpdateUnusedWhilePending = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT,
    PartiallyBound = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
    VariableDescriptorCount = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
};
ENUM_FLAGS(EVulkanDescriptorFlags);
//----------------------------------------------------------------------------
enum class EVulkanDescriptorType : u32 {
    Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
    CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    InlineUniformBlock = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
    AccelerationStructure = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
};
//----------------------------------------------------------------------------
struct FVulkanDescriptorBinding {
    u32 BindingIndex;
    u32 NumDescriptors;
    EVulkanDescriptorFlags BindingFlags;
    EVulkanDescriptorType DescriptorType;
    EVulkanShaderStageFlags StageFlags;
};
//----------------------------------------------------------------------------
enum class EVulkanDescriptorSetFlags : u32 {
    None = 0,
    UpdateAfterBindPool = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
    PushDescriptor = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
};
ENUM_FLAGS(EVulkanDescriptorSetFlags);
//----------------------------------------------------------------------------
struct FVulkanDescriptorSetLayout {
    EVulkanDescriptorSetFlags SetFlags;
    VECTORINSITU(RHIState, FVulkanDescriptorBinding, 1) Bindings;
};
//----------------------------------------------------------------------------
struct FVulkanPipelineLayout {
    VECTORINSITU(RHIState, FVulkanDescriptorSetLayoutHandle, 2) SetLayouts;
    VECTORINSITU(RHIState, FVulkanPushConstantRange, 2) PushConstantRanges;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
