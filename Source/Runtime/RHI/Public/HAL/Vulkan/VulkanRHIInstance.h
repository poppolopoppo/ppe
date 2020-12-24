#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIInstance.h"
#include "HAL/Vulkan/VulkanAPI.h"
#include "Misc/DynamicLibrary.h"
#include "Thread/CriticalSection.h"

#define USE_PPE_VULKAN_DEBUGLAYER (USE_PPE_RHIDEBUG)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanInstance : public FGenericInstance, public FVulkanInstanceFunctions {
public: // must be defined by every RHI:

    FVulkanInstance() = default;

    static bool Create(FVulkanInstance* pInstance);
    static void Destroy(FVulkanInstance* pInstance);

    using FWindowHandle = FVulkanWindowHandle;
    using FWindowSurface = FVulkanWindowSurface;

    ETargetRHI TargetRHI() const NOEXCEPT;

    FWindowSurface CreateWindowSurface(FWindowHandle hwnd);
    void DestroyWindowSurface(FWindowSurface surface);

    using EPhysicalDeviceFlags = EVulkanPhysicalDeviceFlags;

    FVulkanDevice* CreateLogicalDevice(
        EPhysicalDeviceFlags deviceFlags,
        FWindowSurface surfaceIFN );
    void DestroyLogicalDevice(FVulkanDevice* pLogicalDevice);

public: // shared by every RHI
    using FGenericInstance::GHeadless;
    using FGenericInstance::GEnableHDR;
#if USE_PPE_RHIDEBUG
    using FGenericInstance::GEnableDebug;
#endif

    using FGenericInstance::ParseOptions;

public: // Vulkan specific
    const VkAllocationCallbacks* vkAllocator() const { return &_vkAllocator; }

private:
    FCriticalSection _barrier;

    VkInstance _vkInstance{ VK_NULL_HANDLE };
    VkAllocationCallbacks _vkAllocator{};
#if USE_PPE_VULKAN_DEBUGLAYER
    VkDebugUtilsMessengerEXT _vkDebugMessenger{ VK_NULL_HANDLE };
#endif

    FDynamicLibrary _sharedLib;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
