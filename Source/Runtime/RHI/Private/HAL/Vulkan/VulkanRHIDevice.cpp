#include "stdafx.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanRHIDevice.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"
#include "HAL/Vulkan/VulkanRHIInstance.h"
#include "HAL/Vulkan/VulkanRHIPipelineLayout.h"
#include "HAL/Vulkan/VulkanRHIShaderStage.h"
#include "HAL/Vulkan/VulkanRHISwapChain.h"

#include "HAL/Vulkan/VulkanError.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDevice::FVulkanDevice(
    const FVulkanInstance& instance,
    VkPhysicalDevice vkPhysicalDevice,
    TArray<EVulkanPresentMode>&& presentModes,
    TArray<FVulkanSurfaceFormat>&& surfaceFormats ) NOEXCEPT
:   _vkDevice(VK_NULL_HANDLE)
,   _vkPhysicalDevice(vkPhysicalDevice)
,   _vkGraphicsQueue(VK_NULL_HANDLE)
,   _vkPresentQueue(VK_NULL_HANDLE)
,   _vkAsyncComputeQueue(VK_NULL_HANDLE)
,   _vkTransferQueue(VK_NULL_HANDLE)
,   _instance(instance)
,   _deviceMemory(*this)
#if USE_PPE_RHIDEBUG
,   _debug(*this)
#endif
,   _presentModes(std::move(presentModes))
,   _surfaceFormats(std::move(surfaceFormats)) {
    Assert_NoAssume(VK_NULL_HANDLE != _vkPhysicalDevice);
    Assert_NoAssume(_presentModes.size());
    Assert_NoAssume(_surfaceFormats.size());
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {
    // should have been destroyed by FVulkanInstance::DestroyLogicalDevice() !
    Assert_NoAssume(VK_NULL_HANDLE == _vkDevice);
    Assert_NoAssume(VK_NULL_HANDLE == _vkGraphicsQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkPresentQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkAsyncComputeQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _vkTransferQueue);
    Assert_NoAssume(VK_NULL_HANDLE == _swapChain);
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
    if (not FVulkanDeviceFunctions::AttachDevice(this, _instance, _vkDevice) ||
        not FVulkanDeviceFunctions::AttachExtensions(this, _instance, _vkDevice)) {
        return false;
    }

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

    PPE_VKDEVICE_SETDEBUGNAME(*this, _vkGraphicsQueue, "GraphicsQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _vkPresentQueue, "PresentQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _vkAsyncComputeQueue, "AsyncComputeQueue");
    PPE_VKDEVICE_SETDEBUGNAME(*this, _vkTransferQueue, "TransferQueue");

    _deviceMemory.CreateDeviceHeaps();

    return true;
}
//----------------------------------------------------------------------------
const VkAllocationCallbacks* FVulkanDevice::vkAllocator() const {
    return _instance.vkAllocator();
}
//----------------------------------------------------------------------------
void FVulkanDevice::CreateSwapChain(
    FVulkanWindowSurface surface,
    EVulkanPresentMode present,
    const FVulkanSurfaceFormat& surfaceFormat ) {
    Assert(surface);
    AssertRelease(Contains(_presentModes, present));
    AssertRelease(Contains(_surfaceFormats, surfaceFormat));

    const FCriticalScope scopeLock(&_barrier);
    Assert(not _swapChain.valid());

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
    createInfo.imageFormat = static_cast<VkFormat>(surfaceFormat.Format);
    createInfo.imageColorSpace = static_cast<VkColorSpaceKHR>(surfaceFormat.ColorSpace);
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
    _swapChain.reset(vkSwapChain, extent, surfaceFormat );
    _swapChain->InitializeSwapChain(*this);
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroySwapChain() {
    const FCriticalScope scopeLock(&_barrier);

    Assert(_swapChain);

    LOG_DIRECT(RHI, Debug, L"destroying swapchain with viewport");

    _swapChain->TearDownSwapChain(*this);

    vkDestroySwapchainKHR(_vkDevice, _swapChain->Handle(), _instance.vkAllocator());

    _swapChain.reset();
}
//----------------------------------------------------------------------------
VkShaderModule FVulkanDevice::CreateShaderModule(const FRawMemoryConst& code) {
    Assert(not code.empty());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = checked_cast<u32>(code.SizeInBytes());
    createInfo.pCode = reinterpret_cast<const u32*>(code.data());

    const FCriticalScope scopeLock(&_barrier);

    VkShaderModule shaderModule;
    PPE_VKDEVICE_CHECKED(vkCreateShaderModule, _vkDevice, &createInfo, _instance.vkAllocator(), &shaderModule);

    Assert(VK_NULL_HANDLE != shaderModule);
    return shaderModule;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyShaderModule(VkShaderModule pShaderMod) {
    const FCriticalScope scopeLock(&_barrier);

    vkDestroyShaderModule(_vkDevice, pShaderMod, _instance.vkAllocator());
}
//----------------------------------------------------------------------------
VkDescriptorSetLayout FVulkanDevice::CreateDescriptorSetLayout(const FVulkanDescriptorSetLayout& desc) {
    STACKLOCAL_POD_ARRAY(VkDescriptorBindingFlags, vkDescriptorBindingFlags, desc.Bindings.size());
    STACKLOCAL_POD_ARRAY(VkDescriptorSetLayoutBinding, vkDescriptorSetLayouBindings, desc.Bindings.size());

    forrange(i, 0, desc.Bindings.size()) {
        const FVulkanDescriptorBinding& binding = desc.Bindings[i];
        vkDescriptorBindingFlags[i] = static_cast<VkDescriptorBindingFlags>(binding.BindingFlags);
        VkDescriptorSetLayoutBinding& vkBinding = vkDescriptorSetLayouBindings[i];
        vkBinding.binding = binding.BindingIndex;
        vkBinding.descriptorCount = checked_cast<u32>(binding.NumDescriptors);
        vkBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.StageFlags);
        vkBinding.descriptorType = static_cast<VkDescriptorType>(binding.DescriptorType);
        vkBinding.pImmutableSamplers = nullptr; // #TODO: immutable sampler ?
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.bindingCount = checked_cast<u32>(vkDescriptorBindingFlags.size());
    bindingFlags.pBindingFlags = vkDescriptorBindingFlags.data();

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.flags = static_cast<VkDescriptorSetLayoutCreateFlags>(desc.SetFlags);
    createInfo.bindingCount = checked_cast<u32>(vkDescriptorSetLayouBindings.size());
    createInfo.pBindings = vkDescriptorSetLayouBindings.data();
    createInfo.pNext = &bindingFlags;

    const FCriticalScope scopeLock(&_barrier);

    VkDescriptorSetLayout setLayout;
    PPE_VKDEVICE_CHECKED(vkCreateDescriptorSetLayout, _vkDevice,&createInfo, _instance.vkAllocator(), &setLayout);

    return setLayout;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyDescriptorSetLayout(VkDescriptorSetLayout setLayout) {
    Assert(VK_NULL_HANDLE != setLayout);

    const FCriticalScope scopeLock(&_barrier);

    vkDestroyDescriptorSetLayout(_vkDevice, setLayout, _instance.vkAllocator());
}
//----------------------------------------------------------------------------
VkPipelineLayout FVulkanDevice::CreatePipelineLayout(const FVulkanPipelineLayout& desc) {
    STACKLOCAL_POD_ARRAY(VkPushConstantRange, vkPushConstantRanges, desc.PushConstantRanges.size());

    forrange(i, 0, desc.PushConstantRanges.size()) {
        const FVulkanPushConstantRange& range = desc.PushConstantRanges[i];
        VkPushConstantRange& vkRange = vkPushConstantRanges[i];
        vkRange.offset = range.Offset;
        vkRange.size = range.Size;
        vkRange.stageFlags = static_cast<VkShaderStageFlags>(range.StageFlags);
    }

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = checked_cast<u32>(desc.SetLayouts.size());
    createInfo.pSetLayouts = &desc.SetLayouts.front();
    createInfo.pushConstantRangeCount = checked_cast<u32>(vkPushConstantRanges.size());
    createInfo.pPushConstantRanges = vkPushConstantRanges.data();

    const FCriticalScope scopeLock(&_barrier);

    VkPipelineLayout pipelineLayout;
    PPE_VKDEVICE_CHECKED(vkCreatePipelineLayout, _vkDevice, &createInfo, _instance.vkAllocator(), &pipelineLayout);

    return pipelineLayout;
}
//----------------------------------------------------------------------------
void FVulkanDevice::DestroyPipelineLayout(VkPipelineLayout pipelineLayout) {
    Assert(VK_NULL_HANDLE != pipelineLayout);

    const FCriticalScope scopeLock(&_barrier);

    vkDestroyPipelineLayout(_vkDevice, pipelineLayout, _instance.vkAllocator());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
