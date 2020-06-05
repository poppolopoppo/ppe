#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "HAL/Generic/GenericRHIInstance.h"

#include "Meta/Optional.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_RHI_API FVulkanInstance : public FGenericInstance {
    public: // must be defined by every RHI:
        static void Start();
    static void Shutdown();

    using FWindowHandle = FVulkanWindowHandle;
    using FWindowSurface = FVulkanWindowSurface;

    static FWindowSurface CreateWindowSurface(FWindowHandle hwnd);
    static void DestroyWindowSurface(FWindowSurface surface);

    using EPhysicalDeviceFlags = EVulkanPhysicalDeviceFlags;

    static FVulkanDevice* CreateLogicalDevice(
        EPhysicalDeviceFlags deviceFlags,
        FWindowSurface surfaceIFN );
    static void DestroyLogicalDevice(FVulkanDevice* pLogicalDevice);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
