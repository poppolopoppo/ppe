#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIDevice.h"

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
        FVulkanDeviceHandle logicalDevice,
        FVulkanQueueHandle graphicsQueue,
        FVulkanQueueHandle presentQueue,
        FVulkanQueueHandle asyncComputeQueue,
        FVulkanQueueHandle transferQueue,
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

    FVulkanShaderModule CreateShaderModule(const FRawMemoryConst& code);
    void DestroyShaderModule(FVulkanShaderModule shaderModule);

    FVulkanDescriptorSetLayoutHandle CreateDescriptorSetLayout(const FVulkanDescriptorSetLayout& desc);
    void DestroyDescriptorSetLayout(FVulkanDescriptorSetLayoutHandle setLayout);

    FVulkanPipelineLayoutHandle CreatePipelineLayout(const FVulkanPipelineLayout& desc);
    void DestroyPipelineLayout(FVulkanPipelineLayoutHandle pipelineLayout);

public: // vulkan specific:
    FVulkanPhysicalDevice PhysicalDevice() const { return _physicalDevice; }
    FVulkanDeviceHandle LogicalDevice() const { return _logicalDevice; }

    FVulkanQueueHandle GraphicsQueue() const { return _graphicsQueue; }
    FVulkanQueueHandle PresentQueue() const { return _presentQueue; }
    FVulkanQueueHandle AsyncComputeQueue() const { return _asyncComputeQueue; }
    FVulkanQueueHandle TransferQueue() const { return _transferQueue; }

    FVulkanAllocationCallbacks Allocator() const { return _allocator; }

    FVulkanMemoryAllocator& DeviceMemory() { return _deviceMemory; }
    const FVulkanMemoryAllocator& DeviceMemory() const { return _deviceMemory; }

private:
    std::recursive_mutex _barrier;

    FVulkanAllocationCallbacks _allocator;

    FVulkanPhysicalDevice _physicalDevice;
    FVulkanDeviceHandle _logicalDevice;

    TUniquePtr<FVulkanSwapChain> _swapChain;

    FVulkanQueueHandle _graphicsQueue;
    FVulkanQueueHandle _presentQueue;
    FVulkanQueueHandle _asyncComputeQueue;
    FVulkanQueueHandle _transferQueue;

    const FPresentModeList _presentModes;
    const FSurfaceFormatList _surfaceFormats;

    FVulkanMemoryAllocator _deviceMemory;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
