#pragma once

#include "Vulkan/VulkanCommon.h"

#include "Vulkan/Common/VulkanAPI.h"

#include "Container/HashSet.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "IO/ConstChar.h"
#include "Thread/CriticalSection.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanDeviceQueueInfo {
    VkQueue Handle{VK_NULL_HANDLE};
    EVulkanQueueFamily FamilyIndex{Default};
    VkQueueFlags FamilyFlags{0};
    float Priority{0.0f};
    uint3 MinImageTransferGranularity;

#if USE_PPE_RHIDEBUG
    FVulkanDebugName DebugName;
#endif
};
//----------------------------------------------------------------------------
struct FVulkanDeviceQueue : FVulkanDeviceQueueInfo {
    // use when call vkQueueSubmit, vkQueueWaitIdle, vkQueueBindSparse, vkQueuePresentKHR
    mutable FCriticalSection Barrier;

    FVulkanDeviceQueue() = default;
    FVulkanDeviceQueue(FVulkanDeviceQueue&& rvalue) NOEXCEPT
    :   FVulkanDeviceQueueInfo(std::move(rvalue))
    {
        rvalue.Handle = VK_NULL_HANDLE;
    }
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDevice final : public FVulkanDeviceFunctions {
public:
    using FQueues = TFixedSizeStack<FVulkanDeviceQueue, 16>;
    using FInstanceLayers = VECTOR(RHIDevice, VkLayerProperties);
    using FExtensionSet = HASHSET(RHIDevice, FConstChar);

    struct FDeviceInfo {
        VkPhysicalDeviceProperties Properties;
        VkPhysicalDeviceFeatures Features;
        VkPhysicalDeviceMemoryProperties MemoryProperties;
        VkPhysicalDeviceMeshShaderFeaturesNV MeshShaderFeatures;
        VkPhysicalDeviceMeshShaderPropertiesNV MeshShaderProperties;
        VkPhysicalDeviceShadingRateImageFeaturesNV ShadingRateImageFeatures;
        VkPhysicalDeviceShadingRateImagePropertiesNV ShadingRateImageProperties;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingProperties;
    };

    FVulkanDevice(
        VkInstance vkInstance,
        VkPhysicalDevice vkPhysicalDevice,
        VkDevice vkDevice,
        TMemoryView<const FVulkanDeviceQueueInfo> queues );
    ~FVulkanDevice();

    VkDevice vkDevice() const { return _vkDevice; }
    VkPhysicalDevice vkPhysicalDevice () const { return _vkPhysicalDevice; }
    VkInstance vkInstance() const { return _vkInstance; }
    EShaderLangFormat vkVersion() const { return _vkVersion; }
    TMemoryView<const FVulkanDeviceQueue> vkQueues() const { return _vkQueues.MakeConstView(); }

    bool EnableDebugUtils() const { return _enableDebugUtils; }
    bool EnableMeshShaderNV() const { return _enableMeshShaderNV; }
    bool EnableShadingRateImageNV() const { return _enableShadingRateImageNV; }
    bool EnableRayTracingKHR() const { return _enableRayTracingKHR; }
    bool SampleMirrorClamp() const { return _samplerMirrorClamp; }

    EVulkanQueueFamilyMask AvailableQueues() const { return _availableQueues; }
    EResourceState GraphicsShaderStages() const { return _graphicsShaderStages; }
    VkPipelineStageFlags AllWritableStages() const { return _allWritableStages; }
    VkPipelineStageFlags AllReadableStages() const { return _allReadableStages; }

    bool HasExtension(FStringView name) const NOEXCEPT;
    bool HasDeviceExtension(FStringView name) const NOEXCEPT;

#if USE_PPE_RHIDEBUG
    bool SetObjectName(u64 id, FConstChar name, VkObjectType type) const;
#endif

private:
    void SetupInstanceExtensions_();
    void SetupDeviceExtensions_();

    VkInstance _vkInstance;
    VkPhysicalDevice _vkPhysicalDevice;
    VkDevice _vkDevice;
    EShaderLangFormat _vkVersion;
    FQueues _vkQueues;

    EVulkanQueueFamilyMask _availableQueues;
    EResourceState _graphicsShaderStages;
    VkPipelineStageFlags _allWritableStages;
    VkPipelineStageFlags _allReadableStages;

    bool _enableDebugUtils : 1;
    bool _enableMeshShaderNV : 1;
    bool _enableShadingRateImageNV : 1;
    bool _enableRayTracingKHR : 1;
    bool _samplerMirrorClamp : 1;

    FDeviceInfo _deviceInfo;

    FExtensionSet _instanceExtensions;
    FExtensionSet _deviceExtensions;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
