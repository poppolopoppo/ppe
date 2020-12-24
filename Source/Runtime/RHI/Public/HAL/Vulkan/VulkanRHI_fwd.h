#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIIncludes_fwd.h"

namespace PPE {
namespace RHI {

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// VulkanRHIDevice.h
//----------------------------------------------------------------------------
enum class EVulkanPresentMode : u32;
using FVulkanShaderModule = VkShaderModule;
class FVulkanDevice;
//----------------------------------------------------------------------------
// VulkanRHIFixedFunctionState.h
//----------------------------------------------------------------------------
enum class EVulkanBlendFactor : u32;
enum class EVulkanBlendOp : u32;
enum class EVulkanColorComponentMask : u32;
enum class EVulkanLogicOp : u32;
struct FVulkanBlendAttachmentState;
struct FVulkanBlendState;
//----------------------------------------------------------------------------
enum class EVulkanCompareOp : u32;
enum class EVulkanStencilOp : u32;
struct FVulkanStencilOpState;
struct FVulkanDepthStencilState;
//----------------------------------------------------------------------------
struct FVulkanMultisampleState;
//----------------------------------------------------------------------------
enum class EVulkanCullMode : u32;
enum class EVulkanFrontFace : u32;
enum class EVulkanPolygonMode : u32;
enum class EVulkanConservativeRasterizationMode : u32;
struct FVulkanRasterizerState;
//----------------------------------------------------------------------------
enum class EVulkanDynamicState : u32;
struct FVulkanFixedFunctionState;
//----------------------------------------------------------------------------
// VulkanRHIFormat.h
//----------------------------------------------------------------------------
enum class EVulkanColorSpace : u32;
enum class EVulkanFormat : u32;
using EVulkanPixelFormat = EVulkanFormat;
using EVulkanVertexFormat = EVulkanFormat;
//----------------------------------------------------------------------------
// VulkanRHIInstance.h
//----------------------------------------------------------------------------
using FVulkanWindowHandle = FGenericWindowHandle;
using FVulkanWindowSurface = VkSurfaceKHR;
class FVulkanInstance;
//----------------------------------------------------------------------------
using EVulkanPhysicalDeviceFlags = EGenericPhysicalDeviceFlags;
class FVulkanInstance;
//----------------------------------------------------------------------------
// VulkanRHIInputAssembly.h
//----------------------------------------------------------------------------
enum class EVulkanPrimitiveTopology : u32;
enum class EVulkanVertexInputRate : u32;
struct FVulkanVertexBinding;
struct FVulkanVertexAttribute;
struct FVulkanInputAssembly;
//----------------------------------------------------------------------------
// VulkanRHIMemoryAllocator.h
//----------------------------------------------------------------------------
using FVulkanDeviceMemory = VkDeviceMemory;
//----------------------------------------------------------------------------
enum class EVulkanMemoryTypeFlags : u32;
struct FVulkanMemoryBlock;
class FVulkanMemoryAllocator;
//----------------------------------------------------------------------------
// VulkanRHIPipelineLayout.h
//----------------------------------------------------------------------------
using FVulkanDescriptorSetLayoutHandle = VkDescriptorSetLayout;
using FVulkanPipelineLayoutHandle = VkPipelineLayout;
//----------------------------------------------------------------------------
struct FVulkanPushConstantRange;
enum class EVulkanDescriptorFlags : u32;
enum class EVulkanDescriptorType : u32;
struct FVulkanDescriptorBinding;
enum class EVulkanDescriptorSetFlags : u32;
struct FVulkanDescriptorSetLayout;
struct FVulkanPipelineLayout;
//----------------------------------------------------------------------------
// VulkanRHIShaderStage.h
//----------------------------------------------------------------------------
enum class EVulkanShaderStageFlags : u32;
enum class EVulkanShaderStageCreateFlags : u32;
using FVulkanShaderSpecialization = FGenericShaderSpecialization;
struct FVulkanShaderStage;
//----------------------------------------------------------------------------
// VulkanRHISurfaceFormat.h
//----------------------------------------------------------------------------
struct FVulkanPixelInfo;
struct FVulkanSurfaceFormat;
//----------------------------------------------------------------------------
// VulkanRHISwapChain.h
//----------------------------------------------------------------------------
class FVulkanSwapChain;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
