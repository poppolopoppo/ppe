
#include "stdafx.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static EShaderLangFormat VulkanApiToVersion_(u32 apiVersion) {
    const u32 major = VK_VERSION_MAJOR(apiVersion);
    const u32 minor = VK_VERSION_MINOR(apiVersion);

    if (1 == major) {
        if (0 == minor) return EShaderLangFormat::Vulkan_100;
        if (1 == minor) return EShaderLangFormat::Vulkan_110;
        if (2 == minor) return EShaderLangFormat::Vulkan_120;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDevice::FVulkanDevice(const FVulkanDeviceInfo& info)
:   FVulkanDeviceFunctions(&info.API)
,   _vkInstance(info.vkInstance)
,   _vkPhysicalDevice(info.vkPhysicalDevice)
,   _vkDevice(info.vkDevice)
,   _vkVersion(EShaderLangFormat::Unknown)
,   _vkAllocator(*info.pAllocator)
,   _instanceExtensions(info.InstanceExtensions)
,   _deviceExtensions(info.DeviceExtensions) {
    Assert_NoAssume(VK_NULL_HANDLE != _vkInstance);

    for (const FVulkanDeviceQueueInfo& q : info.Queues) {
        Assert_NoAssume(q.FamilyIndex < EVulkanQueueFamily::_Count);
        _flags.AvailableQueues |= q.FamilyIndex;
        _vkQueues.Push(q);
    }

    FPlatformMemory::Memzero(&_caps, sizeof(_caps));
    vkGetPhysicalDeviceFeatures(_vkPhysicalDevice, &_caps.Features);
    vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &_caps.Properties);
    vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_caps.MemoryProperties);

    _vkVersion = VulkanApiToVersion_(_caps.Properties.apiVersion);

    SetupDeviceFeatures_();
    SetupDeviceFlags_();

    // validate device limits
    AssertRelease(_caps.Properties.limits.maxPushConstantsSize >= MaxPushConstantsSize);
    AssertRelease(_caps.Properties.limits.maxVertexInputAttributes >= MaxVertexAttribs);
    AssertRelease(_caps.Properties.limits.maxVertexInputBindings >= MaxVertexBuffers);
    AssertRelease(_caps.Properties.limits.maxViewports >= MaxViewports);
    AssertRelease(_caps.Properties.limits.maxColorAttachments >= MaxColorBuffers);
    AssertRelease(_caps.Properties.limits.maxBoundDescriptorSets >= MaxDescriptorSets);
    AssertRelease(_caps.Properties.limits.maxDescriptorSetUniformBuffersDynamic +
                  _caps.Properties.limits.maxDescriptorSetStorageBuffersDynamic >= MaxBufferDynamicOffsets);
}
//----------------------------------------------------------------------------
FVulkanDevice::~FVulkanDevice() {

}
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
bool FVulkanDevice::SetObjectName(u64 id, FConstChar name, VkObjectType type) const {
    if (_enabled.DebugUtils && !!name && !!id) {
        VkDebugUtilsObjectNameInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.objectType = type;
        info.objectHandle = id;
        info.pObjectName = name.c_str();

        VK_CHECK( vkSetDebugUtilsObjectNameEXT(_vkDevice, &info) );
        return true;
    }

    return false;
}
#endif
//----------------------------------------------------------------------------
void FVulkanDevice::SetupDeviceFeatures_() {
#ifdef VK_KHR_surface
    _enabled.Surface = HasExtension(EVulkanInstanceExtension::KHR_surface);
#endif
#ifdef VK_KHR_get_surface_capabilities2
    _enabled.SurfaceCaps2 = HasExtension(EVulkanInstanceExtension::KHR_get_surface_capabilities_2);
#endif
#ifdef VK_KHR_swapchain
    _enabled.Swapchain = HasExtension( EVulkanDeviceExtension::KHR_swapchain );
#endif
#ifdef VK_EXT_debug_utils
    _enabled.DebugUtils = HasExtension(EVulkanInstanceExtension::EXT_debug_utils);
#endif
#if defined(VK_KHR_get_memory_requirements2) and defined(VK_KHR_bind_memory2)
    _enabled.BindMemory2 = (_vkVersion >= EShaderLangFormat::Vulkan_110 || (HasExtension(EVulkanDeviceExtension::KHR_get_memory_requirements_2) && HasExtension(EVulkanDeviceExtension::KHR_bind_memory_2)));
#endif
#ifdef VK_KHR_dedicated_allocation
    _enabled.DedicatedAllocation = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_dedicated_allocation));
#endif
#ifdef VK_KHR_maintenance2
    _enabled.BlockTexelView = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_maintenance2));
#endif
#ifdef VK_KHR_maintenance1
    const bool hasMaintenance1 = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_maintenance1));
    _enabled.CommandPoolTrim = hasMaintenance1;
    _enabled.Array2DCompatible = hasMaintenance1;
#endif
#ifdef VK_KHR_device_group
    _enabled.DispatchBase = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_device_group));
#endif
#ifdef VK_KHR_create_renderpass2
    _enabled.RenderPass2 = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_create_renderpass_2));
#endif
#ifdef VK_KHR_sampler_mirror_clamp_to_edge
    _enabled.SamplerMirrorClamp = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_sampler_mirror_clamp_to_edge));
#endif
#ifdef VK_KHR_draw_indirect_count
    _enabled.DrawIndirectCount = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_draw_indirect_count));
#endif
#ifdef VK_EXT_descriptor_indexing
    _enabled.DescriptorIndexing = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::EXT_descriptor_indexing));
#endif
#ifdef VK_NV_mesh_shader
    _enabled.MeshShaderNV = HasExtension(EVulkanDeviceExtension::NV_mesh_shader);
#endif
#ifdef VK_NV_ray_tracing
    _enabled.RayTracingNV = HasExtension(EVulkanDeviceExtension::NV_ray_tracing);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    _enabled.RayTracingKHR = HasExtension(EVulkanDeviceExtension::KHR_ray_tracing_pipeline);
#endif
#ifdef VK_NV_shading_rate_image
    _enabled.ShadingRateImageNV = HasExtension(EVulkanDeviceExtension::NV_shading_rate_image);
#endif
#ifdef VK_KHR_depth_stencil_resolve
    _enabled.DepthStencilResolve = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_depth_stencil_resolve));
#endif
#ifdef VK_EXT_robustness2
    _enabled.Robustness2 = HasExtension(EVulkanDeviceExtension::EXT_robustness_2);
#endif

    if (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanInstanceExtension::KHR_get_physical_device_properties_2)) {

        // Features

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        void** pNextFeature = &features2.pNext;
        UNUSED(pNextFeature);

        auto enableFeature = [&](auto* pFeature, VkStructureType sType) NOEXCEPT {
            *pNextFeature = pFeature;
            pNextFeature = &pFeature->pNext;
            pFeature->sType = sType;
        };

#ifdef VK_NV_mesh_shader
        if (_enabled.MeshShaderNV)
            enableFeature(&_caps.MeshShaderFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV);
#endif
#ifdef VK_NV_shading_rate_image
        if (_enabled.ShadingRateImageNV)
            enableFeature(&_caps.ShadingRateImageFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV);
#endif
#ifdef VK_EXT_descriptor_indexing
        if (_enabled.DescriptorIndexing)
            enableFeature(&_caps.DescriptorIndexingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT);
#endif
#ifdef VK_EXT_robustness2
        if (_enabled.Robustness2)
            enableFeature(&_caps.Robustness2Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
        if (_enabled.RayTracingKHR)
            enableFeature(&_caps.RayTracingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
#endif

        vkGetPhysicalDeviceFeatures2(_vkPhysicalDevice, &features2);

#ifdef VK_NV_mesh_shader
        _enabled.MeshShaderNV &= (_caps.MeshShaderFeatures.meshShader or _caps.MeshShaderFeatures.taskShader);
#endif
#ifdef VK_NV_shading_rate_image
        _enabled.ShadingRateImageNV &= (_caps.ShadingRateImageFeatures.shadingRateImage == VK_TRUE);
#endif
#ifdef VK_EXT_robustness2
        _enabled.Robustness2 &= (!!_caps.Robustness2Features.robustBufferAccess2 | !!_caps.Robustness2Features.nullDescriptor);
#endif

        // Properties

        VkPhysicalDeviceProperties2 properties2{};
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        void** pNextProperties = &properties2.pNext;
        UNUSED(pNextProperties);

        auto enableProperties = [&](auto* pProperties, VkStructureType sType) NOEXCEPT {
            *pNextProperties = pProperties;
            pNextProperties = &pProperties->pNext;
            pProperties->sType = sType;
        };

#ifdef VK_NV_mesh_shader
        if (_enabled.MeshShaderNV)
            enableProperties(&_caps.MeshShaderProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV);
#endif
#ifdef VK_NV_shading_rate_image
        if (_enabled.ShadingRateImageNV)
            enableProperties(&_caps.ShadingRateImageProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV);
#endif
#ifdef VK_NV_ray_tracing
        if (_enabled.RayTracingNV)
            enableProperties(&_caps.RayTracingNVProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
        if (_enabled.RayTracingKHR)
            enableProperties(&_caps.RayTracingProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
#endif
#ifdef VK_KHR_depth_stencil_resolve
        if (_enabled.DepthStencilResolve)
            enableProperties(&_caps.DepthStencilResolve, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES_KHR);
#endif
#ifdef VK_VERSION_1_2
        if (_vkVersion >= EShaderLangFormat::Vulkan_120)
            enableProperties(&_caps.Properties120, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES);
        if (_vkVersion >= EShaderLangFormat::Vulkan_110)
            enableProperties(&_caps.Properties110, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES);
#endif
#ifdef VK_EXT_descriptor_indexing
        if (_enabled.DescriptorIndexing)
            enableProperties(&_caps.DescriptorIndexingProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES);
#endif
#ifdef VK_EXT_robustness2
        if (_enabled.Robustness2)
            enableProperties(&_caps.Robustness2Properties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT);
#endif

        vkGetPhysicalDeviceProperties2(_vkPhysicalDevice, &properties2);
    }
}
//----------------------------------------------------------------------------
void FVulkanDevice::SetupDeviceFlags_() {
    // add shader stages
    _flags.GraphicsShaderStages = EResourceState::_VertexShader | EResourceState::_FragmentShader;

    if (_caps.Features.tessellationShader)
        _flags.GraphicsShaderStages |= (EResourceState::_TessControlShader | EResourceState::_TessEvaluationShader);

    if (_caps.Features.geometryShader)
        _flags.GraphicsShaderStages |= (EResourceState::_GeometryShader);

    if (_enabled.MeshShaderNV)
        _flags.GraphicsShaderStages |= (EResourceState::_MeshTaskShader | EResourceState::_MeshShader);

    // pipeline stages
    _flags.AllWritableStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    _flags.AllReadableStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

#ifdef VK_NV_ray_tracing
    if (_enabled.RayTracingNV)
        _flags.AllReadableStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV;
#endif
#ifdef VK_KHR_ray_tracing_pipeline
    if (_enabled.RayTracingKHR)
        _flags.AllReadableStages |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
#endif

    // image create flags
    _flags.ImageCreateFlags =
        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT |
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT |
        (_enabled.Array2DCompatible ? VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT : Zero) |
        (_enabled.BlockTexelView ? VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT : Zero);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
