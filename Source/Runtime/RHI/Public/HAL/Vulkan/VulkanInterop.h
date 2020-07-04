#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIIncludes_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanInterop {
public:
    static VkColorSpaceKHR Vk(EVulkanColorSpace value) NOEXCEPT;
    static EVulkanColorSpace RHI(VkColorSpaceKHR value) NOEXCEPT;

    static VkFormat Vk(EVulkanPixelFormat value) NOEXCEPT;
    static EVulkanPixelFormat RHI(VkFormat value) NOEXCEPT;

    static VkPresentModeKHR Vk(EVulkanPresentMode value) NOEXCEPT;
    static EVulkanPresentMode RHI(VkPresentModeKHR value) NOEXCEPT;

public:
    static VkBlendFactor Vk(EVulkanBlendFactor value) NOEXCEPT;
    static EVulkanBlendFactor RHI(VkBlendFactor value) NOEXCEPT;

    static VkBlendOp Vk(EVulkanBlendOp value) NOEXCEPT;
    static EVulkanBlendOp RHI(VkBlendOp value) NOEXCEPT;

    static VkColorComponentFlagBits Vk(EVulkanColorComponentMask value) NOEXCEPT;
    static EVulkanColorComponentMask RHI(VkColorComponentFlagBits value) NOEXCEPT;

    static VkLogicOp Vk(EVulkanLogicOp value) NOEXCEPT;
    static EVulkanLogicOp RHI(VkLogicOp value) NOEXCEPT;

    static VkCompareOp Vk(EVulkanCompareOp value) NOEXCEPT;
    static EVulkanCompareOp RHI(VkCompareOp value) NOEXCEPT;

    static VkStencilOp Vk(EVulkanStencilOp value) NOEXCEPT;
    static EVulkanStencilOp RHI(VkStencilOp value) NOEXCEPT;

    static VkCullModeFlagBits Vk(EVulkanCullMode value) NOEXCEPT;
    static EVulkanCullMode RHI(VkCullModeFlagBits value) NOEXCEPT;

    static VkFrontFace Vk(EVulkanFrontFace value) NOEXCEPT;
    static EVulkanFrontFace RHI(VkFrontFace value) NOEXCEPT;

    static VkPolygonMode Vk(EVulkanPolygonMode value) NOEXCEPT;
    static EVulkanPolygonMode RHI(VkPolygonMode value) NOEXCEPT;

    static VkConservativeRasterizationModeEXT Vk(EVulkanConservativeRasterizationMode value) NOEXCEPT;
    static EVulkanConservativeRasterizationMode RHI(VkConservativeRasterizationModeEXT value) NOEXCEPT;

    static VkDynamicState Vk(EVulkanDynamicState value) NOEXCEPT;
    static EVulkanDynamicState RHI(VkDynamicState value) NOEXCEPT;

    static VkPrimitiveTopology Vk(EVulkanPrimitiveTopology value) NOEXCEPT;
    static EVulkanPrimitiveTopology RHI(VkPrimitiveTopology value) NOEXCEPT;

    static VkVertexInputRate Vk(EVulkanVertexInputRate value) NOEXCEPT;
    static EVulkanVertexInputRate RHI(VkVertexInputRate value) NOEXCEPT;

public:
    static VkMemoryPropertyFlagBits Vk(EVulkanMemoryTypeFlags value) NOEXCEPT;
    static EVulkanMemoryTypeFlags RHI(VkMemoryPropertyFlagBits value) NOEXCEPT;

public:
    static VkShaderStageFlagBits Vk(EVulkanShaderStageFlags value) NOEXCEPT;
    static EVulkanShaderStageFlags RHI(VkShaderStageFlagBits value) NOEXCEPT;

    static VkPipelineShaderStageCreateFlagBits Vk(EVulkanShaderStageCreateFlags value) NOEXCEPT;
    static EVulkanShaderStageCreateFlags RHI(VkPipelineShaderStageCreateFlagBits value) NOEXCEPT;

public:
    static VkDescriptorBindingFlagBits Vk(EVulkanDescriptorFlags value) NOEXCEPT;
    static EVulkanDescriptorFlags RHI(VkDescriptorBindingFlagBits value) NOEXCEPT;

    static VkDescriptorType Vk(EVulkanDescriptorType value) NOEXCEPT;
    static EVulkanDescriptorType RHI(VkDescriptorType value) NOEXCEPT;

    static VkDescriptorSetLayoutCreateFlagBits Vk(EVulkanDescriptorSetFlags value) NOEXCEPT;
    static EVulkanDescriptorSetFlags RHI(VkDescriptorSetLayoutCreateFlagBits value) NOEXCEPT;

};
//----------------------------------------------------------------------------
/////////////////////////////////////////////////////
////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
