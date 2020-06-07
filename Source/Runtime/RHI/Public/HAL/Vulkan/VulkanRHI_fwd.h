#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#ifdef RHI_VULKAN

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Vulkan
//----------------------------------------------------------------------------
#define PPE_VK_HANDLE(_NAME) \
    struct CONCAT(_NAME, _T); \
    namespace PPE { namespace RHI { \
        using _NAME = ::CONCAT(_NAME, _T)*; \
    }}
#ifdef ARCH_64BIT
#    define PPE_VK_HANDLE_NON_DISPATCHABLE(_NAME) PPE_VK_HANDLE(_NAME)
#else
#    define PPE_VK_HANDLE_NON_DISPATCHABLE(_NAME) namespace PPE { namespace RHI { \
    using _NAME = u64; \
}}
#endif
//----------------------------------------------------------------------------
namespace PPE {
namespace RHI {
using VkFlags = u32;
using VkBool32 = u32;
using VkDeviceSize = u64;
using VkSampleMask =  u32;
}}
struct VkAllocationCallbacks;
PPE_VK_HANDLE(VkInstance)
PPE_VK_HANDLE(VkPhysicalDevice)
PPE_VK_HANDLE(VkDevice)
PPE_VK_HANDLE(VkQueue)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSemaphore)
PPE_VK_HANDLE(VkCommandBuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkFence)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDeviceMemory)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkBuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkImage)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkEvent)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkQueryPool)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkBufferView)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkImageView)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkShaderModule)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipelineCache)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipelineLayout)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkRenderPass)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipeline)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorSetLayout)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSampler)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorPool)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorSet)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkFramebuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkCommandPool)
//----------------------------------------------------------------------------
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSwapchainKHR)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSurfaceKHR)
//----------------------------------------------------------------------------
#undef PPE_VK_HANDLE
#undef PPE_VK_HANDLE_NON_DISPATCHABLE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using EVulkanPixelFormat = EGenericPixelFormat;
using EVulkanColorSpace = EGenericColorSpace;
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
class FVulkanDevice;
using FVulkanDeviceHandle = VkDevice;
using FVulkanQueueHandle = VkQueue;
using EVulkanPresentMode = EGenericPresentMode;
//----------------------------------------------------------------------------
using FVulkanSwapChainHandle = VkSwapchainKHR;
class FVulkanSwapChain;
//----------------------------------------------------------------------------
using FVulkanImageHandle = VkImage;
using FVulkanImageViewHandle = VkImageView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
