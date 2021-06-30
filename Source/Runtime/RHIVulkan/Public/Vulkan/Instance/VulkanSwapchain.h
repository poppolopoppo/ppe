#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Container/Array.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSwapchain final : Meta::FNonCopyable {
public:
    STATIC_CONST_INTEGRAL(u32, MaxImages, 8);
    using FSwapchainImages = TFixedSizeStack<FImageID, MaxImages>;

    FVulkanSwapchain();
    ~FVulkanSwapchain();

    NODISCARD bool Construct(FVulkanFrameGraph& fg, const FSwapchainDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD bool ReConstruct(FVulkanFrameGraph& fg, const FSwapchainDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool Acquire(FRawImageID* pimageId, FVulkanCommandBuffer& cmd ARGS_IF_RHIDEBUG(bool debugSync = false)) const;
    NODISCARD bool Present(const FVulkanDevice& device) const;

    const PVulkanDeviceQueue& PresentQueue() const { return _presentQueue; }

private:
    NODISCARD bool CreateSwapchain_(FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD bool CreateImages_(FVulkanResourceManager& resources);
    NODISCARD bool TearDownImages_(FVulkanResourceManager& resources);

    NODISCARD bool CreateSemaphores_(const FVulkanDevice& device);
    NODISCARD bool CreateFence_(const FVulkanDevice& device);
    NODISCARD bool ChoosePresentQueue_(const FVulkanFrameGraph& fg);

    NODISCARD bool IsImageAcquired_() const;

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
