#include "stdafx.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIDevice.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIInstance.h"
#include "HAL/Vulkan/VulkanRHIPipelineLayout.h"
#include "HAL/Vulkan/VulkanRHIShaderStage.h"
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
    FVulkanAllocationCallbacks allocator,
    FVulkanPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkQueue asyncComputeQueue,
    VkQueue transferQueue,
    FPresentModeList&& presentModes,
    FSurfaceFormatList&& surfaceFormats ) NOEXCEPT
:   _allocator(allocator)
,   _physicalDevice(physicalDevice)
,   _logicalDevice(logicalDevice)
,   _graphicsQueue(graphicsQueue)
,   _presentQueue(presentQueue)
,   _asyncComputeQueue(asyncComputeQueue)
,   _transferQueue(transferQueue)
,   _presentModes(std::move(presentModes))
,   _surfaceFormats(std::move(surfaceFormats))
,   _deviceMemory(*this)
#if USE_PPE_RHIDEBUG
,   _debug(logicalDevice)
#endif
{
    Assert_NoAssume(_allocator);
    Assert_NoAssume(VK_NULL_HANDLE != _physicalDevice);
    Assert_NoAssume(VK_NULL_HANDLE != _logicalDevice);
    Assert_NoAssume( // at least one queue should be created !
        VK_NULL_HANDLE != _graphicsQueue ||
        VK_NULL_HANDLE != _presentQueue ||
        VK_NULL_HANDLE != _asyncComputeQueue ||
        VK_NULL_HANDLE != _transferQueue );
    Assert_NoAssume(VK_NULL_HANDLE != _graphicsQueue || VK_NULL_HANDLE != _presentQueue);
    Assert_NoAssume(_presentModes.size());
    Assert_NoAssume(_surfaceFormats.size());

    PPE_VKDEVICE_SETDEBUGNAME(*this, _graphicsQueue, "GraphicsQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _presentQueue, "PresentQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _asyncComputeQueue, "AsyncComputeQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _transferQueue, "TransferQueue");
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {
    // should have been destroyed by FVulkanInstance::DestroyLogicalDevice() !
    Assert_NoAssume(VK_NULL_HANDLE == _logicalDevice);
    Assert_NoAssume(VK_NULL_HANDLE == _graphicsQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _presentQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _asyncComputeQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _transferQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _swapChain);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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

    VkSurfaceCapabilitiesKHR capabilities{};
    PPE_VKDEVICE_CHECKED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, _physicalDevice, surface, &capabilities);

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

    createInfo.presentMode = FVulkanInterop::Vk(present);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    createInfo.minImageCount = numImages;
    createInfo.imageFormat = FVulkanInterop::Vk(surfaceFormat.Format);
    createInfo.imageColorSpace = FVulkanInterop::Vk(surfaceFormat.ColorSpace);
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
    PPE_VKDEVICE_CHECKED(vkCreateSwapchainKHR, _logicalDevice, &createInfo, _allocator, &vkSwapChain);

    Assert(VK_NULL_HANDLE != vkSwapChain);

    _swapChain.reset(
        _allocator,
        vkSwapChain,
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

    vkDestroySwapchainKHR(
        _logicalDevice,
        _swapChain->Handle(),
        _allocator );

    _swapChain.reset();
}
//----------------------------------------------------------------------------
VkShaderModule FVulkanDevice::CreateShaderModule(const FRawMemoryConst& code) {
    Assert(not code.empty());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = checked_cast<u32>(code.SizeInBytes());
    createInfo.pCode = reinterpret_cast<const u32*>(code.data());

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    VkShaderModule shaderModule;
    PPE_VKDEVICE_CHECKED(vkCreateShaderModule, _logicalDevice, &createInfo, _allocator, &shaderModule);

    Assert(VK_NULL_HANDLE != shaderModule);
    return shaderModule;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyShaderModule(VkShaderModule pShaderMod) {
    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    vkDestroyShaderModule(_logicalDevice, pShaderMod, _allocator);
}
//----------------------------------------------------------------------------
VkDescriptorSetLayout FVulkanDevice::CreateDescriptorSetLayout(const FVulkanDescriptorSetLayout& desc) {
    STACKLOCAL_POD_ARRAY(VkDescriptorBindingFlags, vkDescriptorBindingFlags, desc.Bindings.size());
    STACKLOCAL_POD_ARRAY(VkDescriptorSetLayoutBinding, vkDescriptorSetLayouBindings, desc.Bindings.size());

    forrange(i, 0, desc.Bindings.size()) {
        const FVulkanDescriptorBinding& binding = desc.Bindings[i];
        vkDescriptorBindingFlags[i] = FVulkanInterop::Vk(binding.BindingFlags);
        VkDescriptorSetLayoutBinding& vkBinding = vkDescriptorSetLayouBindings[i];
        vkBinding.binding = binding.BindingIndex;
        vkBinding.descriptorCount = checked_cast<u32>(binding.NumDescriptors);
        vkBinding.stageFlags = FVulkanInterop::Vk(binding.StageFlags);
        vkBinding.descriptorType = FVulkanInterop::Vk(binding.DescriptorType);
        vkBinding.pImmutableSamplers = nullptr; // #TODO ?
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.bindingCount = checked_cast<u32>(vkDescriptorBindingFlags.size());
    bindingFlags.pBindingFlags = vkDescriptorBindingFlags.data();

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.flags = FVulkanInterop::Vk(desc.SetFlags);
    createInfo.bindingCount = checked_cast<u32>(vkDescriptorSetLayouBindings.size());
    createInfo.pBindings = vkDescriptorSetLayouBindings.data();
    createInfo.pNext = &bindingFlags;

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    VkDescriptorSetLayout setLayout;
    PPE_VKDEVICE_CHECKED(vkCreateDescriptorSetLayout, _logicalDevice,&createInfo, _allocator, &setLayout);

    return setLayout;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyDescriptorSetLayout(VkDescriptorSetLayout setLayout) {
    Assert(VK_NULL_HANDLE != setLayout);

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    vkDestroyDescriptorSetLayout(_logicalDevice, setLayout, _allocator);
}
//----------------------------------------------------------------------------
VkPipelineLayout FVulkanDevice::CreatePipelineLayout(const FVulkanPipelineLayout& desc) {
    STACKLOCAL_POD_ARRAY(VkPushConstantRange, vkPushConstantRanges, desc.PushConstantRanges.size());

    forrange(i, 0, desc.PushConstantRanges.size()) {
        const FVulkanPushConstantRange& range = desc.PushConstantRanges[i];
        VkPushConstantRange& vkRange = vkPushConstantRanges[i];
        vkRange.offset = range.Offset;
        vkRange.size = range.Size;
        vkRange.stageFlags = FVulkanInterop::Vk(range.StageFlags);
    }

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = checked_cast<u32>(desc.DescriptorSetLayouts.size());
    createInfo.pSetLayouts = desc.DescriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = checked_cast<u32>(vkPushConstantRanges.size());
    createInfo.pPushConstantRanges = vkPushConstantRanges.data();

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    VkPipelineLayout pipelineLayout;
    PPE_VKDEVICE_CHECKED(vkCreatePipelineLayout, _logicalDevice, &createInfo, _allocator, &pipelineLayout);

    return pipelineLayout;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyPipelineLayout(VkPipelineLayout pipelineLayout) {
    Assert(VK_NULL_HANDLE != pipelineLayout);

    const Meta::FRecursiveLockGuard scopeLock(_barrier);

    vkDestroyPipelineLayout(_logicalDevice, pipelineLayout, _allocator);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
