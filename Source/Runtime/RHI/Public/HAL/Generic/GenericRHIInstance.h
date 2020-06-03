#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Meta/Enum.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericRHIPhysicalDevice);
//----------------------------------------------------------------------------
enum class EGenericRHIPhysicalDeviceFlags {
    None        = 0,
    Swapchain   = 1<<0,
    Graphics    = 1<<1,
    Compute     = 1<<2,
    Discrete    = 1<<3,

    Default     = Swapchain|Graphics|Compute,
};
ENUM_FLAGS(EGenericRHIPhysicalDeviceFlags);
//----------------------------------------------------------------------------
struct PPE_RHI_API FGenericRHIInstance : Meta::FNonCopyableNorMovable {
public: // must be defined by every RHI:
    static void Start() = delete;
    static void Shutdown() = delete;

    using FPhysicalDevice = FGenericRHIPhysicalDevice;
    using EPhysicalDeviceFlags = EGenericRHIPhysicalDeviceFlags;

    static FPhysicalDevice PickPhysicalDevice(EPhysicalDeviceFlags flags) = delete;

    static FGenericRHIDevice* CreateLogicalDevice(
        FPhysicalDevice physicalDevice,
        EPhysicalDeviceFlags flags ) = delete;
    static void DestroyLogicalDevice(FGenericRHIDevice** pLogicalDevice) = delete;

public: // shared by each instance
    static bool GHeadless;
#if USE_PPE_RHIDEBUG
    static bool GDebugEnabled;
#endif
    static void ParseOptions();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
