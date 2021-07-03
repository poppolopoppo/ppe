#pragma once

#include "Vulkan/VulkanCommon.h"

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
    VkQueue Handle{ VK_NULL_HANDLE };
    EVulkanQueueFamily FamilyIndex{ Default };
    u32 QueueIndex{ UMax };
    VkQueueFlags FamilyFlags{ 0 };
    float Priority{ 0.0f };
    uint3 MinImageTransferGranularity{ Meta::ForceInit };
    bool SupportsPresent{ false };

#if USE_PPE_RHIDEBUG
    FVulkanDebugName DebugName;
#endif
};
//----------------------------------------------------------------------------
struct FVulkanDeviceQueue : FVulkanDeviceQueueInfo {
    // use when call vkQueueSubmit, vkQueueWaitIdle, vkQueueBindSparse, vkQueuePresentKHR
    mutable FCriticalSection Barrier;

    FVulkanDeviceQueue() = default;
    explicit FVulkanDeviceQueue(const FVulkanDeviceQueueInfo& info) : FVulkanDeviceQueueInfo(info) {}
    FVulkanDeviceQueue(FVulkanDeviceQueue&& rvalue) NOEXCEPT
    :   FVulkanDeviceQueueInfo(std::move(rvalue))
    {
        rvalue.Handle = VK_NULL_HANDLE;
    }
};
//----------------------------------------------------------------------------
struct FVulkanDeviceInfo {
    FVulkanDeviceAPI API{};
    VkInstance vkInstance{ VK_NULL_HANDLE };
    VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
    VkDevice vkDevice{ VK_NULL_HANDLE };
    TFixedSizeStack<FVulkanDeviceQueueInfo, MaxQueueFamilies> Queues;
    FVulkanInstanceExtensionSet InstanceExtensions{ Meta::ForceInit };
    FVulkanDeviceExtensionSet DeviceExtensions{ Meta::ForceInit };
    const VkAllocationCallbacks* pAllocator{ nullptr };
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanDevice final : public FVulkanDeviceFunctions, Meta::FNonCopyable {
public:
    using FQueues = TFixedSizeStack<FVulkanDeviceQueue, 16>;
    using FInstanceLayers = VECTOR(RHIDevice, VkLayerProperties);

    struct FEnabledFeatures {
        FEnabledFeatures() = default;
        // vulkan 1.1 core
        bool BindMemory2                : 1;
        bool DedicatedAllocation        : 1;
        bool CommandPoolTrim            : 1;
        bool DispatchBase               : 1;
        bool Array2DCompatible          : 1;
        bool BlockTexelView             : 1;
        // vulkan 1.2 core
        bool SamplerMirrorClamp         : 1;
        bool DescriptorIndexing         : 1;
        bool RenderPass2                : 1;
        bool DepthStencilResolve        : 1;
        bool DrawIndirectCount          : 1;
        // window extensions
        bool Surface                    : 1;
        bool SurfaceCaps2               : 1;
        bool Swapchain                  : 1;
        // extensions
        bool DebugUtils                 : 1;
        bool MemoryBudget               : 1;
        bool MeshShaderNV               : 1;
        bool RayTracingNV               : 1;
        bool ShadingRateImageNV         : 1;
        bool Robustness2                : 1;
        bool RayTracingKHR              : 1;
    };

    struct FDeviceFlags {
        EVulkanQueueFamilyMask AvailableQueues{ Default };
        EResourceState GraphicsShaderStages{ Default };
        VkPipelineStageFlagBits AllWritableStages{ VK_PIPELINE_STAGE_NONE_KHR };
        VkPipelineStageFlagBits AllReadableStages{ VK_PIPELINE_STAGE_NONE_KHR };
        VkImageCreateFlagBits ImageCreateFlags{ static_cast<VkImageCreateFlagBits>(0) };
    };

    struct FDeviceCaps {
        VkPhysicalDeviceProperties Properties;
        VkPhysicalDeviceFeatures Features;
        VkPhysicalDeviceMemoryProperties MemoryProperties;
    #ifdef VK_NV_mesh_shader
        VkPhysicalDeviceMeshShaderFeaturesNV MeshShaderFeatures;
        VkPhysicalDeviceMeshShaderPropertiesNV MeshShaderProperties;
    #endif
    #ifdef VK_NV_shading_rate_image
        VkPhysicalDeviceShadingRateImageFeaturesNV ShadingRateImageFeatures;
        VkPhysicalDeviceShadingRateImagePropertiesNV ShadingRateImageProperties;
    #endif
    #ifdef VK_NV_ray_tracing
        VkPhysicalDeviceRayTracingPropertiesNV RayTracingNVProperties;
    #endif
    #ifdef VK_KHR_depth_stencil_resolve
        VkPhysicalDeviceDepthStencilResolvePropertiesKHR DepthStencilResolve;
    #endif
    #ifdef VK_EXT_descriptor_indexing
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT DescriptorIndexingFeatures;
        VkPhysicalDeviceDescriptorIndexingPropertiesEXT DescriptorIndexingProperties;
    #endif
    #ifdef VK_VERSION_1_2
        VkPhysicalDeviceVulkan11Properties Properties110;
        VkPhysicalDeviceVulkan12Properties Properties120;
    #endif
    #ifdef VK_EXT_memory_budget
        VkPhysicalDeviceMemoryBudgetPropertiesEXT MemoryBudget;
    #endif
    #ifdef VK_EXT_robustness2
        VkPhysicalDeviceRobustness2FeaturesEXT Robustness2Features;
        VkPhysicalDeviceRobustness2PropertiesEXT Robustness2Properties;
    #endif
    #ifdef VK_KHR_ray_tracing_pipeline
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingFeatures;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingProperties;
    #endif
    };

    explicit FVulkanDevice(const FVulkanDeviceInfo& info);
    ~FVulkanDevice();

    VkDevice vkDevice() const { return _vkDevice; }
    VkPhysicalDevice vkPhysicalDevice () const { return _vkPhysicalDevice; }
    VkInstance vkInstance() const { return _vkInstance; }
    EShaderLangFormat vkVersion() const { return _vkVersion; }
    TMemoryView<const FVulkanDeviceQueue> vkQueues() const { return _vkQueues.MakeConstView(); }
    const VkAllocationCallbacks* vkAllocator() const { return std::addressof(_vkAllocator); };

    const FEnabledFeatures& Enabled() const NOEXCEPT { return _enabled; }
    const FDeviceFlags& Flags() const NOEXCEPT { return _flags; }

    EVulkanQueueFamilyMask AvailableQueues() const { return _flags.AvailableQueues; }
    EResourceState GraphicsShaderStages() const { return _flags.GraphicsShaderStages; }
    VkPipelineStageFlags AllWritableStages() const { return _flags.AllWritableStages; }
    VkPipelineStageFlags AllReadableStages() const { return _flags.AllReadableStages; }

    bool HasExtension(EVulkanInstanceExtension ext) const NOEXCEPT { return (_instanceExtensions & ext); }
    bool HasExtension(EVulkanDeviceExtension ext) const NOEXCEPT { return (_deviceExtensions & ext); }

    const FDeviceCaps& Capabilities() const { return _caps; }
    const VkPhysicalDeviceFeatures& Features() const { return _caps.Features; }
    const VkPhysicalDeviceLimits& Limits() const { return _caps.Properties.limits; }
    const VkPhysicalDeviceProperties& Properties() const { return _caps.Properties; }

#if USE_PPE_RHITASKNAME
    bool SetObjectName(u64 id, FConstChar name, VkObjectType type) const;
#endif

private:
    void SetupDeviceFeatures_();
    void SetupDeviceFlags_();

    VkInstance _vkInstance;
    VkPhysicalDevice _vkPhysicalDevice;
    VkDevice _vkDevice;
    EShaderLangFormat _vkVersion;
    FQueues _vkQueues;
    VkAllocationCallbacks _vkAllocator;

    FDeviceCaps _caps;
    FDeviceFlags _flags;
    FEnabledFeatures _enabled;

    FVulkanInstanceExtensionSet _instanceExtensions;
    FVulkanDeviceExtensionSet _deviceExtensions;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
