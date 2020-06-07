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
    const u322 Extent() const { return _extent; }
    const u32 NumImages() const { return checked_cast<u32>(_images.size()); }

public: // vulkan specific:
    FVulkanSwapChain(
        FVulkanSwapChainHandle swapChainHandle,
        u322 extent,
        const FVulkanSurfaceFormat& surfaceFormat ) NOEXCEPT;

    ~FVulkanSwapChain();

    FVulkanSwapChainHandle Handle() const { return _handle; }

    TMemoryView<const FVulkanImageHandle> Images() const { return _images.MakeView(); }
    TMemoryView<const FVulkanImageViewHandle> ImageViews() const { return _imageViews.MakeView(); }

private:
    FVulkanSwapChainHandle _handle;
    u322 _extent;
    FVulkanSurfaceFormat _surfaceFormat;

    VECTORINSITU(RHIDevice, FVulkanImageHandle, 4) _images;
    VECTORINSITU(RHIDevice, FVulkanImageViewHandle, 4) _imageViews;

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
