#pragma once

#include "RHI_fwd.h"

#ifdef EXPORT_PPE_RUNTIME_RHIVULKAN
#   define PPE_RHIVULKAN_API DLL_EXPORT
#else
#   define PPE_RHIVULKAN_API DLL_IMPORT
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanInstance;
class FVulkanDevice;
class FVulkanFrameGraph;
class FVulkanSwapchain;
class FVulkanMemoryAllocator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
