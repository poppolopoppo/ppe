#pragma once

#include "Vulkan/VulkanCommon.h"

//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkPipelineStageFlagBits);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkPipelineStageFlagBits);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkDependencyFlagBits);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkDependencyFlagBits);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkAccessFlagBits);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkAccessFlagBits);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageLayout);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageLayout);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageAspectFlagBits);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageAspectFlagBits);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkFilter);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkFilter);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkColorSpaceKHR);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkColorSpaceKHR);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkPresentModeKHR);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkPresentModeKHR);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkSurfaceTransformFlagBitsKHR);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkSurfaceTransformFlagBitsKHR);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkCompositeAlphaFlagBitsKHR);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkCompositeAlphaFlagBitsKHR);
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageUsageFlagBits);
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageUsageFlagBits);
//----------------------------------------------------------------------------
