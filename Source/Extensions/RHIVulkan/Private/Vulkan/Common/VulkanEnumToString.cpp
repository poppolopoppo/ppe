// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Common/VulkanEnumToString.h"

#include "Container/BitMask.h"
#include "IO/FormatHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkMemoryHeapFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkMemoryHeapFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_MEMORY_HEAP_DEVICE_LOCAL_BIT: oss << sep << STRING_LITERAL(_Char, "DEVICE_LOCAL"); break;
        case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT: oss << sep << STRING_LITERAL(_Char, "MULTI_INSTANCE"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkMemoryPropertyFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkMemoryPropertyFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: oss << sep << STRING_LITERAL(_Char, "DEVICE_LOCAL"); break;
        case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: oss << sep << STRING_LITERAL(_Char, "HOST_VISIBLE"); break;
        case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: oss << sep << STRING_LITERAL(_Char, "HOST_COHERENT"); break;
        case VK_MEMORY_PROPERTY_HOST_CACHED_BIT: oss << sep << STRING_LITERAL(_Char, "HOST_CACHED"); break;
        case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT: oss << sep << STRING_LITERAL(_Char, "LAZILY_ALLOCATED"); break;
        case VK_MEMORY_PROPERTY_PROTECTED_BIT: oss << sep << STRING_LITERAL(_Char, "PROTECTED"); break;
        case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD: oss << sep << STRING_LITERAL(_Char, "DEVICE_COHERENT_AMD"); break;
        case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD: oss << sep << STRING_LITERAL(_Char, "DEVICE_UNCACHED_AMD"); break;
        case VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV: oss << sep << STRING_LITERAL(_Char, "RDMA_CAPABLE_NV"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPhysicalDeviceType value) {
    switch (value) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return oss << STRING_LITERAL(_Char, "OTHER");
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return oss << STRING_LITERAL(_Char, "INTEGRATED_GPU");
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return oss << STRING_LITERAL(_Char, "DISCRETE_GPU");
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return oss << STRING_LITERAL(_Char, "VIRTUAL_GPU");
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return oss << STRING_LITERAL(_Char, "CPU");
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM: AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPipelineStageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkPipelineStageFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT: oss << sep << STRING_LITERAL(_Char, "TopOfPipe"); break;
        case VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT: oss << sep << STRING_LITERAL(_Char, "DrawIndirect"); break;
        case VK_PIPELINE_STAGE_VERTEX_INPUT_BIT: oss << sep << STRING_LITERAL(_Char, "VertexInput"); break;
        case VK_PIPELINE_STAGE_VERTEX_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "VertexShader"); break;
        case VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "TessellationControlShader"); break;
        case VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "TessellationEvaluationShader"); break;
        case VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "GeometryShader"); break;
        case VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "FragmentShader"); break;
        case VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT: oss << sep << STRING_LITERAL(_Char, "EarlyFragmentTests"); break;
        case VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT: oss << sep << STRING_LITERAL(_Char, "LateFragmentTests"); break;
        case VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT: oss << sep << STRING_LITERAL(_Char, "ColorAttachmentOutput"); break;
        case VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT: oss << sep << STRING_LITERAL(_Char, "ComputeShader"); break;
        case VK_PIPELINE_STAGE_TRANSFER_BIT: oss << sep << STRING_LITERAL(_Char, "Transfer"); break;
        case VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT: oss << sep << STRING_LITERAL(_Char, "BottomOfPipe"); break;
        case VK_PIPELINE_STAGE_HOST_BIT: oss << sep << STRING_LITERAL(_Char, "Host"); break;
        case VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT: oss << sep << STRING_LITERAL(_Char, "AllGraphics"); break;
        case VK_PIPELINE_STAGE_ALL_COMMANDS_BIT: oss << sep << STRING_LITERAL(_Char, "AllCommands"); break;
#ifdef VK_EXT_transform_feedback
        case VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "TransformFeedbackExt"); break;
#endif
#ifdef VK_EXT_conditional_rendering
        case VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "ConditionalRenderingExt"); break;
#endif
#ifdef VK_NV_shading_rate_image
        case VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV: oss << sep << STRING_LITERAL(_Char, "ShadingRateImageNV"); break;
#endif
#ifdef VK_NV_ray_tracing
        case VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV: oss << sep << STRING_LITERAL(_Char, "AccelerationStructureBuildNv"); break;
        case VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV: oss << sep << STRING_LITERAL(_Char, "RayTracingShaderNv"); break;
#endif
#ifdef VK_NV_mesh_shader
        case VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV: oss << sep << STRING_LITERAL(_Char, "TaskShaderNv"); break;
        case VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV: oss << sep << STRING_LITERAL(_Char, "MeshShaderNv"); break;
#endif
#ifdef VK_EXT_fragment_density_map
        case VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "FragmentDensityProcessExt"); break;
#endif
#ifdef VK_NV_device_generated_commands
        case VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessNv"); break;
#elif defined(VK_NVX_device_generated_commands)
        case VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessNvx"); break;
#endif
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkDependencyFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkDependencyFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_DEPENDENCY_BY_REGION_BIT: oss << sep << STRING_LITERAL(_Char, "ByRegion"); break;
        case VK_DEPENDENCY_DEVICE_GROUP_BIT: oss << sep << STRING_LITERAL(_Char, "DeviceGroup"); break;
        case VK_DEPENDENCY_VIEW_LOCAL_BIT: oss << sep << STRING_LITERAL(_Char, "ViewLocal"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkAccessFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkAccessFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_ACCESS_INDIRECT_COMMAND_READ_BIT: oss << sep << STRING_LITERAL(_Char, "IndirectCommandRead"); break;
        case VK_ACCESS_INDEX_READ_BIT: oss << sep << STRING_LITERAL(_Char, "IndexRead"); break;
        case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT: oss << sep << STRING_LITERAL(_Char, "VertexAttributeRead"); break;
        case VK_ACCESS_UNIFORM_READ_BIT: oss << sep << STRING_LITERAL(_Char, "UniformRead"); break;
        case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT: oss << sep << STRING_LITERAL(_Char, "InputAttachmentRead"); break;
        case VK_ACCESS_SHADER_READ_BIT: oss << sep << STRING_LITERAL(_Char, "ShaderRead"); break;
        case VK_ACCESS_SHADER_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "ShaderWrite"); break;
        case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT: oss << sep << STRING_LITERAL(_Char, "ColorAttachmentRead"); break;
        case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "ColorAttachmentWrite"); break;
        case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT: oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachmentRead"); break;
        case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachmentWrite"); break;
        case VK_ACCESS_TRANSFER_READ_BIT: oss << sep << STRING_LITERAL(_Char, "TransferRead"); break;
        case VK_ACCESS_TRANSFER_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "TransferWrite"); break;
        case VK_ACCESS_HOST_READ_BIT: oss << sep << STRING_LITERAL(_Char, "HostRead"); break;
        case VK_ACCESS_HOST_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "HostWrite"); break;
        case VK_ACCESS_MEMORY_READ_BIT: oss << sep << STRING_LITERAL(_Char, "MemoryRead"); break;
        case VK_ACCESS_MEMORY_WRITE_BIT: oss << sep << STRING_LITERAL(_Char, "MemoryWrite"); break;
#ifdef VK_EXT_transform_feedback
        case VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "TransformFeedbackWriteExt"); break;
        case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "TransformFeedbackCounterReadExt"); break;
        case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "TransformFeedbackCounterWriteExt"); break;
#endif
#ifdef VK_EXT_conditional_rendering
        case VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "ConditionalRenderingReadExt"); break;
#endif
#ifdef VK_EXT_blend_operation_advanced
        case VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "ColorAttachmentReadNoncoherentExt"); break;
#endif
#ifdef VK_NV_ray_tracing
        case VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AccelerationStructureReadKhr"); break;
        case VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AccelerationStructureWriteKhr"); break;
#endif
#ifdef VK_EXT_fragment_density_map
        case VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "FragmentDensityMapReadExt"); break;
#endif
#ifdef VK_NV_shading_rate_image
        case VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "FragmentShadingRateAttachmentReadKhr"); break;
#endif
#ifdef VK_NV_device_generated_commands
        case VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessReadNv"); break;
        case VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessWriteNv"); break;
#elif defined(VK_NVX_device_generated_commands)
        case VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NVX: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessReadNvx"); break;
        case VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NVX: oss << sep << STRING_LITERAL(_Char, "CommandPreprocessWriteNvx"); break;
#endif
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageLayout value) {
    switch (value) {
    case VK_IMAGE_LAYOUT_UNDEFINED: return oss << STRING_LITERAL(_Char, "Undefined");
    case VK_IMAGE_LAYOUT_GENERAL: return oss << STRING_LITERAL(_Char, "General");
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "ColorAttachmentOptimal");

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "ShaderReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return oss << STRING_LITERAL(_Char, "TransferSrcOptimal");
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return oss << STRING_LITERAL(_Char, "TransferDstOptimal");
    case VK_IMAGE_LAYOUT_PREINITIALIZED: return oss << STRING_LITERAL(_Char, "Preinitialized");

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthAttachmentOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "StencilAttachmentOptimal");
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "StencilReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return oss << STRING_LITERAL(_Char, "PresentSrcKhr");
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR: return oss << STRING_LITERAL(_Char, "SharedPresentKhr");

#ifdef VK_EXT_fragment_density_map
    case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return oss << STRING_LITERAL(_Char, "FragmentDensityMapOptimalExt");
#endif
#ifdef VK_NV_shading_rate_image
    //case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV: return oss << STRING_LITERAL(_Char, "ShadingRateAttachmentOptimalNV");
    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "FragmentShadingRateAttachmentOptimalKhr");
#endif
#ifdef VK_KHR_separate_depth_stencil_layouts
    //case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "DepthAttachmentOptimalKhr");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthStencilAttachmentOptimal");
    //case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "DepthReadOnlyKhr");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthReadOnlyStencilAttachmentOptimal");
    //case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "StencilAttachmentOptimalKhr");
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthAttachmentStencilReadOnlyOptimal");
    //case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "StencilReadOnlyKhr");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthStencilReadOnlyOptimal");
#endif
    case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "ReadOnlyOptimalKhr");
    case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "AttachmentOptimalKhr");
    case VK_IMAGE_LAYOUT_MAX_ENUM:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageAspectFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkImageAspectFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_IMAGE_ASPECT_COLOR_BIT: oss << sep << STRING_LITERAL(_Char, "Color"); break;
        case VK_IMAGE_ASPECT_DEPTH_BIT: oss << sep << STRING_LITERAL(_Char, "Depth"); break;
        case VK_IMAGE_ASPECT_STENCIL_BIT: oss << sep << STRING_LITERAL(_Char, "Stencil"); break;
        case VK_IMAGE_ASPECT_METADATA_BIT: oss << sep << STRING_LITERAL(_Char, "Metadata"); break;
        case VK_IMAGE_ASPECT_PLANE_0_BIT: oss << sep << STRING_LITERAL(_Char, "Plane0"); break;
        case VK_IMAGE_ASPECT_PLANE_1_BIT: oss << sep << STRING_LITERAL(_Char, "Plane1"); break;
        case VK_IMAGE_ASPECT_PLANE_2_BIT: oss << sep << STRING_LITERAL(_Char, "Plane2"); break;
        case VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "MemoryPlane0Ext"); break;
        case VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "MemoryPlane1Ext"); break;
        case VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "MemoryPlane2Ext"); break;
        case VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "MemoryPlane3Ext"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkFilter value) {
    switch (value) {
    case VK_FILTER_NEAREST: return oss << STRING_LITERAL(_Char, "Nearest");
    case VK_FILTER_LINEAR: return oss << STRING_LITERAL(_Char, "Linear");
    case VK_FILTER_CUBIC_IMG: return oss << STRING_LITERAL(_Char, "CubicImg");
    case VK_FILTER_MAX_ENUM:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkColorSpaceKHR value) {
    switch (value) {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return oss << STRING_LITERAL(_Char, "SrgbNonlinearKhr");
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "DisplayP3NonlinearExt");
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "ExtendedSrgbLinearExt");
    case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "DisplayP3LinearExt");
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "DciP3NonlinearExt");
    case VK_COLOR_SPACE_BT709_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "Bt709LinearExt");
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "Bt709NonlinearExt");
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "Bt2020LinearExt");
    case VK_COLOR_SPACE_HDR10_ST2084_EXT: return oss << STRING_LITERAL(_Char, "Hdr10St2084Ext");
    case VK_COLOR_SPACE_DOLBYVISION_EXT: return oss << STRING_LITERAL(_Char, "DolbyvisionExt");
    case VK_COLOR_SPACE_HDR10_HLG_EXT: return oss << STRING_LITERAL(_Char, "Hdr10HlgExt");
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "AdobergbLinearExt");
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "AdobergbNonlinearExt");
    case VK_COLOR_SPACE_PASS_THROUGH_EXT: return oss << STRING_LITERAL(_Char, "PassThroughExt");
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "ExtendedSrgbNonlinearExt");
    case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: return oss << STRING_LITERAL(_Char, "DisplayNativeAmd");
    case VK_COLOR_SPACE_MAX_ENUM_KHR:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPresentModeKHR value) {
    switch (value) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: return oss << STRING_LITERAL(_Char, "Immediate");
    case VK_PRESENT_MODE_MAILBOX_KHR: return oss << STRING_LITERAL(_Char, "Mailbox");
    case VK_PRESENT_MODE_FIFO_KHR: return oss << STRING_LITERAL(_Char, "Fifo");
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return oss << STRING_LITERAL(_Char, "FifoRelaxed");
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR: return oss << STRING_LITERAL(_Char, "SharedDemandRefresh");
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR: return oss << STRING_LITERAL(_Char, "SharedContinuousRefresh");
    case VK_PRESENT_MODE_MAX_ENUM_KHR:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkSurfaceTransformFlagBitsKHR value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeBitMask(static_cast<std::underlying_type_t<VkSurfaceTransformFlagBitsKHR>>(value)); mask; ) {
        const auto it = static_cast<VkSurfaceTransformFlagBitsKHR>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "Identity"); break;
        case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "Rotate90"); break;
        case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "Rotate180"); break;
        case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "Rotate270"); break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "HorizontalMirror"); break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate90"); break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate180"); break;
        case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate270"); break;
        case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "Inherit"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkCompositeAlphaFlagBitsKHR value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeBitMask(static_cast<std::underlying_type_t<VkCompositeAlphaFlagBitsKHR>>(value)); mask; ) {
        const auto it = static_cast<VkCompositeAlphaFlagBitsKHR>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AlphaOpaque"); break;
        case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AlphaPreMultiplied"); break;
        case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AlphaPostMultiplied"); break;
        case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "AlphaInherit"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageUsageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));

    for (auto mask = MakeEnumBitMask(value); mask; ) {
        const auto it = static_cast<VkImageUsageFlagBits>(1u << mask.PopFront_AssumeNotEmpty());

        switch (it) {
        case VK_IMAGE_USAGE_TRANSFER_SRC_BIT: oss << sep << STRING_LITERAL(_Char, "TransferSrc"); break;
        case VK_IMAGE_USAGE_TRANSFER_DST_BIT: oss << sep << STRING_LITERAL(_Char, "TransferDst"); break;
        case VK_IMAGE_USAGE_SAMPLED_BIT: oss << sep << STRING_LITERAL(_Char, "Sampled"); break;
        case VK_IMAGE_USAGE_STORAGE_BIT: oss << sep << STRING_LITERAL(_Char, "Storage"); break;
        case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: oss << sep << STRING_LITERAL(_Char, "ColorAttachment"); break;
        case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachment"); break;
        case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT: oss << sep << STRING_LITERAL(_Char, "TransientAttachment"); break;
        case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT: oss << sep << STRING_LITERAL(_Char, "InputAttachment"); break;
        case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT: oss << sep << STRING_LITERAL(_Char, "FragmentDensityMap"); break;
        case VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR: oss << sep << STRING_LITERAL(_Char, "FragmentShadingRateAttachment"); break;
        case VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI: oss << sep << STRING_LITERAL(_Char, "InvocationMaskHuawei"); break;
        default: AssertNotImplemented();
        }
    }

    if (value == Zero)
        oss << STRING_LITERAL(_Char, "0");

    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkMemoryHeapFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkMemoryHeapFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkMemoryPropertyFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkMemoryPropertyFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkPhysicalDeviceType value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkPhysicalDeviceType value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkPipelineStageFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkPipelineStageFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkDependencyFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkDependencyFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkAccessFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkAccessFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageLayout value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageLayout value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageAspectFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageAspectFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkFilter value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkFilter value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkColorSpaceKHR value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkColorSpaceKHR value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkPresentModeKHR value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkPresentModeKHR value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkSurfaceTransformFlagBitsKHR value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkSurfaceTransformFlagBitsKHR value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkCompositeAlphaFlagBitsKHR value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkCompositeAlphaFlagBitsKHR value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API PPE::FTextWriter& operator <<(PPE::FTextWriter& oss, VkImageUsageFlagBits value) { return PPE::RHI::Write_(oss, value); }
PPE_RHIVULKAN_API PPE::FWTextWriter& operator <<(PPE::FWTextWriter& oss, VkImageUsageFlagBits value) { return PPE::RHI::Write_(oss, value); }
//----------------------------------------------------------------------------
