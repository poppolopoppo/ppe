#include "stdafx.h"

#include "Vulkan/VulkanDevice.h"

#include "Vulkan/VulkanIncludes.h"
//#include "Vulkan/VulkanInstance.h"
#include "Vulkan/VulkanSwapchain.h"

#include "Vulkan/Common/VulkanError.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDevice::FVulkanDevice(
    FVulkanInstance& instance,
    VkPhysicalDevice vkPhysicalDevice,
    EVulkanVendor vendorId,
    TArray<VkPresentModeKHR>&& presentModes,
    TArray<VkSurfaceFormatKHR>&& surfaceFormats ) NOEXCEPT
:   _vendorId(vendorId)
,   _vkPhysicalDevice(vkPhysicalDevice)
,   _vkAllocator(*instance.vkAllocator())
,   _vkDevice(VK_NULL_HANDLE)
,   _vkGraphicsQueue(VK_NULL_HANDLE)
,   _vkPresentQueue(VK_NULL_HANDLE)
,   _vkAsyncComputeQueue(VK_NULL_HANDLE)
,   _vkTransferQueue(VK_NULL_HANDLE)
,   _instance(instance)
,   _deviceMemory(*this)
,   _presentModes(std::move(presentModes))
,   _surfaceFormats(std::move(surfaceFormats))
#if USE_PPE_RHIDEBUG
,   _debug(*this)
#endif
{
    Assert_NoAssume(VK_NULL_HANDLE != _vkPhysicalDevice);
    Assert_NoAssume(not _presentModes.empty());
    Assert_NoAssume(not _surfaceFormats.empty());
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {
    // should have been destroyed by FVulkanInstance::DestroyLogicalDevice() !
    Assert_NoAssume(VK_NULL_HANDLE == _vkDevice);
    Assert_NoAssume(VK_NULL_HANDLE == _vkGraphicsQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkPresentQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkAsyncComputeQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkTransferQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _swapchain);
}
//----------------------------------------------------------------------------
bool FVulkanDevice::SetupDevice(
    VkDevice vkDevice,
    FVulkanDeviceQueueInfo graphicsQueue,
    FVulkanDeviceQueueInfo presentQueue,
    FVulkanDeviceQueueInfo asyncComputeQueue,
    FVulkanDeviceQueueInfo transferQueue) {
    AssertRelease(VK_NULL_HANDLE != vkDevice);

    const FCriticalScope deviceLock(&_barrier); // not necessary, since we never shared the ptr ATM

    Assert_NoAssume(VK_NULL_HANDLE == _vkDevice);
    Assert_NoAssume(VK_NULL_HANDLE == _vkGraphicsQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkPresentQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkAsyncComputeQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkTransferQueue);

    _vkDevice = vkDevice;

    // need to bind device functions before anything else
    if (not AttachDevice(this, _instance, _vkDevice))
        return false;

    if (not graphicsQueue.IsInvalid())
        vkGetDeviceQueue(_vkDevice, graphicsQueue.FamilyIndex, graphicsQueue.FamilyQueueIndex, &_vkGraphicsQueue);
    if (not presentQueue.IsInvalid())
        vkGetDeviceQueue(_vkDevice, presentQueue.FamilyIndex, presentQueue.FamilyQueueIndex, &_vkPresentQueue);
    if (not asyncComputeQueue.IsInvalid())
        vkGetDeviceQueue(_vkDevice, asyncComputeQueue.FamilyIndex, asyncComputeQueue.FamilyQueueIndex, &_vkAsyncComputeQueue);
    if (not transferQueue.IsInvalid())
        vkGetDeviceQueue(_vkDevice, transferQueue.FamilyIndex, transferQueue.FamilyQueueIndex, &_vkTransferQueue);

    Assert_NoAssume( // at least one queue should be created !
        VK_NULL_HANDLE != _vkGraphicsQueue ||
        VK_NULL_HANDLE != _vkPresentQueue ||
        VK_NULL_HANDLE != _vkAsyncComputeQueue ||
        VK_NULL_HANDLE != _vkTransferQueue);
    Assert_NoAssume(VK_NULL_HANDLE != _vkGraphicsQueue || VK_NULL_HANDLE != _vkPresentQueue);

    PPE_VKDEVICE_SETDEBUGNAME(*this, VkQueue, _vkGraphicsQueue, "GraphicsQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, VkQueue, _vkPresentQueue, "PresentQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, VkQueue, _vkAsyncComputeQueue, "AsyncComputeQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, VkQueue, _vkTransferQueue, "TransferQueue");

    _deviceMemory.CreateDeviceHeaps();

    return true;
}
//----------------------------------------------------------------------------
void FVulkanDevice::TearDownDevice() {
    AssertRelease(VK_NULL_HANDLE != _vkDevice);

    const FCriticalScope deviceLock(&_barrier); // not necessary, since we never shared the ptr ATM

    Assert_NoAssume( // at least one queue should have been created !
        VK_NULL_HANDLE != _vkGraphicsQueue ||
        VK_NULL_HANDLE != _vkPresentQueue ||
        VK_NULL_HANDLE != _vkAsyncComputeQueue ||
        VK_NULL_HANDLE != _vkTransferQueue);
    Assert_NoAssume(VK_NULL_HANDLE != _vkGraphicsQueue || VK_NULL_HANDLE != _vkPresentQueue);

    _deviceMemory.DestroyDeviceHeaps();

    _vkGraphicsQueue = VK_NULL_HANDLE;
    _vkPresentQueue = VK_NULL_HANDLE;
    _vkAsyncComputeQueue = VK_NULL_HANDLE;
    _vkTransferQueue = VK_NULL_HANDLE;

    _vkDevice = VK_NULL_HANDLE;
}
//----------------------------------------------------------------------------
const VkAllocationCallbacks* FVulkanDevice::vkAllocator() const {
    return _instance.vkAllocator();
}
//----------------------------------------------------------------------------
void FVulkanDevice::CreateSwapChain(
    FVulkanWindowSurface surface,
    VkPresentModeKHR present,
    const VkSurfaceFormatKHR& surfaceFormat ) {
    Assert(surface);
    AssertRelease(Contains(_presentModes, present));
    AssertRelease(_surfaceFormats.end() != std::find_if(_surfaceFormats.begin(), _surfaceFormats.end(), [&surfaceFormat](const VkSurfaceFormatKHR& it) {
        return (it.format == surfaceFormat.format && it.colorSpace == surfaceFormat.colorSpace);
    }));

    const FCriticalScope scopeLock(&_barrier);
    Assert(not _swapchain.valid());

    VkSurfaceCapabilitiesKHR capabilities{};
    VkResult vkResult = _instance.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vkPhysicalDevice, surface, &capabilities);
    if (VK_SUCCESS != vkResult)
        PPE_THROW_IT(FVulkanDeviceException{ "vkGetPhysicalDeviceSurfaceCapabilitiesKHR", vkResult });

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

    createInfo.presentMode = static_cast<VkPresentModeKHR>(present);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    createInfo.minImageCount = numImages;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = viewport;
    createInfo.imageArrayLayers = 1; // stereoscopy
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // #TODO: VK_IMAGE_USAGE_TRANSFER_DST_BIT
    //  It is also possible that you'll render images to a separate image first to perform operations like post-processing.
    //  In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // avoid VK_SHARING_MODE_CONCURRENT, prefer explicit resource transition
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    LOG(RHI, Debug, L"creating swapchain with viewport {0}x{1} and {2} images", viewport.width, viewport.height, numImages);

    VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
    PPE_VKDEVICE_CHECKED(vkCreateSwapchainKHR, _vkDevice, &createInfo, _instance.vkAllocator(), &vkSwapChain);

    Assert(VK_NULL_HANDLE != vkSwapChain);

    const u322 extent{ viewport.width, viewport.height };
    _swapchain.reset(vkSwapChain, extent, surfaceFormat );
    _swapchain->InitializeSwapchain(*this);
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroySwapChain() {
    const FCriticalScope scopeLock(&_barrier);

    Assert(_swapchain);

    LOG_DIRECT(RHI, Debug, L"destroying swapchain with viewport");

    _swapchain->TearDownSwapchain(*this);

    vkDestroySwapchainKHR(_vkDevice, _swapchain->Handle(), _instance.vkAllocator());

    _swapchain.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
