#include "stdafx.h"

#include "Vulkan/Common/VulkanEnumToString.h"

#include "IO/FormatHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static bool PopBit_(T& value, T bit) NOEXCEPT {
    if (Meta::EnumHas(value, bit)) {
        value = Meta::EnumRemove(value, bit);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char>
struct TVkEnumFlagsNone_ {
    TBasicTextWriter<_Char>& Oss;
    const std::streamoff StartedAt;

    explicit TVkEnumFlagsNone_(TBasicTextWriter<_Char>& oss) NOEXCEPT
    :   Oss(oss)
    ,   StartedAt(Oss.Stream()->TellO()) {
    }

    ~TVkEnumFlagsNone_() {
        if (Oss.Stream()->TellO() == StartedAt)
            Oss << STRING_LITERAL(_Char, "0");
    }
};
template <typename _Char>
TVkEnumFlagsNone_(TBasicTextWriter<_Char>&)->TVkEnumFlagsNone_<_Char>;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPipelineStageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)) oss << sep << STRING_LITERAL(_Char, "TopOfPipe");
    if (PopBit_(value, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)) oss << sep << STRING_LITERAL(_Char, "DrawIndirect");
    if (PopBit_(value, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)) oss << sep << STRING_LITERAL(_Char, "VertexInput");
    if (PopBit_(value, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "VertexShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "TessellationControlShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "TessellationEvaluationShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "GeometryShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "FragmentShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)) oss << sep << STRING_LITERAL(_Char, "EarlyFragmentTests");
    if (PopBit_(value, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)) oss << sep << STRING_LITERAL(_Char, "LateFragmentTests");
    if (PopBit_(value, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentOutput");
    if (PopBit_(value, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "ComputeShader");
    if (PopBit_(value, VK_PIPELINE_STAGE_TRANSFER_BIT)) oss << sep << STRING_LITERAL(_Char, "Transfer");
    if (PopBit_(value, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)) oss << sep << STRING_LITERAL(_Char, "BottomOfPipe");
    if (PopBit_(value, VK_PIPELINE_STAGE_HOST_BIT)) oss << sep << STRING_LITERAL(_Char, "Host");
    if (PopBit_(value, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)) oss << sep << STRING_LITERAL(_Char, "AllGraphics");
    if (PopBit_(value, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)) oss << sep << STRING_LITERAL(_Char, "AllCommands");
    if (PopBit_(value, VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TransformFeedbackExt");
    if (PopBit_(value, VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "ConditionalRenderingExt");
    if (PopBit_(value, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AccelerationStructureBuildKhr");
    if (PopBit_(value, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "RayTracingShaderKhr");
    if (PopBit_(value, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "TaskShaderNv");
    if (PopBit_(value, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "MeshShaderNv");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FragmentDensityProcessExt");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FragmentShadingRateAttachmentKhr");
    if (PopBit_(value, VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "CommandPreprocessNv");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkDependencyFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_DEPENDENCY_BY_REGION_BIT)) oss << sep << STRING_LITERAL(_Char, "ByRegion");
    if (PopBit_(value, VK_DEPENDENCY_DEVICE_GROUP_BIT)) oss << sep << STRING_LITERAL(_Char, "DeviceGroup");
    if (PopBit_(value, VK_DEPENDENCY_VIEW_LOCAL_BIT)) oss << sep << STRING_LITERAL(_Char, "ViewLocal");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkAccessFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_ACCESS_INDIRECT_COMMAND_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "IndirectCommandRead");
    if (PopBit_(value, VK_ACCESS_INDEX_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "IndexRead");
    if (PopBit_(value, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "VertexAttributeRead");
    if (PopBit_(value, VK_ACCESS_UNIFORM_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "UniformRead");
    if (PopBit_(value, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "InputAttachmentRead");
    if (PopBit_(value, VK_ACCESS_SHADER_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "ShaderRead");
    if (PopBit_(value, VK_ACCESS_SHADER_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "ShaderWrite");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentRead");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentWrite");
    if (PopBit_(value, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachmentRead");
    if (PopBit_(value, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachmentWrite");
    if (PopBit_(value, VK_ACCESS_TRANSFER_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "TransferRead");
    if (PopBit_(value, VK_ACCESS_TRANSFER_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "TransferWrite");
    if (PopBit_(value, VK_ACCESS_HOST_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "HostRead");
    if (PopBit_(value, VK_ACCESS_HOST_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "HostWrite");
    if (PopBit_(value, VK_ACCESS_MEMORY_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "MemoryRead");
    if (PopBit_(value, VK_ACCESS_MEMORY_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "MemoryWrite");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TransformFeedbackWriteExt");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TransformFeedbackCounterReadExt");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TransformFeedbackCounterWriteExt");
    if (PopBit_(value, VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "ConditionalRenderingReadExt");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "ColorAttachmentReadNoncoherentKhr");
    if (PopBit_(value, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AccelerationStructureReadKhr");
    if (PopBit_(value, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AccelerationStructureWriteKhr");
    if (PopBit_(value, VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FragmentDensityMapReadExt");
    if (PopBit_(value, VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FragmentShadingRateAttachmentReadKhr");
    if (PopBit_(value, VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "CommandPreprocessReadNv");
    if (PopBit_(value, VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "CommandPreprocessWriteNv");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageLayout value) {
    switch (value) {
    case VK_IMAGE_LAYOUT_UNDEFINED: return oss << STRING_LITERAL(_Char, "Undefined");
    case VK_IMAGE_LAYOUT_GENERAL: return oss << STRING_LITERAL(_Char, "General");
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "ColorAttachmentOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthStencilAttachmentOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthStencilReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "ShaderReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return oss << STRING_LITERAL(_Char, "TransferSrcOptimal");
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return oss << STRING_LITERAL(_Char, "TransferDstOptimal");
    case VK_IMAGE_LAYOUT_PREINITIALIZED: return oss << STRING_LITERAL(_Char, "Preinitialized");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthReadOnlyStencilAttachmentOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthAttachmentStencilReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthAttachmentOptimal");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DepthReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "StencilAttachmentOptimal");
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "StencilReadOnlyOptimal");
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return oss << STRING_LITERAL(_Char, "PresentSrcKhr");
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR: return oss << STRING_LITERAL(_Char, "SharedPresentKhr");
    case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return oss << STRING_LITERAL(_Char, "FragmentDensityMapOptimalExt");
    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "FragmentShadingRateAttachmentOptimalKhr");
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
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_IMAGE_ASPECT_COLOR_BIT)) oss << sep << STRING_LITERAL(_Char, "Color");
    if (PopBit_(value, VK_IMAGE_ASPECT_DEPTH_BIT)) oss << sep << STRING_LITERAL(_Char, "Depth");
    if (PopBit_(value, VK_IMAGE_ASPECT_STENCIL_BIT)) oss << sep << STRING_LITERAL(_Char, "Stencil");
    if (PopBit_(value, VK_IMAGE_ASPECT_METADATA_BIT)) oss << sep << STRING_LITERAL(_Char, "Metadata");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_0_BIT)) oss << sep << STRING_LITERAL(_Char, "Plane0");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_1_BIT)) oss << sep << STRING_LITERAL(_Char, "Plane1");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_2_BIT)) oss << sep << STRING_LITERAL(_Char, "Plane2");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MemoryPlane0Ext");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MemoryPlane1Ext");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MemoryPlane2Ext");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MemoryPlane3Ext");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

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
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "Identity");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "Rotate90");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "Rotate180");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "Rotate270");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HorizontalMirror");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate90");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate180");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HorizontalMirrorRotate270");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "Inherit");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkCompositeAlphaFlagBitsKHR value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AlphaOpaque");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AlphaPreMultiplied");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AlphaPostMultiplied");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "AlphaInherit");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageUsageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, " | "));
    TVkEnumFlagsNone_ none{ oss };
    Unused(none);

    if (PopBit_(value, VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) oss << sep << STRING_LITERAL(_Char, "TransferSrc");
    if (PopBit_(value, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) oss << sep << STRING_LITERAL(_Char, "TransferDst");
    if (PopBit_(value, VK_IMAGE_USAGE_SAMPLED_BIT)) oss << sep << STRING_LITERAL(_Char, "Sampled");
    if (PopBit_(value, VK_IMAGE_USAGE_STORAGE_BIT)) oss << sep << STRING_LITERAL(_Char, "Storage");
    if (PopBit_(value, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "ColorAttachment");
    if (PopBit_(value, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "DepthStencilAttachment");
    if (PopBit_(value, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "TransientAttachment");
    if (PopBit_(value, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "InputAttachment");
    if (PopBit_(value, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FragmentDensityMap");
    if (PopBit_(value, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FragmentShadingRateAttachment");
    if (PopBit_(value, VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)) oss << sep << STRING_LITERAL(_Char, "InvocationMaskHuawei");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

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
