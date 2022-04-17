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
    mutable FVulkanDebugName DebugName;
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
    TFixedSizeStack<FVulkanDeviceQueueInfo, MaxQueueFamilies> Queues;
    FVulkanInstanceExtensionSet RequiredInstanceExtensions{ Meta::ForceInit };
    FVulkanInstanceExtensionSet OptionalInstanceExtensions{ Meta::ForceInit };
    FVulkanDeviceExtensionSet RequiredDeviceExtensions{ Meta::ForceInit };
    FVulkanDeviceExtensionSet OptionalDeviceExtensions{ Meta::ForceInit };
    VkInstance vkInstance{ VK_NULL_HANDLE };
    VkPhysicalDevice vkPhysicalDevice{ VK_NULL_HANDLE };
    VkDevice vkDevice{ VK_NULL_HANDLE };
    const VkAllocationCallbacks* pAllocator{ nullptr };
    size_t MaxStagingBufferMemory{ ~0_b };
    size_t StagingBufferSize{ 0_b };
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
        bool DescriptorUpdateTemplate   : 1;
        bool ImageViewUsage             : 1;
        bool CommandPoolTrim            : 1;
        bool DispatchBase               : 1;
        bool Array2DCompatible          : 1;
        bool BlockTexelView             : 1;
        bool Maintenance3               : 1;
        // vulkan 1.2 core
        bool SamplerMirrorClamp         : 1;
        bool ShaderAtomicInt64          : 1;
        bool Float16Arithmetic          : 1;
        bool Int8Arithmetic             : 1;
        bool BufferAddress              : 1;
        bool DescriptorIndexing         : 1;
        bool RenderPass2                : 1;
        bool DepthStencilResolve        : 1;
        bool DrawIndirectCount          : 1;
        bool Spirv14                    : 1;
        bool MemoryModel                : 1;
        bool SamplerFilterMinmax        : 1;
        // window extensions
        bool Surface                    : 1;
        bool SurfaceCaps2               : 1;
        bool Swapchain                  : 1;
        // extensions
        bool DebugUtils                 : 1;
        bool MemoryBudget               : 1;
        bool MeshShaderNV               : 1;
        bool RayTracingNV               : 1;
        bool ImageFootprintNV           : 1;
        bool ShadingRateImageNV         : 1;
        bool InlineUniformBlock         : 1;
        bool ShaderClock                : 1;
        bool TimelineSemaphore          : 1;
        bool PushDescriptor             : 1;
        bool ShaderStencilExport        : 1;
        bool ExtendedDynamicState       : 1;
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
    #ifdef VK_NV_shader_image_footprint
        VkPhysicalDeviceShaderImageFootprintFeaturesNV ShaderImageFootprintFeatures;
    #endif
    #ifdef VK_NV_ray_tracing
        VkPhysicalDeviceRayTracingPropertiesNV RayTracingNVProperties;
    #endif
    #ifdef VK_KHR_timeline_semaphore
        VkPhysicalDeviceTimelineSemaphorePropertiesKHR TimelineSemaphoreProperties;
    #endif
    #ifdef VK_KHR_shader_clock
        VkPhysicalDeviceShaderClockFeaturesKHR ShaderClockFeatures;
    #endif
    #ifdef VK_KHR_buffer_device_address
        VkPhysicalDeviceBufferDeviceAddressFeaturesKHR BufferDeviceAddress;
    #endif
    #ifdef VK_KHR_shader_atomic_int64
        VkPhysicalDeviceShaderAtomicInt64FeaturesKHR ShaderAtomicInt64;
    #endif
    #ifdef VK_KHR_vulkan_memory_model
        VkPhysicalDeviceVulkanMemoryModelFeaturesKHR MemoryModel;
    #endif
    #ifdef VK_KHR_depth_stencil_resolve
        VkPhysicalDeviceDepthStencilResolvePropertiesKHR DepthStencilResolve;
    #endif
    #ifdef VK_KHR_maintenance3
        VkPhysicalDeviceMaintenance3Properties Maintenance3Properties;
    #endif
    #ifdef VK_EXT_descriptor_indexing
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT DescriptorIndexingFeatures;
        VkPhysicalDeviceDescriptorIndexingPropertiesEXT DescriptorIndexingProperties;
    #endif
    #ifdef VK_EXT_robustness2
        VkPhysicalDeviceRobustness2FeaturesEXT Robustness2Features;
        VkPhysicalDeviceRobustness2PropertiesEXT Robustness2Properties;
    #endif
    #ifdef VK_EXT_extended_dynamic_state
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ExtendedDynamicStateFeatures;
    #endif
    #ifdef VK_EXT_sampler_filter_minmax
        VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT SamplerFilerMinmaxProperties;
    #endif
    #ifdef VK_NV_ray_tracing
        VkPhysicalDeviceRayTracingPropertiesNV RayTracingPropertiesNV;
    #endif
    #ifdef VK_KHR_ray_tracing_pipeline
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingFeaturesKHR;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPropertiesKHR;
    #endif
    #ifdef VK_VERSION_1_2
            VkPhysicalDeviceVulkan11Properties Properties110;
            VkPhysicalDeviceVulkan12Properties Properties120;
    #endif
    #ifdef VK_VERSION_1_1
            VkPhysicalDeviceSubgroupProperties Subgroup;
    #endif
    };

    explicit FVulkanDevice(const FVulkanDeviceInfo& info);
#if USE_PPE_RHIDEBUG
    ~FVulkanDevice();
#endif

    VkDevice vkDevice() const { return _vkDevice; }
    VkPhysicalDevice vkPhysicalDevice () const { return _vkPhysicalDevice; }
    VkInstance vkInstance() const { return _vkInstance; }
    EVulkanVendor vkVendorId() const { return static_cast<EVulkanVendor>(_caps.Properties.vendorID); }
    EShaderLangFormat vkVersion() const { return _vkVersion; }
    TMemoryView<const FVulkanDeviceQueue> vkQueues() const { return _vkQueues.MakeConstView(); }
    const VkAllocationCallbacks* vkAllocator() const { return std::addressof(_vkAllocator); };

    const FEnabledFeatures& Enabled() const NOEXCEPT { return _enabled; }
    const FDeviceFlags& Flags() const NOEXCEPT { return _flags; }

    EVulkanQueueFamilyMask AvailableQueues() const { return _flags.AvailableQueues; }
    EResourceState GraphicsShaderStages() const { return _flags.GraphicsShaderStages; }
    VkPipelineStageFlagBits AllWritableStages() const { return _flags.AllWritableStages; }
    VkPipelineStageFlagBits AllReadableStages() const { return _flags.AllReadableStages; }

    bool HasExtension(EVulkanInstanceExtension ext) const NOEXCEPT { return (_instanceExtensions & ext); }
    bool HasExtension(EVulkanDeviceExtension ext) const NOEXCEPT { return (_deviceExtensions & ext); }

    bool AnyExtension(const FVulkanInstanceExtensionSet& exts) const NOEXCEPT { return exts.Contains(_instanceExtensions & exts); }
    bool AnyExtension(const FVulkanDeviceExtensionSet& exts) const NOEXCEPT { return exts.Contains(_deviceExtensions & exts); }

    const FDeviceCaps& Capabilities() const { return _caps; }
    const VkPhysicalDeviceFeatures& Features() const { return _caps.Features; }
    const VkPhysicalDeviceLimits& Limits() const { return _caps.Properties.limits; }
    const VkPhysicalDeviceProperties& Properties() const { return _caps.Properties; }

    const FVulkanDeviceQueue& DeviceQueue(EVulkanQueueFamily familyIndex) const NOEXCEPT;
    const FVulkanDeviceQueue& DeviceQueue(u32 queueFamilyIndex) const NOEXCEPT {
        return DeviceQueue(static_cast<EVulkanQueueFamily>(queueFamilyIndex));
    }

#if USE_PPE_RHITASKNAME
    bool SetObjectName(FVulkanExternalObject id, FConstChar name, VkObjectType type) const;
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
