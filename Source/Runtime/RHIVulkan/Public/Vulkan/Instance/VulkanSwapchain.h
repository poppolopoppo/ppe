#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanSwapchainDesc {
    using FRequiredSurfaceFormats = TFixedSizeStack<VkSurfaceFormatKHR, 4>;
    using FRequiredPresentModes = TFixedSizeStack<VkPresentModeKHR, 4>;

    VkSurfaceKHR Surface{};
    uint2 Dimension{ 0 };
    u32 MinImageCount{ 2 };
    VkSurfaceTransformFlagBitsKHR PreTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    VkCompositeAlphaFlagBitsKHR CompositeAlpha{ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR };
    VkImageUsageFlags RequiredUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
    VkImageUsageFlags OptionalUsage{};

    // both from higher to lower priority:
    FRequiredSurfaceFormats SurfaceFormats;
    FRequiredPresentModes PresentModes;
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSwapchain final : public FRefCountable {
public:
    STATIC_CONST_INTEGRAL(u32, MaxImages, 8);
    using FSwapchainImages = TFixedSizeStack<FImageID, MaxImages>;

    FVulkanSwapchain();
    ~FVulkanSwapchain();

    bool Create(FVulkanFrameGraph& fg, const FVulkanSwapchainDesc& desc ARGS_IF_RHIDEBUG(FStringView debugName));
    void TearDown(FVulkanResourceManager& resources);

    bool Acquire(FRawImageID* pimageId, FVulkanCommandBuffer& cmd, ESwapchainImage type ARGS_IF_RHIDEBUG(bool debugSync = false)) const;
    bool Present(const FVulkanDevice& device) const;

    const PVulkanDeviceQueue& PresentQueue() const { return _presentQueue; }

private:
    bool CreateSwapchain_(FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FStringView debugName));

    bool CreateImages_(FVulkanResourceManager& resources);
    bool TearDownImages_(FVulkanResourceManager& resources);

    bool CreateSemaphores_(const FVulkanDevice& device);
    bool CreateFence_(const FVulkanDevice& device);
    bool ChoosePresnetQueue_(const FVulkanFrameGraph& fg);

    bool IsImageAcquired_() const;

    PVulkanDeviceQueue _presentQueue;
    FSwapchainImages _imageIds;
    uint2 _surfaceSize;

    VkSwapchainKHR _vkSwapchain{ VK_NULL_HANDLE };
    VkSurfaceKHR _vkSurface{ VK_NULL_HANDLE };
    mutable u32 _currentImage{ UMax };

    TStaticArray<VkSemaphore, 2> _imageAvailable;
    TStaticArray<VkSemaphore, 2> _renderFinished;
    VkFence _vkFence{ VK_NULL_HANDLE };
    mutable u32 _currentSemaphore : 1;

    VkFormat _colorFormat{ VK_FORMAT_UNDEFINED };
    VkColorSpaceKHR _colorSpace{ VK_COLOR_SPACE_MAX_ENUM_KHR };
    u32 _minImageCount{ 2 };
    VkSurfaceTransformFlagBitsKHR _preTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    VkPresentModeKHR __presentMode{ VK_PRESENT_MODE_FIFO_KHR };
    VkCompositeAlphaFlagBitsKHR _compositeAlpha{ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR };
    VkImageUsageFlags _colorImageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };

    EVulkanQueueFamilyMask _queueFamilyMask{ Default };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
