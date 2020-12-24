#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHISwapChain.h"

#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

#include "Container/Vector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanSwapChain : public FGenericSwapChain {
public: // must be implemented by each RHI:
    const FVulkanSurfaceFormat& SurfaceFormat() const { return _surfaceFormat; }
    const u322& Extent() const { return _extent; }
    u32 NumImages() const { return checked_cast<u32>(_images.size()); }

public: // vulkan specific:
    FVulkanSwapChain(
        VkSwapchainKHR vkSwapChain,
        u322 extent,
        const FVulkanSurfaceFormat& surfaceFormat ) NOEXCEPT;

    ~FVulkanSwapChain();

    VkSwapchainKHR Handle() const { return _vkSwapChain; }

    TMemoryView<const VkImage> Images() const { return _images.MakeView(); }
    TMemoryView<const VkImageView> ImageViews() const { return _imageViews.MakeView(); }

private:
    VkSwapchainKHR _vkSwapChain;
    u322 _extent;
    FVulkanSurfaceFormat _surfaceFormat;

    VECTORINSITU(RHIDevice, VkImage, 4) _images;
    VECTORINSITU(RHIDevice, VkImageView, 4) _imageViews;

    friend class FVulkanDevice;
    void InitializeSwapChain(const FVulkanDevice& device);
    void TearDownSwapChain(const FVulkanDevice& device);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
