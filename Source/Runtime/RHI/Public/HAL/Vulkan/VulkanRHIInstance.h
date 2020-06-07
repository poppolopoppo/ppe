#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

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

public: // vulkan specific
    static FVulkanAllocationCallbacks Allocator() NOEXCEPT;

public: // shared by every RHI
    using FGenericInstance::GHeadless;
    using FGenericInstance::GEnableHDR;
#if USE_PPE_RHIDEBUG
    using FGenericInstance::GEnableDebug;
#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
