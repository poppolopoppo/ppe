#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "HAL/TargetRHI.h"

#include "Vulkan/Common/VulkanAPI.h"
#include "Misc/DynamicLibrary.h"
#include "Thread/CriticalSection.h"

#define USE_PPE_VULKAN_DEBUGLAYER (USE_PPE_RHIDEBUG)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanInstance : public FVulkanInstanceFunctions {
public:
    FVulkanInstance() = default;

    static bool Create(FVulkanInstance* pInstance, ERHIFeature features);
    static void Destroy(FVulkanInstance* pInstance);

    using FWindowHandle = FVulkanWindowHandle;
    using FWindowSurface = FVulkanWindowSurface;

    ETargetRHI TargetRHI() const NOEXCEPT;

    FWindowSurface CreateWindowSurface(FWindowHandle hwnd);
    void DestroyWindowSurface(FWindowSurface surface);

    FVulkanDevice* CreateLogicalDevice(
        ERHIFeature features,
        FWindowSurface surfaceIFN );
    void DestroyLogicalDevice(FVulkanDevice* pLogicalDevice);

    const VkAllocationCallbacks* vkAllocator() const { return &_vkAllocator; }

private:
    FCriticalSection _barrier;

    VkInstance _vkInstance{ VK_NULL_HANDLE };
    VkAllocationCallbacks _vkAllocator{};

    ERHIFeature _features;

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
