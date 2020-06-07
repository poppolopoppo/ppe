#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"

#ifndef RHI_VULKAN
#    error "invalid RHI !"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanInterop {
    static CONSTEXPR VkColorSpaceKHR ColorSpace_to_VkColorSpaceKHR(EVulkanColorSpace colorSpace) NOEXCEPT;
    static CONSTEXPR EVulkanColorSpace VkColorSpaceKHR_to_ColorSpace(VkColorSpaceKHR vkColorSpace) NOEXCEPT;

    static CONSTEXPR VkFormat PixelFormat_to_VkFormat(EVulkanPixelFormat pixelFormat) NOEXCEPT;
    static CONSTEXPR EVulkanPixelFormat VkFormat_to_PixelFormat(VkFormat vkFormat) NOEXCEPT;

    static CONSTEXPR VkPresentModeKHR PresentMode_to_VkPresentModeKHR(EVulkanPresentMode presentMode) NOEXCEPT;
    static CONSTEXPR EVulkanPresentMode VkPresentModeKHR_to_PresentMode(VkPresentModeKHR vkPresentMode) NOEXCEPT;
};
//----------------------------------------------------------------------------
/////////////////////////////////////////////////////
////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#include "HAL/Vulkan/VulkanInterop-inl.h"
