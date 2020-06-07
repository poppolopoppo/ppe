#include "stdafx.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIDevice.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIInstance.h"
#include "HAL/Vulkan/VulkanRHISwapChain.h"

#include "HAL/Vulkan/VulkanError.h"
#include "HAL/Vulkan/VulkanInterop.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDevice::FVulkanDevice(
    FVulkanPhysicalDevice physicalDevice,
    FVulkanDeviceHandle logicalDevice,
    FVulkanQueueHandle graphicsQueue,
    FVulkanQueueHandle presentQueue,
    FVulkanQueueHandle asyncComputeQueue,
    FVulkanQueueHandle transferQueue,
    FVulkanQueueHandle readbackQueue,
    FPresentModeList&& presentModes,
    FSurfaceFormatList&& surfaceFormats ) NOEXCEPT
:   _physicalDevice(physicalDevice)
,   _logicalDevice(logicalDevice)
,   _graphicsQueue(graphicsQueue)
,   _presentQueue(presentQueue)
,   _asyncComputeQueue(asyncComputeQueue)
,   _transferQueue(transferQueue)
,   _readBackQueue(readbackQueue)
,   _presentModes(std::move(presentModes))
,   _surfaceFormats(std::move(surfaceFormats)) {
    Assert_NoAssume(VK_NULL_HANDLE != _physicalDevice);
    Assert_NoAssume(VK_NULL_HANDLE != _logicalDevice);
    Assert_NoAssume( // at least one queue should be created !
        VK_NULL_HANDLE != _graphicsQueue ||
        VK_NULL_HANDLE != _presentQueue ||
        VK_NULL_HANDLE != _asyncComputeQueue ||
        VK_NULL_HANDLE != _transferQueue ||
        VK_NULL_HANDLE != _readBackQueue );
    Assert_NoAssume(VK_NULL_HANDLE != _graphicsQueue || VK_NULL_HANDLE != _presentQueue);
    Assert_NoAssume(_presentModes.size());
    Assert_NoAssume(_surfaceFormats.size());
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {
    // should have been destroyed by FVulkanInstance::DestroyLogicalDevice() !
    Assert_NoAssume(VK_NULL_HANDLE == _logicalDevice);
    Assert_NoAssume(VK_NULL_HANDLE == _graphicsQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _presentQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _asyncComputeQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _transferQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _readBackQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _swapChain);
}
//----------------------------------------------------------------------------
void FVulkanDevice::CreateSwapChain(
    FVulkanWindowSurface surface,
    EVulkanPresentMode present,
    const FVulkanSurfaceFormat& surfaceFormat ) {
    Assert(surface);
    AssertRelease(Contains(_presentModes, present));
    AssertRelease(Contains(_surfaceFormats, surfaceFormat));

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    Assert(VK_NULL_HANDLE == _swapChain);

    VkResult result;

    VkSurfaceCapabilitiesKHR capabilities{};
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, surface, &capabilities);
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result));

    // viewport size:
    VkExtent2D viewport;
    if (capabilities.currentExtent.height != UINT32_MAX) {
        viewport = {
            capabilities.currentExtent.width,
            capabilities.currentExtent.height };
    }
    else { // some window managers do allow us to differ here:
        viewport = {
            capabilities.maxImageExtent.width,
            capabilities.maxImageExtent.height };
    }
    Assert_NoAssume(viewport.width <= capabilities.maxImageExtent.width);
    Assert_NoAssume(viewport.height <= capabilities.maxImageExtent.height);

    // image count:
    u32 numImages = (capabilities.minImageCount + 1); // recommended to request at least one more image than the minimum
    if (capabilities.maxImageCount > 0 && numImages > capabilities.maxImageCount)
        numImages = capabilities.maxImageCount;

    // swap chain:
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    createInfo.surface = surface;
    createInfo.preTransform = capabilities.currentTransform; // screen rotation
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // desktop composition

    createInfo.presentMode = FVulkanInterop::PresentMode_to_VkPresentModeKHR(present);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    createInfo.minImageCount = numImages;
    createInfo.imageFormat = FVulkanInterop::PixelFormat_to_VkFormat(surfaceFormat.Format);
    createInfo.imageColorSpace = FVulkanInterop::ColorSpace_to_VkColorSpaceKHR(surfaceFormat.ColorSpace);
    createInfo.imageExtent = viewport;
    createInfo.imageArrayLayers = 1; // stereoscopy
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // #TODO: VK_IMAGE_USAGE_TRANSFER_DST_BIT
    //  It is also possible that you'll render images to a separate image first to perform operations like post-processing.
    //  In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // avoid VK_SHARING_MODE_CONCURRENT, prefer explicit resource transition
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    LOG(RHI, Debug, L"creating swapchain with viewport {0}x{1}", viewport.width, viewport.height);

    VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
    result = vkCreateSwapchainKHR(
        _logicalDevice,
        &createInfo,
        FVulkanInstance::Allocator(),
        &vkSwapChain );
    if (VK_SUCCESS != result)
        PPE_THROW_IT(FVulkanDeviceException("vkCreateSwapchainKHR", result));

    Assert(VK_NULL_HANDLE != vkSwapChain);

    _swapChain.reset(
        FVulkanSwapChainHandle{ vkSwapChain },
        u322{ viewport.width, viewport.height },
        surfaceFormat );
    _swapChain->InitializeSwapChain(*this);
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroySwapChain() {
    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    Assert(_swapChain);

    LOG_DIRECT(RHI, Debug, L"destroying swapchain with viewport");

    _swapChain->TearDownSwapChain(*this);
    _swapChain.reset();

    vkDestroySwapchainKHR(
        _logicalDevice,
        _swapChain->Handle(),
        FVulkanInstance::Allocator() );;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
