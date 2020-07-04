#include "stdafx.h"

#include "HAL/Vulkan/VulkanRHISwapChain.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanError.h"
#include "HAL/Vulkan/VulkanInterop.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIDevice.h"
#include "HAL/Vulkan/VulkanRHIInstance.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanSwapChain::FVulkanSwapChain(
    FVulkanSwapChainHandle swapChainHandle,
    u322 extent,
    const FVulkanSurfaceFormat& surfaceFormat ) NOEXCEPT
:   _handle(swapChainHandle)
,   _extent(extent)
,   _surfaceFormat(surfaceFormat) {
    Assert_NoAssume(_handle);
    Assert_NoAssume(_extent.x > 0 && _extent.y > 0);
}
//----------------------------------------------------------------------------
FVulkanSwapChain::~FVulkanSwapChain() {
    Assert_NoAssume(_images.empty());
    Assert_NoAssume(_imageViews.empty());
}
//----------------------------------------------------------------------------
void FVulkanSwapChain::InitializeSwapChain(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE != device.LogicalDevice());
    Assert_NoAssume(_images.empty());
    Assert_NoAssume(_imageViews.empty());

    // retrieve all images
    u32 numImagesInSwapChain;
    PPE_VKDEVICE_CHECKED(vkGetSwapchainImagesKHR, device.LogicalDevice(), _handle, &numImagesInSwapChain, nullptr);

    _images.resize_Uninitialized(numImagesInSwapChain);
    PPE_VKDEVICE_CHECKED(vkGetSwapchainImagesKHR, device.LogicalDevice(), _handle, &numImagesInSwapChain, _images.data());

    // create image views
    _imageViews.resize_Uninitialized(numImagesInSwapChain);
    forrange(i, 0, numImagesInSwapChain) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = FVulkanInterop::Vk(_surfaceFormat.Format);
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer  = 0;
        createInfo.subresourceRange.layerCount = 1;

        PPE_VKDEVICE_CHECKED(vkCreateImageView,
            device.LogicalDevice(),
            &createInfo,
            FVulkanInstance::Allocator(),
            &_imageViews[i] );
    }
}
//----------------------------------------------------------------------------
void FVulkanSwapChain::TearDownSwapChain(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE != device.LogicalDevice());

    for (VkImageView imageView : _imageViews)
        vkDestroyImageView(device.LogicalDevice(), imageView, FVulkanInstance::Allocator());

    _images.clear();
    _imageViews.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
