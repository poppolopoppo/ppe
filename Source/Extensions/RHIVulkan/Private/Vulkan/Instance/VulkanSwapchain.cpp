// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Instance/VulkanSwapchain.h"

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Common/VulkanEnumToString.h"
#include "Vulkan/Command/VulkanCommandBuffer.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanFrameGraph.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "RHI/EnumToString.h"
#include "RHI/SwapchainDesc.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void DefaultColorFormats_(
    const TAppendable<TPair<VkFormat, VkColorSpaceKHR>>& supported,
    const TMemoryView<const VkSurfaceFormat2KHR>& formats ) {
    const VkFormat defaultFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // add first 3 formats
    forrange(i, 0, Min(3_size_t, formats.size())) {
        auto& item = formats[i].surfaceFormat;
        supported.emplace_back(
            item.format != VK_FORMAT_UNDEFINED
                ? item.format
                : defaultFormat,
            item.colorSpace );
    }
}
//----------------------------------------------------------------------------
static void DefaultPresentModes_(
    const TAppendable<VkPresentModeKHR>& supported,
    const TMemoryView<const VkPresentModeKHR>& presentModes) {
    Assert(not presentModes.empty());

    bool supportMode_FIFO = false;
    bool supportMode_MAILBOX = false;
    bool supportMode_IMMEDIATE = false;

    for (const VkPresentModeKHR mode : presentModes) {
        Assert(mode < VK_PRESENT_MODE_MAX_ENUM_KHR);
        supportMode_FIFO |= (mode == VK_PRESENT_MODE_FIFO_KHR);
        supportMode_MAILBOX |= (mode == VK_PRESENT_MODE_MAILBOX_KHR);
        supportMode_IMMEDIATE |= (mode == VK_PRESENT_MODE_IMMEDIATE_KHR);
    }

    if (supportMode_MAILBOX)
        supported.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
    else if (supportMode_FIFO)
        supported.push_back(VK_PRESENT_MODE_FIFO_KHR);
    else if (supportMode_IMMEDIATE)
        supported.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);
    else
        supported.push_back(presentModes.front());
}
//----------------------------------------------------------------------------
NODISCARD static bool CompositeAlpha_(
    VkCompositeAlphaFlagBitsKHR* pCompositeAlpha,
    const VkSurfaceCapabilitiesKHR& surfaceCaps ) {
    Assert(pCompositeAlpha);

    if (0 == *pCompositeAlpha)
        *pCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    if (Meta::EnumHas(surfaceCaps.supportedCompositeAlpha, static_cast<VkFlags>(*pCompositeAlpha)))
        return true;

    *pCompositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;

    for(VkCompositeAlphaFlagBitsKHR flag = Zero;
        flag < VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
        flag = static_cast<VkCompositeAlphaFlagBitsKHR>(flag << 1u) ) {
        if (Meta::EnumHas(surfaceCaps.supportedCompositeAlpha, static_cast<VkFlags>(flag))) {
            *pCompositeAlpha = flag;
            return true;
        }
    }

    PPE_LOG(RHI, Error, "no suitable composite alpha flags found");
    return false;
}
//----------------------------------------------------------------------------
static void SwapchainExtent_(uint2* pExtent, const VkSurfaceCapabilitiesKHR& surfaceCaps ) {
    Assert(pExtent);

    if (surfaceCaps.currentExtent.width == UMax &&
        surfaceCaps.currentExtent.height == UMax ) {
        // keep window size
    }
    else {
        *pExtent = {
            surfaceCaps.currentExtent.width,
            surfaceCaps.currentExtent.height };
    }
}
//----------------------------------------------------------------------------
static void SurfaceImageCount_(u32* pMinImageCount, const VkSurfaceCapabilitiesKHR& surfaceCaps ) {
    Assert(pMinImageCount);

    if (*pMinImageCount < surfaceCaps.minImageCount)
        *pMinImageCount = surfaceCaps.minImageCount;

    if (surfaceCaps.maxImageCount > 0 &&
        *pMinImageCount > surfaceCaps.maxImageCount )
        *pMinImageCount = surfaceCaps.minImageCount;
}
//----------------------------------------------------------------------------
static void SurfaceTransform_(VkSurfaceTransformFlagBitsKHR* pTransform, const VkSurfaceCapabilitiesKHR& surfaceCaps ) {
    Assert(pTransform);

    if (0 == *pTransform)
        *pTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    if (Meta::EnumHas(surfaceCaps.supportedTransforms, static_cast<VkFlags>(*pTransform)))
        return; // keep current

    if (Meta::EnumHas(surfaceCaps.supportedTransforms, static_cast<VkFlags>(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)))
        *pTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        *pTransform = surfaceCaps.currentTransform;
}
//----------------------------------------------------------------------------
NODISCARD static bool IsColorFormatSupported_(
    const TMemoryView<const VkSurfaceFormat2KHR>& formats,
    VkFormat requiredFormat, VkColorSpaceKHR requiredColorSpace ) {
    for (VkSurfaceFormat2KHR fmt : formats) {
        if (fmt.surfaceFormat.format == VK_FORMAT_UNDEFINED &&
            fmt.surfaceFormat.colorSpace == requiredColorSpace )
            return true;

        if (fmt.surfaceFormat.format == requiredFormat &&
            fmt.surfaceFormat.colorSpace == requiredColorSpace )
            return true;
    }

    return true;
}
//----------------------------------------------------------------------------
NODISCARD static bool IsPresentModeSupported_(
    const TMemoryView<const VkPresentModeKHR>& presentModes,
    VkPresentModeKHR required ) {
    return presentModes.Contains(required);
}
//----------------------------------------------------------------------------
NODISCARD static bool IsPresentModeSupported_(
    const TMemoryView<const EPresentMode>& presentModes,
    VkPresentModeKHR required ) {
    return !!presentModes.Any(
        [required](EPresentMode mode) {
            return (VkCast(mode) == required);
        });
}
//----------------------------------------------------------------------------
NODISCARD static bool ImageUsage_(
    VkImageUsageFlags* pImageUsage,
    const FVulkanDevice& device,
    VkPresentModeKHR presentMode,
    VkFormat colorFormat,
    VkSurfaceCapabilities2KHR surfaceCaps ) {
    if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ||
        presentMode == VK_PRESENT_MODE_MAILBOX_KHR ||
        presentMode == VK_PRESENT_MODE_FIFO_KHR ||
        presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR ) {
        *pImageUsage = surfaceCaps.surfaceCapabilities.supportedUsageFlags;
    }
    else
    if (presentMode == VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR ||
        presentMode == VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR ) {
        for(const auto* it = reinterpret_cast<VkBaseInStructure*>(&surfaceCaps);
            nullptr != it; it = it->pNext ) {
            if (it->sType == VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR) {
                *pImageUsage = reinterpret_cast<const VkSharedPresentSurfaceCapabilitiesKHR*>(it)
                     ->sharedPresentSupportedUsageFlags;
                break;
            }
        }
    }
    else {
        PPE_LOG(RHI, Error, "unsupported presented mode, can't choose image usage");
        return false;
    }

    Assert(Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)));
    *pImageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // validation:
    VkFormatProperties formatProps{};
    device.vkGetPhysicalDeviceFormatProperties(
        device.vkPhysicalDevice(),
        colorFormat,
        &formatProps );

    PPE_LOG_CHECK(RHI, Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)));
    Assert(Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)));

    if (Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) &&
        (not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) ||
         not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_BLIT_DST_BIT)) )) {
        *pImageUsage &= ~VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) &&
        not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) ) {
        *pImageUsage &= ~VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_STORAGE_BIT)) &&
        not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) ) {
        *pImageUsage &= ~VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_SAMPLED_BIT)) &&
        not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ) {
        *pImageUsage &= ~VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (Meta::EnumHas(*pImageUsage, static_cast<VkFlags>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
        not Meta::EnumHas(formatProps.optimalTilingFeatures, static_cast<VkFlags>(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) ) {
        *pImageUsage &= ~VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return true;
}
//----------------------------------------------------------------------------
static bool IsSupported_(
    VkImageUsageFlagBits* pColorImageUsage,
    const FVulkanDevice& device,
    const VkSurfaceCapabilities2KHR& surfaceCaps,
    const uint2& surfaceSize,
    VkPresentModeKHR presentMode,
    VkFormat colorFormat ) {
    Assert(pColorImageUsage);
    Assert(*pColorImageUsage);

    VkImageUsageFlags imageUsage{ 0 };
    if (not ImageUsage_(&imageUsage, device, presentMode, colorFormat, surfaceCaps))
        return false;

    if (not Meta::EnumXor(imageUsage, static_cast<VkFlags>(*pColorImageUsage)))
        return false;

    VkImageFormatProperties imageProps{};
    VK_CALL(device.vkGetPhysicalDeviceImageFormatProperties(
        device.vkPhysicalDevice(),
        colorFormat,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        imageUsage, 0, &imageProps ));

    if (not Meta::EnumHas(imageProps.sampleCounts, static_cast<VkFlags>(VK_SAMPLE_COUNT_1_BIT)))
        return false;

    if (surfaceSize.x > imageProps.maxExtent.width ||
        surfaceSize.y > imageProps.maxExtent.height ||
        imageProps.maxExtent.depth == 0 )
        return false;

    if (imageProps.maxArrayLayers < 1)
        return false;

    *pColorImageUsage = static_cast<VkImageUsageFlagBits>(imageUsage);
    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanSwapchain::FVulkanSwapchain() {
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SemaphoreId = 0;
    Broadcast(exclusiveData->ImageAvailable.MakeView(), VK_NULL_HANDLE);
    Broadcast(exclusiveData->RenderFinished.MakeView(), VK_NULL_HANDLE);
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanSwapchain::~FVulkanSwapchain() {
    Assert_NoAssume(VK_NULL_HANDLE == _data.LockExclusive()->vkSwapchain);
}
#endif
//----------------------------------------------------------------------------
bool FVulkanSwapchain::Construct(
    FVulkanFrameGraph& fg, const FSwapchainDesc& desc
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto& exclusiveData = _data.LockExclusive();
    const FVulkanExternalObject windowSurface{ desc.Surface.Value };

    PPE_LOG_CHECK(RHI, not exclusiveData->vkSwapchain || windowSurface.Cast<VkSurfaceKHR>() == exclusiveData->vkSurface);
    exclusiveData->vkSurface = windowSurface.Cast<VkSurfaceKHR>();

    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo.surface = exclusiveData->vkSurface;

    const FVulkanDevice& device = fg.Device();

    // get surface capabilities
    VkSurfaceCapabilities2KHR surfaceCaps{};
    surfaceCaps.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    VK_CHECK(device.vkGetPhysicalDeviceSurfaceCapabilities2KHR(
        device.vkPhysicalDevice(),
        &surfaceInfo,
        &surfaceCaps ));

    // get surface formats
    VECTOR(RHIImage, VkSurfaceFormat2KHR) surfaceFormats;
    u32 surfaceFormatCount = 0;
    VK_CHECK(device.vkGetPhysicalDeviceSurfaceFormats2KHR(
        device.vkPhysicalDevice(),
        &surfaceInfo,
        &surfaceFormatCount,
        nullptr ));

    AssertRelease(surfaceFormatCount > 0);
    surfaceFormats.resize(surfaceFormatCount, VkSurfaceFormat2KHR{
        VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
        nullptr,
        VkSurfaceFormatKHR{}
    });
    VK_CHECK(device.vkGetPhysicalDeviceSurfaceFormats2KHR(
        device.vkPhysicalDevice(),
        &surfaceInfo,
        &surfaceFormatCount,
        surfaceFormats.data() ));

    // get present modes
    VECTOR(RHIImage, VkPresentModeKHR) presentModes;
    u32 presentModesCount = 0;
    VK_CHECK(device.vkGetPhysicalDeviceSurfacePresentModesKHR(
        device.vkPhysicalDevice(),
        exclusiveData->vkSurface,
        &presentModesCount,
        nullptr ));

    AssertRelease(presentModesCount > 0);
    presentModes.resize(presentModesCount);
    VK_CHECK(device.vkGetPhysicalDeviceSurfacePresentModesKHR(
        device.vkPhysicalDevice(),
        exclusiveData->vkSurface,
        &presentModesCount,
        presentModes.data() ));

    exclusiveData->CompositeAlpha = VkCast(desc.CompositeAlpha);
    PPE_LOG_CHECK(RHI, CompositeAlpha_(&exclusiveData->CompositeAlpha, surfaceCaps.surfaceCapabilities));

    exclusiveData->SurfaceSize = desc.Dimensions;
    SwapchainExtent_(&exclusiveData->SurfaceSize, surfaceCaps.surfaceCapabilities);

    exclusiveData->MinImageCount = desc.MinImageCount;
    SurfaceImageCount_(&exclusiveData->MinImageCount, surfaceCaps.surfaceCapabilities);

    exclusiveData->PreTransform = VkCast(desc.PreTransform);
    SurfaceTransform_(&exclusiveData->PreTransform, surfaceCaps.surfaceCapabilities);

    // try to use current settings
    if (exclusiveData->vkSwapchain) {
        const bool presentModeSupported = (
            IsPresentModeSupported_(presentModes, exclusiveData->PresentMode) && (presentModes.empty() ||
            IsPresentModeSupported_(desc.PresentModes, exclusiveData->PresentMode) ));

        bool colorFormatSupported = IsColorFormatSupported_(
            surfaceFormats,
            exclusiveData->ColorFormat,
            exclusiveData->ColorSpace );

        if (not desc.SurfaceFormats.empty()) {
            colorFormatSupported &= !!desc.SurfaceFormats.MakeView().Any(
                [&exclusiveData](const FSurfaceFormat& it) {
                    return (VkCast(it.Format) == exclusiveData->ColorFormat &&
                            VkCast(it.ColorSpace) == exclusiveData->ColorSpace );
                });
        }

        if (presentModeSupported && colorFormatSupported)
            return exclusiveData->CreateSwapchain_(fg ARGS_IF_RHIDEBUG(debugName));
    }

    // find suitable color format
    TFixedSizeStack<VkPresentModeKHR, MaxImages> defaultPresentModes;
    DefaultPresentModes_(MakeAppendable(defaultPresentModes), presentModes);

    TFixedSizeStack<TPair<VkFormat, VkColorSpaceKHR>, MaxImages> defaultColorFormats;
    DefaultColorFormats_(MakeAppendable(defaultColorFormats), surfaceFormats);

    const VkImageUsageFlagBits requiredUsage = VkCast(desc.RequiredUsage);
    const VkImageUsageFlagBits optionalUsage = (VkCast(desc.OptionalUsage) | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    TFixedSizeStack<VkPresentModeKHR, MaxImages> requiredPresentModes;
    if (desc.PresentModes.empty())
        requiredPresentModes.Assign(defaultPresentModes);
    else
        requiredPresentModes.Assign(desc.PresentModes.MakeView().Map(
            [](EPresentMode presentMode) {
                return VkCast(presentMode);
            }));

    TFixedSizeStack<TPair<VkFormat, VkColorSpaceKHR>, MaxImages> requiredColorFormats;
    if (desc.SurfaceFormats.empty())
        requiredColorFormats.Assign(defaultColorFormats);
    else
        requiredColorFormats.Assign(desc.SurfaceFormats.MakeView().Map(
            [](const FSurfaceFormat& fmt) {
                return MakePair(VkCast(fmt.Format), VkCast(fmt.ColorSpace));
            }));

    forrange(i, 0, 3) {
        for (VkPresentModeKHR presentMode : requiredPresentModes) {
            if (not IsPresentModeSupported_(presentModes, presentMode))
                continue;

            for (const auto& fmt : requiredColorFormats) {
                exclusiveData->ColorFormat = fmt.first;
                exclusiveData->ColorSpace = fmt.second;

                if (not IsColorFormatSupported_(surfaceFormats, fmt.first, fmt.second))
                    continue;

                exclusiveData->ColorImageUsage = (requiredUsage | optionalUsage);
                exclusiveData->PresentMode = presentMode;

                if (IsSupported_(&exclusiveData->ColorImageUsage, device, surfaceCaps, exclusiveData->SurfaceSize, exclusiveData->PresentMode, exclusiveData->ColorFormat) && (
                    not requiredUsage || Meta::EnumHas(exclusiveData->ColorImageUsage, checked_cast<VkImageUsageFlagBits>(requiredUsage)) )) {

                    exclusiveData->Desc = desc;

                    return exclusiveData->CreateSwapchain_(fg ARGS_IF_RHIDEBUG(debugName));
                }
            }
        }

        // reset to default
        requiredPresentModes = defaultPresentModes;
        requiredColorFormats = defaultColorFormats;
    }

    PPE_LOG(RHI, Error, "can't find suitable format for swapchain");
    return false;
}
//----------------------------------------------------------------------------
void FVulkanSwapchain::TearDown(FVulkanResourceManager& resources) {
    const auto& exclusiveData = _data.LockExclusive();
    PPE_LOG_CHECKVOID(RHI, not exclusiveData->IsImageAcquired_());

    const FVulkanDevice& device = resources.Device();

    if (exclusiveData->vkSwapchain != VK_NULL_HANDLE) {
        device.vkDestroySwapchainKHR(
            device.vkDevice(),
            exclusiveData->vkSwapchain,
            device.vkAllocator() );
    }

    for (auto& vkSemaphore : exclusiveData->ImageAvailable) {
        if (VK_NULL_HANDLE != vkSemaphore) {
            device.vkDestroySemaphore(
                device.vkDevice(),
                vkSemaphore,
                device.vkAllocator() );

            vkSemaphore = VK_NULL_HANDLE;
        }
    }

    for (auto& vkSemaphore : exclusiveData->RenderFinished) {
        if (VK_NULL_HANDLE != vkSemaphore) {
            device.vkDestroySemaphore(
                device.vkDevice(),
                vkSemaphore,
                device.vkAllocator() );

            vkSemaphore = VK_NULL_HANDLE;
        }
    }

    if (exclusiveData->vkFence != VK_NULL_HANDLE) {
        device.vkDestroyFence(
            device.vkDevice(),
            exclusiveData->vkFence,
            device.vkAllocator() );

        exclusiveData->vkFence = VK_NULL_HANDLE;
    }

    exclusiveData->TearDownImages_(resources);

    exclusiveData->Desc = Default;
    exclusiveData->PresentQueue = PVulkanDeviceQueue{};
    exclusiveData->SurfaceSize = uint2::Zero;
    exclusiveData->vkSwapchain = VK_NULL_HANDLE;
    exclusiveData->vkSurface = VK_NULL_HANDLE;
    exclusiveData->vkFence = VK_NULL_HANDLE;
    exclusiveData->CurrentImageIndex = UMax;

    exclusiveData->ColorFormat = VK_FORMAT_UNDEFINED;
    exclusiveData->ColorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;
    exclusiveData->MinImageCount = 2;
    exclusiveData->PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    exclusiveData->CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    exclusiveData->ColorImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::Acquire(
    FRawImageID* pImageId,
    FVulkanCommandBuffer& cmd
    ARGS_IF_RHIDEBUG(bool debugSync) ) const {
    const auto exclusiveData = _data.LockExclusive();
    Assert(pImageId);
    Assert(VK_NULL_HANDLE != exclusiveData->vkSwapchain);

    if (exclusiveData->IsImageAcquired_()) {
        *pImageId = *exclusiveData->ImageIds[exclusiveData->CurrentImageIndex];
        return true;
    }

    VkAcquireNextImageInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    info.swapchain = exclusiveData->vkSwapchain;
    info.timeout = UMax;
    info.semaphore = exclusiveData->ImageAvailable[exclusiveData->SemaphoreId];
    info.deviceMask = 1;

#if USE_PPE_RHIDEBUG
    if (debugSync) {
        info.fence = exclusiveData->vkFence;
    }
#endif

    const FVulkanDevice& device = cmd.Device();
    VK_CHECK(device.vkAcquireNextImage2KHR(
        device.vkDevice(),
        &info,
        &exclusiveData->CurrentImageIndex ));

#if USE_PPE_RHIDEBUG
    if (debugSync) {
        VK_CALL(device.vkWaitForFences(
            device.vkDevice(),
            1, &exclusiveData->vkFence,
            VK_TRUE, UMax ));
        VK_CALL(device.vkResetFences(
            device.vkDevice(),
            1, &exclusiveData->vkFence ));
    }
#endif

    *pImageId = *exclusiveData->ImageIds[exclusiveData->CurrentImageIndex];

    cmd.WaitSemaphore(
        exclusiveData->ImageAvailable[exclusiveData->SemaphoreId],
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT );
    cmd.SignalSemaphore(exclusiveData->RenderFinished[exclusiveData->SemaphoreId]);

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::Present(const FVulkanDevice& device) const {
    const auto exclusiveData = _data.LockExclusive();
    if (not exclusiveData->IsImageAcquired_())
        return true;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &exclusiveData->vkSwapchain;
    presentInfo.pImageIndices = &exclusiveData->CurrentImageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &exclusiveData->RenderFinished[exclusiveData->SemaphoreId];

    const VkResult err = device.vkQueuePresentKHR(exclusiveData->PresentQueue->Handle, &presentInfo);

    exclusiveData->CurrentImageIndex = UMax;
    exclusiveData->SemaphoreId++;

    switch (err) {
    case VK_SUCCESS: return true;

    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_SURFACE_LOST_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        // #TODO: recreate swapchain
    default:
        PPE_LOG(RHI, Error, "present failed: {0}", FVulkanError(err));
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::IsImageAcquired_() const NOEXCEPT {
    return CurrentImageIndex < ImageIds.size();
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::CreateSwapchain_(FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    VkSwapchainKHR oldSwapchain = vkSwapchain;
    const FVulkanDevice& device = fg.Device();

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkSurface;
    createInfo.imageFormat = ColorFormat;
    createInfo.imageColorSpace = ColorSpace;
    createInfo.imageExtent = { SurfaceSize.x, SurfaceSize.y };
    createInfo.imageArrayLayers = 1;
    createInfo.minImageCount = MinImageCount;
    createInfo.oldSwapchain = oldSwapchain;
    createInfo.clipped = VK_TRUE;
    createInfo.preTransform = PreTransform;
    createInfo.presentMode = PresentMode;
    createInfo.compositeAlpha = CompositeAlpha;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage = ColorImageUsage;

    VK_CHECK(device.vkCreateSwapchainKHR(
        device.vkDevice(),
        &createInfo,
        device.vkAllocator(),
        &vkSwapchain ));

    TearDownImages_(fg.ResourceManager());

    if (oldSwapchain != VK_NULL_HANDLE)
        device.vkDestroySwapchainKHR(device.vkDevice(), oldSwapchain, device.vkAllocator());

#if USE_PPE_RHIDEBUG
    if (debugName) {
        device.SetObjectName(vkSwapchain, debugName, VK_OBJECT_TYPE_SWAPCHAIN_KHR);
    }
#endif

    // get supported queue families
    {
        EVulkanQueueFamilyMask availableQueues = device.AvailableQueues();
        QueueFamilyMask = Default;

        for (u32 i = 0; (1u << i) <= static_cast<u32>(availableQueues); ++i) {
            const auto q = static_cast<EVulkanQueueFamilyMask>(1u << i);

            if (availableQueues & q) {
                VkBool32 supportsPresent{ 0 };
                VK_CALL(device.vkGetPhysicalDeviceSurfaceSupportKHR(
                    device.vkPhysicalDevice(),
                    i, vkSurface, &supportsPresent ));

                if (supportsPresent)
                    QueueFamilyMask |= q;
            }
        }
    }

    PPE_LOG_CHECK(RHI, CreateImages_(fg.ResourceManager()));
    PPE_LOG_CHECK(RHI, CreateSemaphores_(device));
    PPE_LOG_CHECK(RHI, CreateFence_(device));
    PPE_LOG_CHECK(RHI, ChoosePresentQueue_(fg));

#if USE_PPE_RHIDEBUG
    PPE_LOG(RHI, Info, "swapchain properties:\n"
        "\tName                    : {0}\n"
        "\tColor format            : {1}\n"
        "\tColor space             : {2}\n"
        "\tImage count             : {3}\n"
        "\tPresent mode            : {4}\n"
        "\tPre transform           : {5}\n"
        "\tComposite alpha         : {6}\n"
        "\tImage usage             : {7}\n"
        "\tQueue family            : {8}\n"
        "\tQueue name              : {9}",
        debugName.MakeView(),
        RHICast(ColorFormat),
        ColorSpace,
        ImageIds.size(),
        PresentMode,
        PreTransform,
        CompositeAlpha,
        ColorImageUsage,
        static_cast<u32>(PresentQueue->FamilyIndex),
        PresentQueue->DebugName );
#endif

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::CreateImages_(FVulkanResourceManager& resources) {
    Assert(ImageIds.empty());

    const FVulkanDevice& device = resources.Device();

    TFixedSizeStack<VkImage, MaxImages> images;
    {
        u32 imageCount = 0;
        VK_CHECK(device.vkGetSwapchainImagesKHR(
            device.vkDevice(),
            vkSwapchain,
            &imageCount,
            nullptr ));

        images.Resize(imageCount);

        VK_CHECK(device.vkGetSwapchainImagesKHR(
            device.vkDevice(),
            vkSwapchain,
            &imageCount,
            images.data() ));
    }

    FVulkanExternalImageDesc desc;
    desc.Type = VK_IMAGE_TYPE_2D;
    desc.Usage = ColorImageUsage;
    desc.Format = ColorFormat;
    desc.CurrentLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    desc.DefaultLayout = desc.CurrentLayout;
    desc.Samples = VK_SAMPLE_COUNT_1_BIT;
    desc.Dimensions = uint3(SurfaceSize, 0);
    desc.ArrayLayers = 1;
    desc.MaxLevels = 1;
    desc.QueueFamily = VK_QUEUE_FAMILY_IGNORED;

#if USE_PPE_RHIDEBUG
    Assert(images.size() < 10);
    char debugName[] = "SwapchainImage-0";
#endif

    forrange(i, 0, images.size()) {
        desc.Image = images[i];
        ONLY_IF_RHIDEBUG(debugName[lengthof(debugName) - 2] = static_cast<char>('0' + i));

        ImageIds.Push(resources.CreateImage(
            desc, NoFunction
            ARGS_IF_RHIDEBUG(debugName) ));

        PPE_LOG_CHECK(RHI, ImageIds[i].Valid());
    }

    Assert_NoAssume(ImageIds.size() == images.size());
    return true;
}
//----------------------------------------------------------------------------
void FVulkanSwapchain::FInternalData_::TearDownImages_(FVulkanResourceManager& resources) {
    for (FImageID& id : ImageIds) {
        resources.ReleaseResource(id.Release());
    }

    ImageIds.clear();
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::CreateSemaphores_(const FVulkanDevice& device) {
    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    forrange(i, 0, ImageAvailable.size()) {
        VkSemaphore& semaphore = ImageAvailable[i];
        if (VK_NULL_HANDLE != semaphore)
            continue;

        VK_CHECK(device.vkCreateSemaphore(
            device.vkDevice(),
            &info,
            device.vkAllocator(),
            &semaphore));

        ONLY_IF_RHIDEBUG(device.SetObjectName(
            semaphore,
            *StringFormat("SwapChain/ImageAvailable#{0}", i),
            VK_OBJECT_TYPE_SEMAPHORE));
    }

    forrange(i, 0, RenderFinished.size()) {
        VkSemaphore& semaphore = RenderFinished[i];
        if (VK_NULL_HANDLE != semaphore)
            continue;

        VK_CHECK(device.vkCreateSemaphore(
            device.vkDevice(),
            &info,
            device.vkAllocator(),
            &semaphore));

        ONLY_IF_RHIDEBUG(device.SetObjectName(
            semaphore,
            *StringFormat("SwapChain/RenderFinished#{0}", i),
            VK_OBJECT_TYPE_SEMAPHORE));
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::CreateFence_(const FVulkanDevice& device) {
    if (VK_NULL_HANDLE == vkFence) {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VK_CHECK(device.vkCreateFence(
            device.vkDevice(),
            &info,
            device.vkAllocator(),
            &vkFence ));
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanSwapchain::FInternalData_::ChoosePresentQueue_(const FVulkanFrameGraph& fg) NOEXCEPT {
    if (PresentQueue)
        return true;

    forrange(i, 0, static_cast<u32>(EQueueType::_Count)) {
        if (const PVulkanDeviceQueue queue = fg.FindQueue(static_cast<EQueueType>(i))) {
            const FVulkanDevice& device = fg.Device();

            VkBool32 supportsPresent = 0;
            VK_CALL(device.vkGetPhysicalDeviceSurfaceSupportKHR(
                device.vkPhysicalDevice(),
                static_cast<uint32_t>(queue->FamilyIndex),
                vkSurface,
                &supportsPresent ));

            if (supportsPresent) {
                PresentQueue = queue;
                return true;
            }
        }
    }

    PPE_LOG(RHI, Error, "can't find queue that supports present");
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
