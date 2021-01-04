#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIDevice.h"

#include "HAL/Vulkan/VulkanAPI.h"
#include "HAL/Vulkan/VulkanDebug.h"
#include "HAL/Vulkan/VulkanRHIMemoryAllocator.h"
#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

#include "Container/Vector.h"
#include "Memory/UniquePtr.h"
#include "Meta/StronglyTyped.h"
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
class PPE_RHI_API FVulkanDevice : public FGenericDevice, public FVulkanDeviceFunctions {
public:
    friend class FVulkanInstance;

    FVulkanDevice(
        const FVulkanInstance& instance,
        VkPhysicalDevice vkPhysicalDevice,
        TArray<EVulkanPresentMode>&& presentModes,
        TArray<FVulkanSurfaceFormat>&& surfaceFormats ) NOEXCEPT;

    NODISCARD bool SetupDevice(
        VkDevice vDevice,
        FVulkanDeviceQueueInfo graphicsQueue,
        FVulkanDeviceQueueInfo presentQueue,
        FVulkanDeviceQueueInfo asyncComputeQueue,
        FVulkanDeviceQueueInfo transferQueue );

    void TearDownDevice();

public: // must be implemented:
    ~FVulkanDevice();

    const TMemoryView<const EVulkanPresentMode> PresentModes() const { return _presentModes.MakeView(); }
    const TMemoryView<const FVulkanSurfaceFormat> SurfaceFormats() const { return _surfaceFormats.MakeView(); }

    const FVulkanSwapChain* SwapChain() const { return _swapChain.get(); }

    void CreateSwapChain(
        FVulkanWindowSurface surface,
        EVulkanPresentMode present,
        const FVulkanSurfaceFormat& surfaceFormat );
    void DestroySwapChain();

    VkShaderModule CreateShaderModule(const FRawMemoryConst& code);
    void DestroyShaderModule(VkShaderModule shaderModule);

    VkDescriptorSetLayout CreateDescriptorSetLayout(const FVulkanDescriptorSetLayout& desc);
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout setLayout);

    VkPipelineLayout CreatePipelineLayout(const FVulkanPipelineLayout& desc);
    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout);

public: // vulkan specific:
    VkDevice vkDevice() const { return _vkDevice; }
    VkPhysicalDevice vkPhysicalDevice() const { return _vkPhysicalDevice; }

    VkQueue vkGraphicsQueue() const { return _vkGraphicsQueue; }
    VkQueue vkPresentQueue() const { return _vkPresentQueue; }
    VkQueue vkAsyncComputeQueue() const { return _vkAsyncComputeQueue; }
    VkQueue vkTransferQueue() const { return _vkTransferQueue; }

    const FVulkanInstance& Instance() const { return _instance;  }

    FVulkanMemoryAllocator& DeviceMemory() { return _deviceMemory; }
    const FVulkanMemoryAllocator& DeviceMemory() const { return _deviceMemory; }

    const VkAllocationCallbacks* vkAllocator() const;

#if USE_PPE_RHIDEBUG
    const FVulkanDebug& Debug() const { return _debug; }
#endif

private:
    FCriticalSection _barrier;

    VkDevice _vkDevice;
    VkPhysicalDevice _vkPhysicalDevice;
    VkAllocationCallbacks _vkAllocator;

    VkQueue _vkGraphicsQueue;
    VkQueue _vkPresentQueue;
    VkQueue _vkAsyncComputeQueue;
    VkQueue _vkTransferQueue;

    const FVulkanInstance& _instance;
    FVulkanMemoryAllocator _deviceMemory;
    TUniquePtr<FVulkanSwapChain> _swapChain;

    const TArray<EVulkanPresentMode> _presentModes;
    const TArray<FVulkanSurfaceFormat> _surfaceFormats;

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
