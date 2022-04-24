
#include "stdafx.h"

#include "Vulkan/Instance/VulkanDevice.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"

#if USE_PPE_LOGGER
#   include "IO/FormatHelpers.h"
#   include "IO/StringBuilder.h"
#endif

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

    LOG(RHI, Warning, L"vulkan: missing support for API v{0}.{1}, fallback to v1.2", major, minor);
    return EShaderLangFormat::Vulkan_120;
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
,   _caps{}
,   _flags{}
,   _enabled{}
,   _instanceExtensions(info.RequiredInstanceExtensions | info.OptionalInstanceExtensions)
,   _deviceExtensions(info.RequiredDeviceExtensions | info.OptionalDeviceExtensions) {
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

    LOG(RHI, Info, L"finished setup for new vulkan device:{0}",
        Fmt::Formator<wchar_t>([this](FWTextWriter& log) {
            log << Eol << FTextFormat::BoolAlpha
            << Tab << L"VendorName:               " << vkVendorId() << Eol
            << Tab << L"DeviceName:               " << MakeCStringView(Properties().deviceName) << Eol
            << Tab << L"ApiVersion:               " << VK_VERSION_MAJOR( Properties().apiVersion ) << L'.'
                                                    << VK_VERSION_MINOR( Properties().apiVersion ) << L'.'
                                                    << VK_VERSION_PATCH( Properties().apiVersion ) << Eol
            << Tab << L"DriverVersion:            " << VK_VERSION_MAJOR( Properties().driverVersion ) << L'.'
                                                    << VK_VERSION_PATCH( Properties().driverVersion ) << Eol
            << Tab << L">---------- 1.1 ----------" << Eol
            << Tab << L"BindMemory2:              " << Enabled().BindMemory2 << Eol
            << Tab << L"DedicatedAllocation:      " << Enabled().DedicatedAllocation << Eol
            << Tab << L"DescriptorUpdateTemplate: " << Enabled().DescriptorUpdateTemplate << Eol
            << Tab << L"ImageViewUsage:           " << Enabled().ImageViewUsage << Eol
            << Tab << L"CommandPoolTrim:          " << Enabled().CommandPoolTrim << Eol
            << Tab << L"DispatchBase:             " << Enabled().DispatchBase << Eol
            << Tab << L"Array2DCompatible:        " << Enabled().Array2DCompatible << Eol
            << Tab << L"BlockTexelView:           " << Enabled().BlockTexelView << Eol
            << Tab << L"Maintenance3:             " << Enabled().Maintenance3 << Eol
            << Tab << L">---------- 1.2 ----------" << Eol
            << Tab << L"SamplerMirrorClamp:       " << Enabled().SamplerMirrorClamp << Eol
            << Tab << L"ShaderAtomicInt64:        " << Enabled().ShaderAtomicInt64 << Eol
            << Tab << L"Float16Arithmetic:        " << Enabled().Float16Arithmetic << Eol
            << Tab << L"BufferAddress:            " << Enabled().BufferAddress << Eol
            << Tab << L"DescriptorIndexing:       " << Enabled().DescriptorIndexing << Eol
            << Tab << L"RenderPass2:              " << Enabled().RenderPass2 << Eol
            << Tab << L"DepthStencilResolve:      " << Enabled().DepthStencilResolve << Eol
            << Tab << L"DrawIndirectCount:        " << Enabled().DrawIndirectCount << Eol
            << Tab << L"Spirv14:                  " << Enabled().Spirv14 << Eol
            << Tab << L"MemoryModel:              " << Enabled().MemoryModel << Eol
            << Tab << L"SamplerFilterMinmax       " << Enabled().SamplerFilterMinmax << Eol
            << Tab << L">---------- Ext ----------" << Eol
            << Tab << L"Surface:                  " << Enabled().Surface << Eol
            << Tab << L"SurfaceCaps2:             " << Enabled().SurfaceCaps2 << Eol
            << Tab << L"Swapchain:                " << Enabled().Swapchain << Eol
            << Tab << L"Debug_utils:              " << Enabled().DebugUtils << Eol
            << Tab << L"MeshShaderNV:             " << Enabled().MeshShaderNV << Eol
            << Tab << L"RayTracingNV:             " << Enabled().RayTracingNV << Eol
            << Tab << L"ShadingRateImageNV:       " << Enabled().ShadingRateImageNV << Eol
            << Tab << L"InlineUniformBlock:       " << Enabled().InlineUniformBlock << Eol
            << Tab << L"ShaderClock:              " << Enabled().ShaderClock << Eol
            << Tab << L"TimelineSemaphore:        " << Enabled().TimelineSemaphore << Eol
            << Tab << L"PushDescriptor:           " << Enabled().PushDescriptor << Eol
            << Tab << L"Robustness2:              " << Enabled().Robustness2 << Eol
            << Tab << L"ShaderStencilExport:      " << Enabled().ShaderStencilExport << Eol
            << Tab << L"ExtendedDynamicState:     " << Enabled().ExtendedDynamicState << Eol
            << Tab << L"RayTracingKHR:            " << Enabled().RayTracingKHR << Eol
            << Tab << L"--------------------------";
        }));

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
#if USE_PPE_RHIDEBUG
FVulkanDevice::~FVulkanDevice() {
    LOG(RHI, Info, L"destroying vulkan device...");
}
#endif
//----------------------------------------------------------------------------
const FVulkanDeviceQueue& FVulkanDevice::DeviceQueue(EVulkanQueueFamily familyIndex) const NOEXCEPT {
    const auto ptr = _vkQueues.MakeView().Any([familyIndex](const FVulkanDeviceQueue& q) NOEXCEPT {
        return (q.FamilyIndex == familyIndex);
    });
    Assert(ptr);
    return *ptr;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHITASKNAME
bool FVulkanDevice::SetObjectName(FVulkanExternalObject id, FConstChar name, VkObjectType type) const {
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
#ifdef VK_KHR_descriptor_update_template
    _enabled.DescriptorUpdateTemplate = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_descriptor_update_template));
#endif
#ifdef VK_KHR_maintenance3
    _enabled.Maintenance3 = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_maintenance3));
#endif
#ifdef VK_KHR_maintenance2
    _enabled.ImageViewUsage = _enabled.BlockTexelView = (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanDeviceExtension::KHR_maintenance2));
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
#ifdef VK_KHR_shader_atomic_int64
    _enabled.ShaderAtomicInt64 = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_shader_atomic_int64));
#endif
#ifdef VK_KHR_shader_float16_int8
    _enabled.Int8Arithmetic = _enabled.Float16Arithmetic = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_shader_float16_int8));
#endif
#ifdef VK_KHR_buffer_device_address
    _enabled.BufferAddress = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_buffer_device_address));
#endif
#ifdef VK_KHR_spirv_1_4
    _enabled.Spirv14 = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_spirv_1_4));
#endif
#ifdef VK_KHR_vulkan_memory_model
    _enabled.MemoryModel = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_vulkan_memory_model));
#endif
#ifdef VK_EXT_sampler_filter_minmax
    _enabled.SamplerFilterMinmax = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::EXT_sampler_filter_minmax));
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
#ifdef VK_NV_shader_image_footprint
    _enabled.ImageFootprintNV = HasExtension(EVulkanDeviceExtension::NV_shader_image_footprint);
#endif
#ifdef VK_EXT_inline_uniform_block
    _enabled.InlineUniformBlock = HasExtension(EVulkanDeviceExtension::EXT_inline_uniform_block) ;
#endif
#ifdef VK_KHR_shader_clock
    _enabled.ShaderClock = HasExtension(EVulkanDeviceExtension::KHR_shader_clock);
#endif
#ifdef VK_KHR_timeline_semaphore
    _enabled.TimelineSemaphore = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_timeline_semaphore));
#endif
#ifdef VK_KHR_push_descriptor
    _enabled.PushDescriptor = HasExtension(EVulkanDeviceExtension::KHR_push_descriptor);
#endif
#ifdef VK_KHR_push_descriptor
    _enabled.PushDescriptor = HasExtension(EVulkanDeviceExtension::KHR_push_descriptor);
#endif
#ifdef VK_EXT_shader_stencil_export
    _enabled.ShaderStencilExport = HasExtension(EVulkanDeviceExtension::EXT_shader_stencil_export);
#endif
#ifdef VK_EXT_extended_dynamic_state
    _enabled.ExtendedDynamicState = HasExtension(EVulkanDeviceExtension::EXT_extended_dynamic_state);
#endif
#ifdef VK_KHR_depth_stencil_resolve
    _enabled.DepthStencilResolve = (_vkVersion >= EShaderLangFormat::Vulkan_120 or HasExtension(EVulkanDeviceExtension::KHR_depth_stencil_resolve));
#endif
#ifdef VK_EXT_memory_budget
    _enabled.MemoryBudget = HasExtension(EVulkanDeviceExtension::EXT_memory_budget);
#endif
#ifdef VK_EXT_robustness2
    _enabled.Robustness2 = HasExtension(EVulkanDeviceExtension::EXT_robustness_2);
#endif

    if (_vkVersion >= EShaderLangFormat::Vulkan_110 or HasExtension(EVulkanInstanceExtension::KHR_get_physical_device_properties_2)) {

        // Features

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        features2.features.samplerAnisotropy = VK_TRUE;

        void** pNextFeature = &features2.pNext;
        Unused(pNextFeature);

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
#ifdef VK_NV_shader_image_footprint
        if (_enabled.ImageFootprintNV)
            enableFeature(&_caps.ShaderImageFootprintFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV);
#endif
#ifdef VK_KHR_shader_clock
        if (_enabled.ShaderClock)
            enableFeature(&_caps.ShaderClockFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR);
#endif
#ifdef VK_KHR_shader_float16_int8
        VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shader_float16_int8_feat = {};
        if (_enabled.Float16Arithmetic or _enabled.Int8Arithmetic or _vkVersion >= EShaderLangFormat::Vulkan_120)
            enableFeature(&shader_float16_int8_feat, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR);
#endif
#ifdef VK_KHR_timeline_semaphore
        VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_sem_feat = {};
        if (_enabled.TimelineSemaphore or _vkVersion >= EShaderLangFormat::Vulkan_120)
            enableFeature(&timeline_sem_feat, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR);
#endif
#ifdef VK_KHR_buffer_device_address
        if (_enabled.BufferAddress or _vkVersion >= EShaderLangFormat::Vulkan_120)
            enableFeature(&_caps.BufferDeviceAddress, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR);
#endif
#ifdef VK_KHR_shader_atomic_int64
        if (_enabled.ShaderAtomicInt64 or _vkVersion >= EShaderLangFormat::Vulkan_120)
            enableFeature(&_caps.ShaderAtomicInt64, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR);
#endif
#ifdef VK_KHR_vulkan_memory_model
        if (_enabled.MemoryModel)
            enableFeature(&_caps.MemoryModel, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR);
#endif
#ifdef VK_EXT_descriptor_indexing
        if (_enabled.DescriptorIndexing)
            enableFeature(&_caps.DescriptorIndexingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT);
#endif
#ifdef VK_EXT_robustness2
        if (_enabled.Robustness2)
            enableFeature(&_caps.Robustness2Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT);
#endif
#ifdef VK_EXT_extended_dynamic_state
        if (_enabled.ExtendedDynamicState)
            enableFeature(&_caps.ExtendedDynamicStateFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
        if (_enabled.RayTracingKHR)
            enableFeature(&_caps.RayTracingFeaturesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
#endif

        vkGetPhysicalDeviceFeatures2(_vkPhysicalDevice, &features2);

#ifdef VK_NV_mesh_shader
        _enabled.MeshShaderNV &= (_caps.MeshShaderFeatures.meshShader or _caps.MeshShaderFeatures.taskShader);
#endif
#ifdef VK_NV_shading_rate_image
        _enabled.ShadingRateImageNV &= (_caps.ShadingRateImageFeatures.shadingRateImage == VK_TRUE);
#endif
#ifdef VK_NV_shader_image_footprint
        _enabled.ImageFootprintNV &= (_caps.ShaderImageFootprintFeatures.imageFootprint == VK_TRUE);
#endif
#ifdef VK_KHR_shader_clock
        _enabled.ShaderClock &= !!(_caps.ShaderClockFeatures.shaderDeviceClock | _caps.ShaderClockFeatures.shaderSubgroupClock);
#endif
#ifdef VK_KHR_shader_float16_int8
        _enabled.Float16Arithmetic &= (shader_float16_int8_feat.shaderFloat16 == VK_TRUE);
        _enabled.Int8Arithmetic &= (shader_float16_int8_feat.shaderInt8 == VK_TRUE);
#endif
#ifdef VK_KHR_timeline_semaphore
        _enabled.TimelineSemaphore &= (timeline_sem_feat.timelineSemaphore == VK_TRUE);
#endif
#ifdef VK_KHR_buffer_device_address
        _enabled.BufferAddress &= (_caps.BufferDeviceAddress.bufferDeviceAddress == VK_TRUE);
#endif
#ifdef VK_KHR_shader_atomic_int64
        _enabled.ShaderAtomicInt64 &= (_caps.ShaderAtomicInt64.shaderBufferInt64Atomics == VK_TRUE);
#endif
#ifdef VK_KHR_vulkan_memory_model
        _enabled.MemoryModel &= (_caps.MemoryModel.vulkanMemoryModel == VK_TRUE);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
        _enabled.RayTracingKHR &= (_caps.RayTracingFeaturesKHR.rayTracingPipeline == VK_TRUE);
#endif

        // Properties

        VkPhysicalDeviceProperties2 properties2{};
        properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

        void** pNextProperties = &properties2.pNext;
        Unused(pNextProperties);

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
            enableProperties(&_caps.RayTracingPropertiesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV);
#endif
#ifdef VK_KHR_ray_tracing_pipeline
        if (_enabled.RayTracingKHR)
            enableProperties(&_caps.RayTracingPropertiesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
#endif
#ifdef VK_KHR_timeline_semaphore
        if (_enabled.TimelineSemaphore)
            enableProperties(&_caps.TimelineSemaphoreProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR);
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
#ifdef VK_VERSION_1_1
        if (_vkVersion >= EShaderLangFormat::Vulkan_110)
            enableProperties(&_caps.Subgroup, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES);
#endif
#ifdef VK_EXT_descriptor_indexing
        if (_enabled.DescriptorIndexing)
            enableProperties(&_caps.DescriptorIndexingProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES);
#endif
#ifdef VK_EXT_robustness2
        if (_enabled.Robustness2)
            enableProperties(&_caps.Robustness2Properties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT);
#endif
#ifdef VK_KHR_maintenance3
        if (_enabled.Maintenance3)
            enableProperties(&_caps.Maintenance3Properties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES_KHR);
#endif
#ifdef VK_EXT_sampler_filter_minmax
        if (_enabled.SamplerFilterMinmax)
            enableProperties(&_caps.SamplerFilerMinmaxProperties, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT);
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
