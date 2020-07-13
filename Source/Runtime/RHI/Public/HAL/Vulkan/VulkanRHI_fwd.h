#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIIncludes_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using EVulkanColorSpace = EGenericColorSpace;
using EVulkanPixelFormat = EGenericPixelFormat;
using EVulkanVertexFormat = EGenericVertexFormat;
struct FVulkanPixelInfo;
struct FVulkanSurfaceFormat;
//----------------------------------------------------------------------------
struct FVulkanInstance;
using FVulkanPhysicalDevice = VkPhysicalDevice;
using EVulkanPhysicalDeviceFlags = EGenericPhysicalDeviceFlags;
using FVulkanWindowHandle = FGenericWindowHandle;
using FVulkanWindowSurface = VkSurfaceKHR;
using FVulkanAllocationCallbacks = const VkAllocationCallbacks*;
//----------------------------------------------------------------------------
using EVulkanMemoryTypeFlags = EGenericMemoryTypeFlags;
using FVulkanDeviceMemory = VkDeviceMemory;
struct FVulkanMemoryBlock;
class FVulkanMemoryAllocator;
//----------------------------------------------------------------------------
class FVulkanDevice;
using EVulkanPresentMode = EGenericPresentMode;
//----------------------------------------------------------------------------
class FVulkanSwapChain;
//----------------------------------------------------------------------------
using EVulkanBlendFactor = EGenericBlendFactor;
using EVulkanBlendOp = EGenericBlendOp;
using EVulkanColorComponentMask = EGenericColorComponentMask;
using EVulkanLogicOp = EGenericLogicOp;
using FVulkanBlendAttachmentState = FGenericBlendAttachmentState;
using FVulkanBlendState = FGenericBlendState;
//----------------------------------------------------------------------------
using EVulkanCompareOp = EGenericCompareOp;
using EVulkanStencilOp = EGenericStencilOp;
using FVulkanStencilOpState = FGenericStencilOpState;
using FVulkanDepthStencilState = FGenericDepthStencilState;
using FVulkanMultisampleState = FGenericMultisampleState;
//----------------------------------------------------------------------------
using EVulkanCullMode = EGenericCullMode;
using EVulkanFrontFace = EGenericFrontFace;
using EVulkanPolygonMode = EGenericPolygonMode;
using EVulkanConservativeRasterizationMode = EGenericConservativeRasterizationMode;
using FVulkanRasterizerState = FGenericRasterizerState;
//----------------------------------------------------------------------------
using EVulkanDynamicState = EGenericDynamicState;
struct FVulkanFixedFunctionState;
//----------------------------------------------------------------------------
using EVulkanPrimitiveTopology = EGenericPrimitiveTopology;
using EVulkanVertexInputRate = EGenericVertexInputRate;
using FVulkanVertexBinding = FGenericVertexBinding;
using FVulkanVertexAttribute = FGenericVertexAttribute;
struct FVulkanInputAssembly;
//----------------------------------------------------------------------------
using EVulkanShaderStageFlags = EGenericShaderStageFlags;
using EVulkanShaderStageCreateFlags = EGenericShaderStageCreateFlags;
using FVulkanShaderSpecialization = FGenericShaderSpecialization;
struct FVulkanShaderStage;
//----------------------------------------------------------------------------
using EVulkanDescriptorFlags = EGenericDescriptorFlags;
using EVulkanDescriptorType = EGenericDescriptorType;
using EVulkanDescriptorSetFlags = EGenericDescriptorSetFlags;
using FVulkanPushConstantRange = FGenericPushConstantRange;
using FVulkanDescriptorBinding = FGenericDescriptorBinding;
struct FVulkanDescriptorSetLayout;
struct FVulkanPipelineLayout;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
