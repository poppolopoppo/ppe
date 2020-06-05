#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#ifdef RHI_VULKAN

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanInstance;
using EVulkanPhysicalDeviceFlags = EGenericPhysicalDeviceFlags;
using FVulkanWindowHandle = FGenericWindowHandle;
using FVulkanWindowSurface = FGenericWindowSurface;
//----------------------------------------------------------------------------
class FVulkanDevice;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
