﻿#include "stdafx.h"

#include "Vulkan/VulkanSwapChain.h"

#ifdef RHI_VULKAN

#include "Vulkan/VulkanError.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanIncludes.h"
#include "Vulkan/VulkanInstance.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanSwapchain::FVulkanSwapchain(
    VkSwapchainKHR vkSwapChain,
    u322 extent,
    const VkSurfaceFormatKHR& surfaceFormat ) NOEXCEPT
:   _vkSwapChain(vkSwapChain)
,   _extent(extent)
,   _surfaceFormat(surfaceFormat) {
    Assert_NoAssume(VK_NULL_HANDLE != _vkSwapChain);
    Assert_NoAssume(_extent.x > 0 && _extent.y > 0);
}
//----------------------------------------------------------------------------
FVulkanSwapchain::~FVulkanSwapchain() {
    Assert_NoAssume(_images.empty());
    Assert_NoAssume(_imageViews.empty());
}
//----------------------------------------------------------------------------
void FVulkanSwapchain::InitializeSwapchain(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE != device.vkDevice());
    Assert_NoAssume(_images.empty());
    Assert_NoAssume(_imageViews.empty());

    // retrieve all images
    u32 numImagesInSwapChain;
    PPE_VKDEVICE_CHECKED(device.vkGetSwapchainImagesKHR, device.vkDevice(), _vkSwapChain, &numImagesInSwapChain, nullptr);

    _images.resize_Uninitialized(numImagesInSwapChain);
    PPE_VKDEVICE_CHECKED(device.vkGetSwapchainImagesKHR, device.vkDevice(), _vkSwapChain, &numImagesInSwapChain, &_images.front());

    // create image views
    _imageViews.resize_Uninitialized(numImagesInSwapChain);
    forrange(i, 0, numImagesInSwapChain) {
        PPE_VKDEVICE_SETDEBUGARGS(device, VkImage, _images[i], "SwapChain#{0}", i);

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer  = 0;
        createInfo.subresourceRange.layerCount = 1;

        PPE_VKDEVICE_CHECKED(device.vkCreateImageView,
            device.vkDevice(),
            &createInfo,
            device.vkAllocator(),
            &_imageViews[i] );
    }
}
//----------------------------------------------------------------------------
void FVulkanSwapchain::TearDownSwapchain(const FVulkanDevice& device) {
    Assert(VK_NULL_HANDLE != device.vkDevice());

    for (VkImageView imageView : _imageViews)
        device.vkDestroyImageView(device.vkDevice(), imageView, device.vkAllocator());

    _images.clear();
    _imageViews.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN