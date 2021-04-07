#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "Container/Vector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanSwapchain : public FRefCountable {
public: // must be implemented by each RHI:
    const VkSurfaceFormatKHR& SurfaceFormat() const { return _surfaceFormat; }
    const u322& Extent() const { return _extent; }
    u32 NumImages() const { return checked_cast<u32>(_images.size()); }

public: // vulkan specific:
    FVulkanSwapchain(
        VkSwapchainKHR vkSwapChain,
        u322 extent,
        const VkSurfaceFormatKHR& surfaceFormat ) NOEXCEPT;

    ~FVulkanSwapchain();

    VkSwapchainKHR Handle() const { return _vkSwapChain; }

    TMemoryView<const VkImage> Images() const { return _images.MakeView(); }
    TMemoryView<const VkImageView> ImageViews() const { return _imageViews.MakeView(); }

private:
    VkSwapchainKHR _vkSwapChain;
    u322 _extent;
    VkSurfaceFormatKHR _surfaceFormat;

    VECTORINSITU(RHIDevice, VkImage, 4) _images;
    VECTORINSITU(RHIDevice, VkImageView, 4) _imageViews;

    friend class FVulkanDevice;
    void InitializeSwapchain(const FVulkanDevice& device);
    void TearDownSwapchain(const FVulkanDevice& device);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
