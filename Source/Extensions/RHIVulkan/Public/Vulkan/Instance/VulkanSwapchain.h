#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHI/SwapchainDesc.h"

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
#if USE_PPE_RHIDEBUG
    ~FVulkanSwapchain();
#endif

    NODISCARD bool Construct(FVulkanFrameGraph& fg, const FSwapchainDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    void TearDown(FVulkanResourceManager& resources);

    NODISCARD bool Acquire(FRawImageID* pImageId, FVulkanCommandBuffer& cmd ARGS_IF_RHIDEBUG(bool debugSync = false)) const;
    NODISCARD bool Present(const FVulkanDevice& device) const;

    auto Read() const { return _data.LockShared(); }

    const FSwapchainDesc& Desc() const { return Read()->Desc; }
    const PVulkanDeviceQueue& PresentQueue() const { return Read()->PresentQueue; }

    VkFormat ColorFormat() const { return Read()->ColorFormat; }
    VkColorSpaceKHR ColorSpace() const { return Read()->ColorSpace; }
    u32 MinImageCount() const { return Read()->MinImageCount; }
    VkSurfaceTransformFlagBitsKHR PreTransform() const { return Read()->PreTransform; }
    VkPresentModeKHR PresentMode() const { return Read()->PresentMode; }
    VkCompositeAlphaFlagBitsKHR CompositeAlpha() const { return Read()->CompositeAlpha; }
    VkImageUsageFlagBits ColorImageUsage() const { return Read()->ColorImageUsage; }

private:
    struct FInternalData_ {
        FSwapchainDesc Desc;

        PVulkanDeviceQueue PresentQueue{};
        FSwapchainImages ImageIds;
        uint2 SurfaceSize;

        VkSwapchainKHR vkSwapchain{ VK_NULL_HANDLE };
        VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
        mutable u32 CurrentImageIndex{ UMax };

        TStaticArray<VkSemaphore, 2> ImageAvailable;
        TStaticArray<VkSemaphore, 2> RenderFinished;
        VkFence vkFence{ VK_NULL_HANDLE };
        mutable u32 SemaphoreId : 1;

        VkFormat ColorFormat{ VK_FORMAT_UNDEFINED };
        VkColorSpaceKHR ColorSpace{ VK_COLOR_SPACE_MAX_ENUM_KHR };
        u32 MinImageCount{ 2 };
        VkSurfaceTransformFlagBitsKHR PreTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
        VkPresentModeKHR PresentMode{ VK_PRESENT_MODE_FIFO_KHR };
        VkCompositeAlphaFlagBitsKHR CompositeAlpha{ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR };
        VkImageUsageFlagBits ColorImageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };

        EVulkanQueueFamilyMask QueueFamilyMask{ Default };

        NODISCARD bool CreateSwapchain_(FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FConstChar debugName));

        NODISCARD bool CreateImages_(FVulkanResourceManager& resources);
        void TearDownImages_(FVulkanResourceManager& resources);

        NODISCARD bool CreateSemaphores_(const FVulkanDevice& device);
        NODISCARD bool CreateFence_(const FVulkanDevice& device);
        NODISCARD bool ChoosePresentQueue_(const FVulkanFrameGraph& fg) NOEXCEPT;

        NODISCARD bool IsImageAcquired_() const NOEXCEPT;
    };

    mutable TThreadSafe<FInternalData_, EThreadBarrier::RWDataRaceCheck> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
