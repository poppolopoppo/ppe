#pragma once

#include "Vulkan/Vulkan_fwd.h"

#ifdef RHI_VULKAN

#include "Vulkan/VulkanAPI.h"
#include "Vulkan/VulkanDebug.h"
#include "Vulkan/VulkanMemoryAllocator.h"

#include "Container/Vector.h"
#include "Memory/UniquePtr.h"
#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanPresentMode : u32 {
    Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
    Fifo = VK_PRESENT_MODE_FIFO_KHR,
    RelaxedFifo = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    Mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
};
//----------------------------------------------------------------------------
struct FVulkanDeviceQueueInfo {
    u32 FamilyIndex{ UINT32_MAX };
    u32 FamilyQueueIndex{ UINT32_MAX };

    bool IsInvalid() const { return ((UINT32_MAX == FamilyIndex) & (UINT32_MAX == FamilyQueueIndex)); }
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDevice : public FVulkanDeviceFunctions {
public:
    friend class FVulkanInstance;

    FVulkanDevice(
        FVulkanInstance& instance,
        VkPhysicalDevice vkPhysicalDevice,
        EVulkanVendor vendorId,
        TArray<VkPresentModeKHR>&& presentModes,
        TArray<VkSurfaceFormatKHR>&& surfaceFormats ) NOEXCEPT;
    ~FVulkanDevice();

    FVulkanInstance& Instance() const { return _instance;  }

    VkPhysicalDevice vkPhysicalDevice() const { return _vkPhysicalDevice; }
    const VkAllocationCallbacks* vkAllocator() const;

    EVulkanVendor VendorId() const { return _vendorId; }

    NODISCARD bool SetupDevice(
        VkDevice vkDevice,
        FVulkanDeviceQueueInfo graphicsQueue,
        FVulkanDeviceQueueInfo presentQueue,
        FVulkanDeviceQueueInfo asyncComputeQueue,
        FVulkanDeviceQueueInfo transferQueue );

    void TearDownDevice();

    const TMemoryView<const VkPresentModeKHR> PresentModes() const { return _presentModes.MakeView(); }
    const TMemoryView<const VkSurfaceFormatKHR> SurfaceFormats() const { return _surfaceFormats.MakeView(); }

    const FVulkanSwapchain* SwapChain() const { return _swapchain.get(); }

    void CreateSwapChain(
        FVulkanWindowSurface surface,
        VkPresentModeKHR present,
        const VkSurfaceFormatKHR& surfaceFormat );
    void DestroySwapChain();

    VkDevice vkDevice() const { return _vkDevice; }

    VkQueue vkGraphicsQueue() const { return _vkGraphicsQueue; }
    VkQueue vkPresentQueue() const { return _vkPresentQueue; }
    VkQueue vkAsyncComputeQueue() const { return _vkAsyncComputeQueue; }
    VkQueue vkTransferQueue() const { return _vkTransferQueue; }

    FVulkanMemoryAllocator& DeviceMemory() { return _deviceMemory; }
    const FVulkanMemoryAllocator& DeviceMemory() const { return _deviceMemory; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebug& Debug() const { return _debug; }
#endif

private:
    FCriticalSection _barrier;

    EVulkanVendor _vendorId;
    VkPhysicalDevice _vkPhysicalDevice;
    VkAllocationCallbacks _vkAllocator;

    VkDevice _vkDevice;

    VkQueue _vkGraphicsQueue;
    VkQueue _vkPresentQueue;
    VkQueue _vkAsyncComputeQueue;
    VkQueue _vkTransferQueue;

    FVulkanInstance& _instance;
    FVulkanMemoryAllocator _deviceMemory;
    TUniquePtr<FVulkanSwapchain> _swapchain;

    const TArray<VkPresentModeKHR> _presentModes;
    const TArray<VkSurfaceFormatKHR> _surfaceFormats;

#if USE_PPE_RHIDEBUG
    FVulkanDebug _debug;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
