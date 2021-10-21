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
    if (value & bit) {
        value = static_cast<T>(value & ~bit);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPipelineStageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)) oss << sep << STRING_LITERAL(_Char, "TOP_OF_PIPE");
    if (PopBit_(value, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)) oss << sep << STRING_LITERAL(_Char, "DRAW_INDIRECT");
    if (PopBit_(value, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)) oss << sep << STRING_LITERAL(_Char, "VERTEX_INPUT");
    if (PopBit_(value, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "VERTEX_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "TESSELLATION_CONTROL_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "TESSELLATION_EVALUATION_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "GEOMETRY_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)) oss << sep << STRING_LITERAL(_Char, "EARLY_FRAGMENT_TESTS");
    if (PopBit_(value, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)) oss << sep << STRING_LITERAL(_Char, "LATE_FRAGMENT_TESTS");
    if (PopBit_(value, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)) oss << sep << STRING_LITERAL(_Char, "COLOR_ATTACHMENT_OUTPUT");
    if (PopBit_(value, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)) oss << sep << STRING_LITERAL(_Char, "COMPUTE_SHADER");
    if (PopBit_(value, VK_PIPELINE_STAGE_TRANSFER_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSFER");
    if (PopBit_(value, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)) oss << sep << STRING_LITERAL(_Char, "BOTTOM_OF_PIPE");
    if (PopBit_(value, VK_PIPELINE_STAGE_HOST_BIT)) oss << sep << STRING_LITERAL(_Char, "HOST");
    if (PopBit_(value, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)) oss << sep << STRING_LITERAL(_Char, "ALL_GRAPHICS");
    if (PopBit_(value, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)) oss << sep << STRING_LITERAL(_Char, "ALL_COMMANDS");
    if (PopBit_(value, VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TRANSFORM_FEEDBACK_EXT");
    if (PopBit_(value, VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "CONDITIONAL_RENDERING_EXT");
    if (PopBit_(value, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ACCELERATION_STRUCTURE_BUILD_KHR");
    if (PopBit_(value, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "RAY_TRACING_SHADER_KHR");
    if (PopBit_(value, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "TASK_SHADER_NV");
    if (PopBit_(value, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "MESH_SHADER_NV");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_DENSITY_PROCESS_EXT");
    if (PopBit_(value, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_SHADING_RATE_ATTACHMENT_KHR");
    if (PopBit_(value, VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "COMMAND_PREPROCESS_NV");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkDependencyFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_DEPENDENCY_BY_REGION_BIT)) oss << sep << STRING_LITERAL(_Char, "BY_REGION");
    if (PopBit_(value, VK_DEPENDENCY_DEVICE_GROUP_BIT)) oss << sep << STRING_LITERAL(_Char, "DEVICE_GROUP");
    if (PopBit_(value, VK_DEPENDENCY_VIEW_LOCAL_BIT)) oss << sep << STRING_LITERAL(_Char, "VIEW_LOCAL");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkAccessFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_ACCESS_INDIRECT_COMMAND_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "INDIRECT_COMMAND_READ");
    if (PopBit_(value, VK_ACCESS_INDEX_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "INDEX_READ");
    if (PopBit_(value, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "VERTEX_ATTRIBUTE_READ");
    if (PopBit_(value, VK_ACCESS_UNIFORM_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "UNIFORM_READ");
    if (PopBit_(value, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "INPUT_ATTACHMENT_READ");
    if (PopBit_(value, VK_ACCESS_SHADER_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "SHADER_READ");
    if (PopBit_(value, VK_ACCESS_SHADER_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "SHADER_WRITE");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "COLOR_ATTACHMENT_READ");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "COLOR_ATTACHMENT_WRITE");
    if (PopBit_(value, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "DEPTH_STENCIL_ATTACHMENT_READ");
    if (PopBit_(value, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "DEPTH_STENCIL_ATTACHMENT_WRITE");
    if (PopBit_(value, VK_ACCESS_TRANSFER_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSFER_READ");
    if (PopBit_(value, VK_ACCESS_TRANSFER_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSFER_WRITE");
    if (PopBit_(value, VK_ACCESS_HOST_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "HOST_READ");
    if (PopBit_(value, VK_ACCESS_HOST_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "HOST_WRITE");
    if (PopBit_(value, VK_ACCESS_MEMORY_READ_BIT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_READ");
    if (PopBit_(value, VK_ACCESS_MEMORY_WRITE_BIT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_WRITE");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TRANSFORM_FEEDBACK_WRITE_EXT");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TRANSFORM_FEEDBACK_COUNTER_READ_EXT");
    if (PopBit_(value, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "TRANSFORM_FEEDBACK_COUNTER_WRITE_EXT");
    if (PopBit_(value, VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "CONDITIONAL_RENDERING_READ_EXT");
    if (PopBit_(value, VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "COLOR_ATTACHMENT_READ_NONCOHERENT_KHR");
    if (PopBit_(value, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ACCELERATION_STRUCTURE_READ_KHR");
    if (PopBit_(value, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ACCELERATION_STRUCTURE_WRITE_KHR");
    if (PopBit_(value, VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_DENSITY_MAP_READ_EXT");
    if (PopBit_(value, VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_SHADING_RATE_ATTACHMENT_READ_KHR");
    if (PopBit_(value, VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "COMMAND_PREPROCESS_READ_NV");
    if (PopBit_(value, VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV)) oss << sep << STRING_LITERAL(_Char, "COMMAND_PREPROCESS_WRITE_NV");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageLayout value) {
    switch (value) {
    case VK_IMAGE_LAYOUT_UNDEFINED: return oss << STRING_LITERAL(_Char, "UNDEFINED");
    case VK_IMAGE_LAYOUT_GENERAL: return oss << STRING_LITERAL(_Char, "GENERAL");
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "COLOR_ATTACHMENT_OPTIMAL");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_STENCIL_ATTACHMENT_OPTIMAL");
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_STENCIL_READ_ONLY_OPTIMAL");
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "SHADER_READ_ONLY_OPTIMAL");
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return oss << STRING_LITERAL(_Char, "TRANSFER_SRC_OPTIMAL");
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return oss << STRING_LITERAL(_Char, "TRANSFER_DST_OPTIMAL");
    case VK_IMAGE_LAYOUT_PREINITIALIZED: return oss << STRING_LITERAL(_Char, "PREINITIALIZED");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL");
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL");
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_ATTACHMENT_OPTIMAL");
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "DEPTH_READ_ONLY_OPTIMAL");
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return oss << STRING_LITERAL(_Char, "STENCIL_ATTACHMENT_OPTIMAL");
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: return oss << STRING_LITERAL(_Char, "STENCIL_READ_ONLY_OPTIMAL");
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return oss << STRING_LITERAL(_Char, "PRESENT_SRC_KHR");
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR: return oss << STRING_LITERAL(_Char, "SHARED_PRESENT_KHR");
    case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return oss << STRING_LITERAL(_Char, "FRAGMENT_DENSITY_MAP_OPTIMAL_EXT");
    case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR");
    case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "READ_ONLY_OPTIMAL_KHR");
    case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "ATTACHMENT_OPTIMAL_KHR");
    case VK_IMAGE_LAYOUT_MAX_ENUM:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageAspectFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_IMAGE_ASPECT_COLOR_BIT)) oss << sep << STRING_LITERAL(_Char, "COLOR");
    if (PopBit_(value, VK_IMAGE_ASPECT_DEPTH_BIT)) oss << sep << STRING_LITERAL(_Char, "DEPTH");
    if (PopBit_(value, VK_IMAGE_ASPECT_STENCIL_BIT)) oss << sep << STRING_LITERAL(_Char, "STENCIL");
    if (PopBit_(value, VK_IMAGE_ASPECT_METADATA_BIT)) oss << sep << STRING_LITERAL(_Char, "METADATA");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_0_BIT)) oss << sep << STRING_LITERAL(_Char, "PLANE_0");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_1_BIT)) oss << sep << STRING_LITERAL(_Char, "PLANE_1");
    if (PopBit_(value, VK_IMAGE_ASPECT_PLANE_2_BIT)) oss << sep << STRING_LITERAL(_Char, "PLANE_2");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_PLANE_0_EXT");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_PLANE_1_EXT");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_PLANE_2_EXT");
    if (PopBit_(value, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "MEMORY_PLANE_3_EXT");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkFilter value) {
    switch (value) {
    case VK_FILTER_NEAREST: return oss << STRING_LITERAL(_Char, "NEAREST");
    case VK_FILTER_LINEAR: return oss << STRING_LITERAL(_Char, "LINEAR");
    case VK_FILTER_CUBIC_IMG: return oss << STRING_LITERAL(_Char, "CUBIC_IMG");
    case VK_FILTER_MAX_ENUM:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkColorSpaceKHR value) {
    switch (value) {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return oss << STRING_LITERAL(_Char, "SRGB_NONLINEAR_KHR");
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "DISPLAY_P3_NONLINEAR_EXT");
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "EXTENDED_SRGB_LINEAR_EXT");
    case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "DISPLAY_P3_LINEAR_EXT");
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "DCI_P3_NONLINEAR_EXT");
    case VK_COLOR_SPACE_BT709_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "BT709_LINEAR_EXT");
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "BT709_NONLINEAR_EXT");
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "BT2020_LINEAR_EXT");
    case VK_COLOR_SPACE_HDR10_ST2084_EXT: return oss << STRING_LITERAL(_Char, "HDR10_ST2084_EXT");
    case VK_COLOR_SPACE_DOLBYVISION_EXT: return oss << STRING_LITERAL(_Char, "DOLBYVISION_EXT");
    case VK_COLOR_SPACE_HDR10_HLG_EXT: return oss << STRING_LITERAL(_Char, "HDR10_HLG_EXT");
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return oss << STRING_LITERAL(_Char, "ADOBERGB_LINEAR_EXT");
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "ADOBERGB_NONLINEAR_EXT");
    case VK_COLOR_SPACE_PASS_THROUGH_EXT: return oss << STRING_LITERAL(_Char, "PASS_THROUGH_EXT");
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return oss << STRING_LITERAL(_Char, "EXTENDED_SRGB_NONLINEAR_EXT");
    case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: return oss << STRING_LITERAL(_Char, "DISPLAY_NATIVE_AMD");
    case VK_COLOR_SPACE_MAX_ENUM_KHR:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkPresentModeKHR value) {
    switch (value) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: return oss << STRING_LITERAL(_Char, "IMMEDIATE");
    case VK_PRESENT_MODE_MAILBOX_KHR: return oss << STRING_LITERAL(_Char, "MAILBOX");
    case VK_PRESENT_MODE_FIFO_KHR: return oss << STRING_LITERAL(_Char, "FIFO");
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return oss << STRING_LITERAL(_Char, "FIFO_RELAXED");
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR: return oss << STRING_LITERAL(_Char, "SHARED_DEMAND_REFRESH");
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR: return oss << STRING_LITERAL(_Char, "SHARED_CONTINUOUS_REFRESH");
    case VK_PRESENT_MODE_MAX_ENUM_KHR:
        AssertNotImplemented();
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkSurfaceTransformFlagBitsKHR value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "IDENTITY");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ROTATE_90");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ROTATE_180");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ROTATE_270");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HORIZONTAL_MIRROR");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HORIZONTAL_MIRROR_ROTATE_90");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HORIZONTAL_MIRROR_ROTATE_180");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "HORIZONTAL_MIRROR_ROTATE_270");
    if (PopBit_(value, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "INHERIT");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkCompositeAlphaFlagBitsKHR value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ALPHA_OPAQUE");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ALPHA_PRE_MULTIPLIED");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ALPHA_POST_MULTIPLIED");
    if (PopBit_(value, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "ALPHA_INHERIT");

    if (value != 0) oss << sep << STRING_LITERAL(_Char, "<Unknown!>");

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Write_(TBasicTextWriter<_Char>& oss, VkImageUsageFlagBits value) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
    if (PopBit_(value, VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSFER_SRC");
    if (PopBit_(value, VK_IMAGE_USAGE_TRANSFER_DST_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSFER_DST");
    if (PopBit_(value, VK_IMAGE_USAGE_SAMPLED_BIT)) oss << sep << STRING_LITERAL(_Char, "SAMPLED");
    if (PopBit_(value, VK_IMAGE_USAGE_STORAGE_BIT)) oss << sep << STRING_LITERAL(_Char, "STORAGE");
    if (PopBit_(value, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "COLOR_ATTACHMENT");
    if (PopBit_(value, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "DEPTH_STENCIL_ATTACHMENT");
    if (PopBit_(value, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "TRANSIENT_ATTACHMENT");
    if (PopBit_(value, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) oss << sep << STRING_LITERAL(_Char, "INPUT_ATTACHMENT");
    if (PopBit_(value, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_DENSITY_MAP");
    if (PopBit_(value, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) oss << sep << STRING_LITERAL(_Char, "FRAGMENT_SHADING_RATE_ATTACHMENT");
    if (PopBit_(value, VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)) oss << sep << STRING_LITERAL(_Char, "INVOCATION_MASK_HUAWEI");

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
