#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#include "HAL/Generic/GenericRHIInstance.h"

#include "Meta/Optional.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FVulkanRHIPhysicalDevice = FGenericRHIPhysicalDevice;
using EVulkanRHIPhysicalDeviceFlags = EGenericRHIPhysicalDeviceFlags;
//----------------------------------------------------------------------------
struct PPE_RHI_API FVulkanRHIInstance : public FGenericRHIInstance {
public: // must be defined by every RHI:
    static void Start();
    static void Shutdown();

    using FPhysicalDevice = FVulkanRHIPhysicalDevice;
    using EPhysicalDeviceFlags = EVulkanRHIPhysicalDeviceFlags;

    static FPhysicalDevice PickPhysicalDevice(EPhysicalDeviceFlags flags);

    static FVulkanRHIDevice* CreateLogicalDevice(
        FPhysicalDevice physicalDevice,
        EPhysicalDeviceFlags flags );
    static void DestroyLogicalDevice(FVulkanRHIDevice** pLogicalDevice);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
