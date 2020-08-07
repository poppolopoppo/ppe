#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericPhysicalDeviceFlags : u32 {
    None        = 0,
    Swapchain   = 1<<0,
    Graphics    = 1<<1,
    Compute     = 1<<2,
    Discrete    = 1<<3,

    Default     = Swapchain|Graphics|Compute,
};
ENUM_FLAGS(EGenericPhysicalDeviceFlags);
//----------------------------------------------------------------------------
struct PPE_RHI_API FGenericInstance : Meta::FNonCopyableNorMovable {
public: // must be defined by every RHI:
    static void Start() = delete;
    static void Shutdown() = delete;

    using FWindowHandle = FGenericWindowHandle;
    using FWindowSurface = FGenericWindowSurface;

    static FWindowSurface CreateWindowSurface(FWindowHandle hwnd) = delete;
    static void DestroyWindowSurface(FWindowSurface surface) = delete;

    using EPhysicalDeviceFlags = EGenericPhysicalDeviceFlags;

    static FGenericDevice* CreateLogicalDevice(
        EPhysicalDeviceFlags deviceFlags,
        FWindowSurface surfaceIFN ) = delete;
    static void DestroyLogicalDevice(FGenericDevice* pLogicalDevice) = delete;

public: // shared by each instance
    static bool GHeadless;
    static bool GEnableHDR;
#if USE_PPE_RHIDEBUG
    static bool GEnableDebug;
#else
    static constexpr bool GEnableDebug = false;
#endif

    static void ParseOptions();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
