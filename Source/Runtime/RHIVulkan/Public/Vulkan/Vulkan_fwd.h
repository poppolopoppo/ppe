﻿#pragma once

#include "RHIVulkan_fwd.h"

#ifdef RHI_VULKAN

#include "Vulkan/VulkanIncludes_fwd.h"

namespace PPE {
namespace RHI {

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// VulkanDevice.h
//----------------------------------------------------------------------------
using FVulkanShaderModule = VkShaderModule;
class FVulkanDevice;
//----------------------------------------------------------------------------
// VulkanFrameGraph.h
//----------------------------------------------------------------------------
FWD_REFPTR(VulkanFrameGraph);
//----------------------------------------------------------------------------
// VulkanInstance.h
//----------------------------------------------------------------------------
using FVulkanWindowHandle = void*;
using FVulkanWindowSurface = VkSurfaceKHR;
class FVulkanInstance;
//----------------------------------------------------------------------------
class FVulkanInstance;
//----------------------------------------------------------------------------
// VulkanMemoryAllocator.h
//----------------------------------------------------------------------------
using FVulkanDeviceMemory = VkDeviceMemory;
//----------------------------------------------------------------------------
enum class EVulkanMemoryTypeFlags : u32;
struct FVulkanMemoryBlock;
class FVulkanMemoryAllocator;
//----------------------------------------------------------------------------
// VulkanSwapChain.h
//----------------------------------------------------------------------------
class FVulkanSwapchain;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN