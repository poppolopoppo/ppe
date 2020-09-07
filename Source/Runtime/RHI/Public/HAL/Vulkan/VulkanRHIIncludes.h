#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "vulkan-external.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same_v<RHI::VkFlags, ::VkFlags>);
STATIC_ASSERT(std::is_same_v<RHI::VkBool32, ::VkBool32>);
STATIC_ASSERT(std::is_same_v<RHI::VkDeviceSize, ::VkDeviceSize>);
STATIC_ASSERT(std::is_same_v<RHI::VkSampleMask, ::VkSampleMask>);
//----------------------------------------------------------------------------
STATIC_ASSERT(std::is_same_v<RHI::VkCommandBuffer, ::VkCommandBuffer>);
#ifdef ARCH_X86
STATIC_ASSERT(std::is_constructible_v<RHI::VkImage, ::VkImage>);
STATIC_ASSERT(std::is_constructible_v<RHI::VkBuffer, ::VkBuffer>);
STATIC_ASSERT(std::is_constructible_v<RHI::VkFence, ::VkFence>);
STATIC_ASSERT(std::is_constructible_v<RHI::VkDeviceMemory, ::VkDeviceMemory>);
#else
STATIC_ASSERT(std::is_same_v<RHI::VkImage, ::VkImage>);
STATIC_ASSERT(std::is_same_v<RHI::VkBuffer, ::VkBuffer>);
STATIC_ASSERT(std::is_same_v<RHI::VkFence, ::VkFence>);
STATIC_ASSERT(std::is_same_v<RHI::VkDeviceMemory, ::VkDeviceMemory>);
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
