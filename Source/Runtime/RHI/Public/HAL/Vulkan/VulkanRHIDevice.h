#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIDevice.h"

#include "HAL/Vulkan/VulkanDebug.h"
#include "HAL/Vulkan/VulkanRHIMemoryAllocator.h"
#include "HAL/Vulkan/VulkanRHISurfaceFormat.h"

#include "Container/Vector.h"
#include "Memory/UniquePtr.h"
#include "Meta/StronglyTyped.h"

#include <mutex>

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanDevice : public FGenericDevice {
public:
    using FPresentModeList = VECTOR(RHIDevice, EVulkanPresentMode);
    using FSurfaceFormatList = VECTOR(RHIDevice, FVulkanSurfaceFormat);

protected:
    friend struct FVulkanInstance;

    FVulkanDevice(
        FVulkanAllocationCallbacks allocator,
        FVulkanPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkQueue asyncComputeQueue,
        VkQueue transferQueue,
        FPresentModeList&& presentModes,
        FSurfaceFormatList&& surfaceFormats ) NOEXCEPT;

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
    VkPhysicalDevice PhysicalDevice() const { return _physicalDevice; }
    VkDevice LogicalDevice() const { return _logicalDevice; }

    VkQueue GraphicsQueue() const { return _graphicsQueue; }
    VkQueue PresentQueue() const { return _presentQueue; }
    VkQueue AsyncComputeQueue() const { return _asyncComputeQueue; }
    VkQueue TransferQueue() const { return _transferQueue; }

    FVulkanAllocationCallbacks Allocator() const { return _allocator; }

    FVulkanMemoryAllocator& DeviceMemory() { return _deviceMemory; }
    const FVulkanMemoryAllocator& DeviceMemory() const { return _deviceMemory; }

#if USE_PPE_RHIDEBUG
    const FVulkanDebug& Debug() const { return _debug; }
#endif

private:
    std::recursive_mutex _barrier;

    FVulkanAllocationCallbacks _allocator;

    VkPhysicalDevice _physicalDevice;
    VkDevice _logicalDevice;

    TUniquePtr<FVulkanSwapChain> _swapChain;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkQueue _asyncComputeQueue;
    VkQueue _transferQueue;

    const FPresentModeList _presentModes;
    const FSurfaceFormatList _surfaceFormats;

    FVulkanMemoryAllocator _deviceMemory;

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
