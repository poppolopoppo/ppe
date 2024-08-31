// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Common/VulkanEnums.h"

#include "RHI/ImageHelpers.h"
#include "RHI/PixelFormatHelpers.h"
#include "RHI/RayTracingEnums.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/ResourceState.h"
#include "RHI/SamplerEnums.h"
#include "RHI/ShaderEnums.h"
#include "RHI/SwapchainDesc.h"
#include "RHI/VertexEnums.h"
#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkSampleCountFlagBits VkCast(FMultiSamples value) NOEXCEPT {
    return Clamp(static_cast<VkSampleCountFlagBits>(*value), VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_64_BIT);
}
//----------------------------------------------------------------------------
VkLogicOp VkCast(ELogicOp value) {
    switch (value) {
    case ELogicOp::Clear: return VK_LOGIC_OP_CLEAR;
    case ELogicOp::Set: return VK_LOGIC_OP_SET;
    case ELogicOp::Copy: return VK_LOGIC_OP_COPY;
    case ELogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
    case ELogicOp::NoOp: return VK_LOGIC_OP_NO_OP;
    case ELogicOp::Invert: return VK_LOGIC_OP_INVERT;
    case ELogicOp::And: return VK_LOGIC_OP_AND;
    case ELogicOp::NotAnd: return VK_LOGIC_OP_NAND;
    case ELogicOp::Or: return VK_LOGIC_OP_OR;
    case ELogicOp::NotOr: return VK_LOGIC_OP_NOR;
    case ELogicOp::Xor: return VK_LOGIC_OP_XOR;
    case ELogicOp::Equiv: return VK_LOGIC_OP_EQUIVALENT;
    case ELogicOp::AndReverse: return VK_LOGIC_OP_AND_REVERSE;
    case ELogicOp::AndInverted: return VK_LOGIC_OP_AND_INVERTED;
    case ELogicOp::OrReverse: return VK_LOGIC_OP_OR_REVERSE;
    case ELogicOp::OrInverted: return VK_LOGIC_OP_OR_INVERTED;

    case ELogicOp::None:
    case ELogicOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkBlendFactor VkCast(EBlendFactor factor) {
    switch (factor) {
    case EBlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
    case EBlendFactor::One: return VK_BLEND_FACTOR_ONE;
    case EBlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
    case EBlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case EBlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
    case EBlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case EBlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
    case EBlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case EBlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
    case EBlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case EBlendFactor::ConstColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case EBlendFactor::OneMinusConstColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case EBlendFactor::ConstAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case EBlendFactor::OneMinusConstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case EBlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case EBlendFactor::Src1Color: return VK_BLEND_FACTOR_SRC1_COLOR;
    case EBlendFactor::OneMinusSrc1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case EBlendFactor::Src1Alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
    case EBlendFactor::OneMinusSrc1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;

    case EBlendFactor::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkBlendOp VkCast(EBlendOp value) {
    switch (value) {
    case EBlendOp::Add: return VK_BLEND_OP_ADD;
    case EBlendOp::Sub: return VK_BLEND_OP_SUBTRACT;
    case EBlendOp::RevSub: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case EBlendOp::Min: return VK_BLEND_OP_MIN;
    case EBlendOp::Max: return VK_BLEND_OP_MAX;

    case EBlendOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkFormat VkCast(EVertexFormat value) {
    switch (value) {
    case EVertexFormat::Byte: return VK_FORMAT_R8_SINT;
    case EVertexFormat::Byte2: return VK_FORMAT_R8G8_SINT;
    case EVertexFormat::Byte3: return VK_FORMAT_R8G8B8_SINT;
    case EVertexFormat::Byte4: return VK_FORMAT_R8G8B8A8_SINT;
    case EVertexFormat::Byte_Norm: return VK_FORMAT_R8_SNORM;
    case EVertexFormat::Byte2_Norm: return VK_FORMAT_R8G8_SNORM;
    case EVertexFormat::Byte3_Norm: return VK_FORMAT_R8G8B8_SNORM;
    case EVertexFormat::Byte4_Norm: return VK_FORMAT_R8G8B8A8_SNORM;
    case EVertexFormat::Byte_Scaled: return VK_FORMAT_R8_SSCALED;
    case EVertexFormat::Byte2_Scaled: return VK_FORMAT_R8G8_SSCALED;
    case EVertexFormat::Byte3_Scaled: return VK_FORMAT_R8G8B8_SSCALED;
    case EVertexFormat::Byte4_Scaled: return VK_FORMAT_R8G8B8A8_SSCALED;
    case EVertexFormat::UByte: return VK_FORMAT_R8_UINT;
    case EVertexFormat::UByte2: return VK_FORMAT_R8G8_UINT;
    case EVertexFormat::UByte3: return VK_FORMAT_R8G8B8_UINT;
    case EVertexFormat::UByte4: return VK_FORMAT_R8G8B8A8_UINT;
    case EVertexFormat::UByte_Norm: return VK_FORMAT_R8_UNORM;
    case EVertexFormat::UByte2_Norm: return VK_FORMAT_R8G8_UNORM;
    case EVertexFormat::UByte3_Norm: return VK_FORMAT_R8G8B8_UNORM;
    case EVertexFormat::UByte4_Norm: return VK_FORMAT_R8G8B8A8_UNORM;
    case EVertexFormat::UByte_Scaled: return VK_FORMAT_R8_USCALED;
    case EVertexFormat::UByte2_Scaled: return VK_FORMAT_R8G8_USCALED;
    case EVertexFormat::UByte3_Scaled: return VK_FORMAT_R8G8B8_USCALED;
    case EVertexFormat::UByte4_Scaled: return VK_FORMAT_R8G8B8A8_USCALED;
    case EVertexFormat::Short: return VK_FORMAT_R16_SINT;
    case EVertexFormat::Short2: return VK_FORMAT_R16G16_SINT;
    case EVertexFormat::Short3: return VK_FORMAT_R16G16B16_SINT;
    case EVertexFormat::Short4: return VK_FORMAT_R16G16B16A16_SINT;
    case EVertexFormat::Short_Norm: return VK_FORMAT_R16_SNORM;
    case EVertexFormat::Short2_Norm: return VK_FORMAT_R16G16_SNORM;
    case EVertexFormat::Short3_Norm: return VK_FORMAT_R16G16B16_SNORM;
    case EVertexFormat::Short4_Norm: return VK_FORMAT_R16G16B16A16_SNORM;
    case EVertexFormat::Short_Scaled: return VK_FORMAT_R16_SSCALED;
    case EVertexFormat::Short2_Scaled: return VK_FORMAT_R16G16_SSCALED;
    case EVertexFormat::Short3_Scaled: return VK_FORMAT_R16G16B16_SSCALED;
    case EVertexFormat::Short4_Scaled: return VK_FORMAT_R16G16B16A16_SSCALED;
    case EVertexFormat::UShort: return VK_FORMAT_R16_UINT;
    case EVertexFormat::UShort2: return VK_FORMAT_R16G16_UINT;
    case EVertexFormat::UShort3: return VK_FORMAT_R16G16B16_UINT;
    case EVertexFormat::UShort4: return VK_FORMAT_R16G16B16A16_UINT;
    case EVertexFormat::UShort_Norm: return VK_FORMAT_R16_UNORM;
    case EVertexFormat::UShort2_Norm: return VK_FORMAT_R16G16_UNORM;
    case EVertexFormat::UShort3_Norm: return VK_FORMAT_R16G16B16_UNORM;
    case EVertexFormat::UShort4_Norm: return VK_FORMAT_R16G16B16A16_UNORM;
    case EVertexFormat::UShort_Scaled: return VK_FORMAT_R16_USCALED;
    case EVertexFormat::UShort2_Scaled: return VK_FORMAT_R16G16_USCALED;
    case EVertexFormat::UShort3_Scaled: return VK_FORMAT_R16G16B16_USCALED;
    case EVertexFormat::UShort4_Scaled: return VK_FORMAT_R16G16B16A16_USCALED;
    case EVertexFormat::Int: return VK_FORMAT_R32_SINT;
    case EVertexFormat::Int2: return VK_FORMAT_R32G32_SINT;
    case EVertexFormat::Int3: return VK_FORMAT_R32G32B32_SINT;
    case EVertexFormat::Int4: return VK_FORMAT_R32G32B32A32_SINT;
    case EVertexFormat::UInt: return VK_FORMAT_R32_UINT;
    case EVertexFormat::UInt2: return VK_FORMAT_R32G32_UINT;
    case EVertexFormat::UInt3: return VK_FORMAT_R32G32B32_UINT;
    case EVertexFormat::UInt4: return VK_FORMAT_R32G32B32A32_UINT;
    case EVertexFormat::Long: return VK_FORMAT_R64_SINT;
    case EVertexFormat::Long2: return VK_FORMAT_R64G64_SINT;
    case EVertexFormat::Long3: return VK_FORMAT_R64G64B64_SINT;
    case EVertexFormat::Long4: return VK_FORMAT_R64G64B64A64_SINT;
    case EVertexFormat::ULong: return VK_FORMAT_R64_UINT;
    case EVertexFormat::ULong2: return VK_FORMAT_R64G64_UINT;
    case EVertexFormat::ULong3: return VK_FORMAT_R64G64B64_UINT;
    case EVertexFormat::ULong4: return VK_FORMAT_R64G64B64A64_UINT;
    case EVertexFormat::Half: return VK_FORMAT_R16_SFLOAT;
    case EVertexFormat::Half2: return VK_FORMAT_R16G16_SFLOAT;
    case EVertexFormat::Half3: return VK_FORMAT_R16G16B16_SFLOAT;
    case EVertexFormat::Half4: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EVertexFormat::Float: return VK_FORMAT_R32_SFLOAT;
    case EVertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case EVertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
    case EVertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case EVertexFormat::Double: return VK_FORMAT_R64_SFLOAT;
    case EVertexFormat::Double2: return VK_FORMAT_R64_SFLOAT;
    case EVertexFormat::Double3: return VK_FORMAT_R64_SFLOAT;
    case EVertexFormat::Double4: return VK_FORMAT_R64_SFLOAT;

    case EVertexFormat::Unknown:
    default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkVertexInputRate VkCast(EVertexInputRate value) {
    switch (value) {
    case EVertexInputRate::Vertex: return VK_VERTEX_INPUT_RATE_VERTEX;
    case EVertexInputRate::Instance: return VK_VERTEX_INPUT_RATE_INSTANCE;

    case EVertexInputRate::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkShaderStageFlagBits VkCast(EShaderType value) {
    switch (value) {
    case EShaderType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
    case EShaderType::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case EShaderType::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case EShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
    case EShaderType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
    case EShaderType::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;

#ifdef VK_NV_mesh_shader
    case EShaderType::MeshTask: return VK_SHADER_STAGE_TASK_BIT_NV;
    case EShaderType::Mesh: return VK_SHADER_STAGE_MESH_BIT_NV;
#endif

#ifdef VK_NV_ray_tracing
    case EShaderType::RayGen: return VK_SHADER_STAGE_RAYGEN_BIT_NV;
    case EShaderType::RayAnyHit: return VK_SHADER_STAGE_ANY_HIT_BIT_NV;
    case EShaderType::RayClosestHit: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
    case EShaderType::RayMiss: return VK_SHADER_STAGE_MISS_BIT_NV;
    case EShaderType::RayIntersection: return VK_SHADER_STAGE_INTERSECTION_BIT_NV;
    case EShaderType::RayCallable: return VK_SHADER_STAGE_CALLABLE_BIT_NV;
#endif

    case EShaderType::_Count:
    case EShaderType::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkShaderStageFlagBits VkCast(EShaderStages values) {
    VkShaderStageFlagBits flags = VkShaderStageFlagBits(0);
    for (EShaderStages st = EShaderStages(1); st <= values; st = EShaderStages(u32(st) << 1u)) {
        if (not (values & st))
            continue;

        switch (st) {
        case EShaderStages::Vertex: flags |= VK_SHADER_STAGE_VERTEX_BIT; break;
        case EShaderStages::TessControl: flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
        case EShaderStages::TessEvaluation: flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
        case EShaderStages::Geometry: flags |= VK_SHADER_STAGE_GEOMETRY_BIT; break;
        case EShaderStages::Fragment: flags |= VK_SHADER_STAGE_FRAGMENT_BIT; break;
        case EShaderStages::Compute: flags |= VK_SHADER_STAGE_COMPUTE_BIT; break;

#ifdef VK_NV_mesh_shader
        case EShaderStages::MeshTask: flags |= VK_SHADER_STAGE_TASK_BIT_NV; break;
        case EShaderStages::Mesh: flags |= VK_SHADER_STAGE_MESH_BIT_NV; break;
#endif

#ifdef VK_NV_ray_tracing
        case EShaderStages::RayGen: flags |= VK_SHADER_STAGE_RAYGEN_BIT_NV; break;
        case EShaderStages::RayAnyHit: flags |= VK_SHADER_STAGE_ANY_HIT_BIT_NV; break;
        case EShaderStages::RayClosestHit: flags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV; break;
        case EShaderStages::RayMiss: flags |= VK_SHADER_STAGE_MISS_BIT_NV; break;
        case EShaderStages::RayIntersection: flags |= VK_SHADER_STAGE_INTERSECTION_BIT_NV; break;
        case EShaderStages::RayCallable: flags |= VK_SHADER_STAGE_CALLABLE_BIT_NV; break;
#endif

        case EShaderStages::All:
        case EShaderStages::AllGraphics:
        case EShaderStages::AllRayTracing:
        case EShaderStages::Unknown:;
        case EShaderStages::_Last: AssertNotImplemented();
        }
    }
    return flags;
}
//----------------------------------------------------------------------------
VkDynamicState VkCast(EPipelineDynamicState value) {
    switch (value) {
    case EPipelineDynamicState::Viewport: return VK_DYNAMIC_STATE_VIEWPORT;
    case EPipelineDynamicState::Scissor: return VK_DYNAMIC_STATE_SCISSOR;
    case EPipelineDynamicState::StencilCompareMask: return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
    case EPipelineDynamicState::StencilWriteMask: return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    case EPipelineDynamicState::StencilReference: return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    case EPipelineDynamicState::ShadingRatePalette: return VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;

    case EPipelineDynamicState::Unknown:
    case EPipelineDynamicState::_Last:
    case EPipelineDynamicState::All:
    case EPipelineDynamicState::Default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkAttachmentLoadOp VkCast(EAttachmentLoadOp value, const FVulkanDevice& device) {
    switch (value) {
    case EAttachmentLoadOp::Invalidate: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case EAttachmentLoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
    case EAttachmentLoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;

    case EAttachmentLoadOp::Keep:
        if (device.HasExtension(EVulkanDeviceExtension::EXT_load_store_op_none))
            return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        return VK_ATTACHMENT_LOAD_OP_LOAD;

    case EAttachmentLoadOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkAttachmentStoreOp VkCast(EAttachmentStoreOp value, const FVulkanDevice& device) {
    switch (value) {
    case EAttachmentStoreOp::Invalidate: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case EAttachmentStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;

    case EAttachmentStoreOp::Keep:
        if (device.HasExtension(EVulkanDeviceExtension::EXT_load_store_op_none))
            return VK_ATTACHMENT_STORE_OP_NONE_EXT;
        return VK_ATTACHMENT_STORE_OP_STORE;

    case EAttachmentStoreOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkCompareOp VkCast(ECompareOp value) {
    switch (value) {
    case ECompareOp::Never: return VK_COMPARE_OP_NEVER;
    case ECompareOp::Less: return VK_COMPARE_OP_LESS;
    case ECompareOp::Equal: return VK_COMPARE_OP_EQUAL;
    case ECompareOp::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::Greater: return VK_COMPARE_OP_GREATER;
    case ECompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::Always: return VK_COMPARE_OP_ALWAYS;

    case ECompareOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkStencilOp VkCast(EStencilOp value) {
    switch (value) {
    case EStencilOp::Keep: return VK_STENCIL_OP_KEEP;
    case EStencilOp::Zero: return VK_STENCIL_OP_ZERO;
    case EStencilOp::Replace: return VK_STENCIL_OP_REPLACE;
    case EStencilOp::Incr: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case EStencilOp::IncrWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case EStencilOp::Decr: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case EStencilOp::DecrWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    case EStencilOp::Invert: return VK_STENCIL_OP_INVERT;

    case EStencilOp::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkPolygonMode VkCast(EPolygonMode value) {
    switch (value) {
    case EPolygonMode::Point: return VK_POLYGON_MODE_POINT;
    case EPolygonMode::Line: return VK_POLYGON_MODE_LINE;
    case EPolygonMode::Fill: return VK_POLYGON_MODE_FILL;

    case EPolygonMode::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkCullModeFlagBits VkCast(ECullMode value) {
    switch (value) {
    case ECullMode::None: return VK_CULL_MODE_NONE;
    case ECullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case ECullMode::Back: return VK_CULL_MODE_BACK_BIT;
    case ECullMode::FontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkFilter VkCast(ETextureFilter value) {
    switch (value) {
    case ETextureFilter::Nearest: return VK_FILTER_NEAREST;
    case ETextureFilter::Linear: return VK_FILTER_LINEAR;
    case ETextureFilter::Cubic: return VK_FILTER_CUBIC_EXT;

    case ETextureFilter::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkSamplerMipmapMode VkCast(EMipmapFilter value) {
    switch (value) {
    case EMipmapFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case EMipmapFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;

    case EMipmapFilter::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkSamplerAddressMode VkCast(EAddressMode value) {
    switch (value) {
    case EAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case EAddressMode::MirrorRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case EAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case EAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case EAddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    case EAddressMode::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkBorderColor VkCast(EBorderColor value) {
    switch (value) {
    case EBorderColor::FloatTransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case EBorderColor::FloatOpaqueBlack: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case EBorderColor::FloatOpaqueWhite: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case EBorderColor::IntTransparentBlack: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case EBorderColor::IntOpaqueBlack: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case EBorderColor::IntOpaqueWhite: return VK_BORDER_COLOR_INT_OPAQUE_WHITE;

    case EBorderColor::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkImageType VkCast(EImageDim value) {
    switch (value) {
    case EImageDim::_1D: return VK_IMAGE_TYPE_1D;
    case EImageDim::_2D: return VK_IMAGE_TYPE_2D;
    case EImageDim::_3D: return VK_IMAGE_TYPE_3D;
    case EImageDim::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkImageCreateFlagBits VkCast(EImageFlags value) {
    VkImageCreateFlagBits result = Zero;

    for (u8 t = 1; t <= static_cast<u8>(value); t <<= 1) {
        if (not Meta::EnumHas(value, t))
            continue;

        switch (static_cast<EImageFlags>(t)) {
        case EImageFlags::Array2DCompatible: result |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; break;
        case EImageFlags::BlockTexelViewCompatible: result |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT; break;
        case EImageFlags::CubeCompatible: result |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; break;
        case EImageFlags::MutableFormat: result |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; break;

        case EImageFlags::Unknown: AssertNotReached();
        }
    }

    return result;
}
//----------------------------------------------------------------------------
VkImageViewType VkCast(EImageView value) {
    switch (value) {
    case EImageView::_1D: return VK_IMAGE_VIEW_TYPE_1D;
    case EImageView::_1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case EImageView::_2D: return VK_IMAGE_VIEW_TYPE_2D;
    case EImageView::_2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case EImageView::_Cube: return VK_IMAGE_VIEW_TYPE_CUBE;
    case EImageView::_CubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    case EImageView::_3D: return VK_IMAGE_VIEW_TYPE_3D;

    case EImageView::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkImageUsageFlagBits VkCast(EImageUsage values) {
    VkImageUsageFlagBits flags = VkImageUsageFlagBits(0);
    for (EImageUsage st = EImageUsage(1); st <= values; st = EImageUsage(u32(st) << 1u)) {
        if (not (values & st))
            continue;

        switch (st) {
        case EImageUsage::BlitSrc: FALLTHROUGH();
        case EImageUsage::TransferSrc: flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; break;
        case EImageUsage::BlitDst: FALLTHROUGH();
        case EImageUsage::TransferDst: flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; break;
        case EImageUsage::SampledMinMax:
        case EImageUsage::Sampled: flags |= VK_IMAGE_USAGE_SAMPLED_BIT; break;
        case EImageUsage::StorageAtomic:
        case EImageUsage::Storage: flags |= VK_IMAGE_USAGE_STORAGE_BIT; break;
        case EImageUsage::ColorAttachmentBlend:
        case EImageUsage::ColorAttachment: flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; break;
        case EImageUsage::DepthStencilAttachment: flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; break;
        case EImageUsage::TransientAttachment: flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; break;
        case EImageUsage::InputAttachment: flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
        case EImageUsage::ShadingRate: flags |= VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV; break;

        case EImageUsage::Unknown:
        default: AssertNotImplemented();
        }
    }
    return flags;
}
//----------------------------------------------------------------------------
VkImageAspectFlagBits VkCast(EImageAspect values) {
    VkImageAspectFlagBits flags{static_cast<VkImageAspectFlagBits>(0)};
    for (u32 st = 1; st <= static_cast<u32>(values); st <<= 1) {
        if (not Meta::EnumHas(values, st))
            continue;

        switch (static_cast<EImageAspect>(st)) {
        case EImageAspect::Color: flags |= VK_IMAGE_ASPECT_COLOR_BIT; break;
        case EImageAspect::Depth: flags |= VK_IMAGE_ASPECT_DEPTH_BIT; break;
        case EImageAspect::Stencil: flags |= VK_IMAGE_ASPECT_STENCIL_BIT; break;
        case EImageAspect::Metadata: flags |= VK_IMAGE_ASPECT_METADATA_BIT; break;

        default: AssertNotImplemented();
        }
    }
    return flags;
}
//----------------------------------------------------------------------------
VkImageAspectFlagBits VkCast(EImageAspect values, EPixelFormat format) {
    if (values == EImageAspect_Auto) {
        if (EPixelFormat_HasDepth(format))
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        if (EPixelFormat_HasStencil(format))
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
    return VkCast(values);
}
//----------------------------------------------------------------------------
VkBufferUsageFlagBits VkCast(EBufferUsage values) {
    VkBufferUsageFlagBits flags{ static_cast<VkBufferUsageFlagBits>(0) };
    for (u32 st = 1; st <= static_cast<u32>(values); st <<= 1) {
        if (not Meta::EnumHas(values, st))
            continue;

        switch (static_cast<EBufferUsage>(st)) {
        case EBufferUsage::TransferSrc: flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
        case EBufferUsage::TransferDst: flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
        case EBufferUsage::UniformTexel: flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT; break;
        case EBufferUsage::StorageTexelAtomic:
        case EBufferUsage::StorageTexel: flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; break;
        case EBufferUsage::Uniform: flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
        case EBufferUsage::VertexPplnStore:
        case EBufferUsage::FragmentPplnStore:
        case EBufferUsage::Storage: flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
        case EBufferUsage::Index: flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
        case EBufferUsage::Vertex: flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
        case EBufferUsage::Indirect: flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; break;
        case EBufferUsage::RayTracing: flags |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV; break;

        default: AssertNotImplemented();
        }
    }
    return flags;
}
//----------------------------------------------------------------------------
VkIndexType VkCast(EIndexFormat value) {
    switch (value) {
    case EIndexFormat::UShort: return VK_INDEX_TYPE_UINT16;
    case EIndexFormat::UInt: return VK_INDEX_TYPE_UINT32;

    case EIndexFormat::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkPrimitiveTopology VkCast(EPrimitiveTopology value) {
    switch ( value )
    {
    case EPrimitiveTopology::Point: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EPrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EPrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EPrimitiveTopology::LineListAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::LineStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EPrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case EPrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case EPrimitiveTopology::TriangleListAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::Patch: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    case EPrimitiveTopology::Unknown:
    case EPrimitiveTopology::_Count: break;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkGeometryFlagBitsKHR VkCast(ERayTracingGeometryFlags values) {
    VkGeometryFlagBitsKHR flags{ static_cast<VkGeometryFlagBitsKHR>(0) };
#ifdef VK_NV_ray_tracing
    for (u32 st = 1; st <= static_cast<u32>(values); st <<= 1) {
        if (not Meta::EnumHas(values, st))
            continue;

        switch (static_cast<ERayTracingGeometryFlags>(st)) {
        case ERayTracingGeometryFlags::Opaque: flags |= VK_GEOMETRY_OPAQUE_BIT_NV; break;
        case ERayTracingGeometryFlags::NoDuplicateAnyHitInvocation: flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_NV; break;

        case ERayTracingGeometryFlags::_Last:
        case ERayTracingGeometryFlags::Unknown: AssertNotImplemented();
        }
    }
#endif
    return flags;
}
//----------------------------------------------------------------------------
VkGeometryInstanceFlagBitsNV VkCast(ERayTracingInstanceFlags values) {
    VkGeometryInstanceFlagBitsNV flags = Zero;
#ifdef VK_NV_ray_tracing
    for (u32 st = 1; st <= static_cast<u32>(values); st <<= 1) {
        if (not Meta::EnumHas(values, st))
            continue;

        switch (static_cast<ERayTracingInstanceFlags>(st)) {
        case ERayTracingInstanceFlags::TriangleCullDisable: flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; break;
        case ERayTracingInstanceFlags::TriangleFrontCCW: flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_NV; break;
        case ERayTracingInstanceFlags::ForceOpaque: flags |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV; break;
        case ERayTracingInstanceFlags::ForceNonOpaque: flags |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV; break;

        case ERayTracingInstanceFlags::_Last:
        case ERayTracingInstanceFlags::Unknown: AssertNotImplemented();
        }
    }
#endif
    return flags;
}
//----------------------------------------------------------------------------
VkBuildAccelerationStructureFlagBitsNV VkCast(ERayTracingBuildFlags values) {
    VkBuildAccelerationStructureFlagBitsNV flags = Zero;
#ifdef VK_NV_ray_tracing
    for (u32 st = 1; st <= static_cast<u32>(values); st <<= 1) {
        if (not Meta::EnumHas(values, st))
            continue;

        switch (static_cast<ERayTracingBuildFlags>(st)) {
        case ERayTracingBuildFlags::AllowUpdate: flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV; break;
        case ERayTracingBuildFlags::AllowCompaction: flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_NV; break;
        case ERayTracingBuildFlags::PreferFastTrace: flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV; break;
        case ERayTracingBuildFlags::PreferFastBuild: flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV; break;
        case ERayTracingBuildFlags::LowMemory: flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV; break;

        case ERayTracingBuildFlags::_Last:
        case ERayTracingBuildFlags::Unknown: AssertNotImplemented();
        }
    }
#endif
    return flags;
}
//----------------------------------------------------------------------------
VkColorSpaceKHR VkCast(EColorSpace value) {
    switch (value) {
    case EColorSpace::PASS_THROUGH: return VK_COLOR_SPACE_PASS_THROUGH_EXT;
    case EColorSpace::SRGB_NONLINEAR: return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    case EColorSpace::DISPLAY_P3_NONLINEAR: return VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT;
    case EColorSpace::EXTENDED_SRGB_LINEAR: return VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
    case EColorSpace::DISPLAY_P3_LINEAR: return VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT;
    case EColorSpace::DCI_P3_NONLINEAR: return VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT;
    case EColorSpace::BT709_LINEAR: return VK_COLOR_SPACE_BT709_LINEAR_EXT;
    case EColorSpace::BT709_NONLINEAR: return VK_COLOR_SPACE_BT709_NONLINEAR_EXT;
    case EColorSpace::BT2020_LINEAR: return VK_COLOR_SPACE_BT2020_LINEAR_EXT;
    case EColorSpace::HDR10_ST2084: return VK_COLOR_SPACE_HDR10_ST2084_EXT;
    case EColorSpace::DOLBYVISION: return VK_COLOR_SPACE_DOLBYVISION_EXT;
    case EColorSpace::HDR10_HLG: return VK_COLOR_SPACE_HDR10_HLG_EXT;
    case EColorSpace::ADOBERGB_LINEAR: return VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT;
    case EColorSpace::ADOBERGB_NONLINEAR: return VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT;
    case EColorSpace::EXTENDED_SRGB_NONLINEAR: return VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT;
    case EColorSpace::DISPLAY_NATIVE_AMD: return VK_COLOR_SPACE_DISPLAY_NATIVE_AMD;
    case EColorSpace::Unknown: break;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkCompositeAlphaFlagBitsKHR VkCast(ECompositeAlpha value) {
    switch (value) {
    case ECompositeAlpha::Opaque: return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    case ECompositeAlpha::PreMultiplied: return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    case ECompositeAlpha::PostMultiplied: return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    case ECompositeAlpha::Inherit: return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    case ECompositeAlpha::Unknown: break;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkPresentModeKHR VkCast(EPresentMode value) {
    switch (value) {
    case EPresentMode::Immediate: return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case EPresentMode::Fifo: return VK_PRESENT_MODE_FIFO_KHR;
    case EPresentMode::RelaxedFifo: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    case EPresentMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
    case EPresentMode::Unknown: break;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkSurfaceTransformFlagBitsKHR VkCast(ESurfaceTransform value) {
    switch (value) {
    case ESurfaceTransform::Identity: return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    case ESurfaceTransform::TransformRotate90: return VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR;
    case ESurfaceTransform::TransformRotate180: return VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR;
    case ESurfaceTransform::TransformRotate270: return VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR;
    case ESurfaceTransform::HorizontalMirror: return VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR;
    case ESurfaceTransform::HorizontalMirror_TransformRotate90: return VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR;
    case ESurfaceTransform::HorizontalMirror_TransformRotate180: return VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR;
    case ESurfaceTransform::HorizontalMirror_TransformRotate270: return VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
// Vk to RHI:
//----------------------------------------------------------------------------
EBufferUsage RHICast(VkBufferUsageFlagBits flags) {
    EBufferUsage result = Default;
    for (VkBufferUsageFlags st = 1; st < VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM; st <<= 1) {
        if (not (flags & st))
            continue;

        switch (static_cast<VkBufferUsageFlagBits>(st)) {
        case VK_BUFFER_USAGE_TRANSFER_SRC_BIT: result |= EBufferUsage::TransferSrc; break;
        case VK_BUFFER_USAGE_TRANSFER_DST_BIT: result |= EBufferUsage::TransferDst; break;
        case VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT: result |= EBufferUsage::UniformTexel; break;
        case VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT: result |= EBufferUsage::StorageTexel; break;
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: result |= EBufferUsage::Uniform; break;
        case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: result |= EBufferUsage::Storage; break;
        case VK_BUFFER_USAGE_INDEX_BUFFER_BIT: result |= EBufferUsage::Index; break;
        case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: result |= EBufferUsage::Vertex; break;
        case VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT: result |= EBufferUsage::Indirect; break;

#ifdef VK_NV_ray_tracing
        case VK_BUFFER_USAGE_RAY_TRACING_BIT_NV: result |= EBufferUsage::RayTracing; break;
#endif
#ifdef VK_KHR_buffer_device_address
        case VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT: //result |= EBufferUsage::ShaderAddress; break;
#endif
#ifdef VK_EXT_transform_feedback
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT:
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT: // #TODO
#endif
#ifdef VK_EXT_conditional_rendering
        case VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT:
#endif

        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR:
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR:

        case VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM:
        default: AssertNotImplemented();
        }
    }
    return result;
}
//----------------------------------------------------------------------------
EImageDim RHICast(VkImageType type) {
    switch (type) {
    case VK_IMAGE_TYPE_1D: return EImageDim_1D;
    case VK_IMAGE_TYPE_2D: return EImageDim_2D;
    case VK_IMAGE_TYPE_3D: return EImageDim_3D;

    case VK_IMAGE_TYPE_MAX_ENUM: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EImageFlags RHICast(VkImageCreateFlagBits flags) {
    EImageFlags result = Default;
    for (VkImageCreateFlags st = 1; st <= static_cast<VkImageCreateFlags>(flags); st <<= 1) {
        if (not (flags & st))
            continue;

        switch (st) {
        case VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT: result |= EImageFlags::CubeCompatible; break;
        case VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT: result |= EImageFlags::Array2DCompatible; break;
        case VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT: result |= EImageFlags::MutableFormat; break;

        case VK_IMAGE_CREATE_SPARSE_BINDING_BIT:
        case VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT:
        case VK_IMAGE_CREATE_SPARSE_ALIASED_BIT:
        case VK_IMAGE_CREATE_ALIAS_BIT:
        case VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT:
        case VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT:
        case VK_IMAGE_CREATE_EXTENDED_USAGE_BIT:
        case VK_IMAGE_CREATE_PROTECTED_BIT:
        case VK_IMAGE_CREATE_DISJOINT_BIT:
        case VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT:
        case VK_IMAGE_CREATE_FLAG_BITS_MAX_ENUM:
#ifdef VK_NV_corner_sampled_image
        case VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV:
#endif
#ifdef VK_EXT_fragment_density_map
        case VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT:
#endif

        default: AssertNotImplemented();
        }
    }
    return result;
}
//----------------------------------------------------------------------------
EImageUsage RHICast(VkImageUsageFlagBits flags) {
    EImageUsage result = Default;
    for (VkImageUsageFlags st = 1; st < VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM; st <<= 1) {
        if (not (flags & st))
            continue;

        switch (st) {
        case VK_IMAGE_USAGE_TRANSFER_SRC_BIT: result |= EImageUsage_BlitTransferSrc; break;
        case VK_IMAGE_USAGE_TRANSFER_DST_BIT: result |= EImageUsage_BlitTransferDst; break;
        case VK_IMAGE_USAGE_SAMPLED_BIT: result |= EImageUsage::Sampled; break;
        case VK_IMAGE_USAGE_STORAGE_BIT: result |= EImageUsage::Storage; break;
        case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: result |= EImageUsage::ColorAttachment; break;
        case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: result |= EImageUsage::DepthStencilAttachment; break;
        case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT: result |= EImageUsage::TransientAttachment; break;
        case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT: result |= EImageUsage::InputAttachment; break;

#ifdef VK_NV_shading_rate_image
        case VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV: result |= EImageUsage::ShadingRate; break;
#endif
#ifdef VK_EXT_fragment_density_map
        case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT: //result |= EImageUsage::FragmentDensityMap; break;
#endif

        default: AssertNotImplemented();
        }
    }
    return result;
}
//----------------------------------------------------------------------------
FMultiSamples RHICast(VkSampleCountFlagBits samples) {
    if (0 == samples)
        return 1_samples;

    Assert(Meta::IsPow2(static_cast<VkFlags>(samples)));
    return FMultiSamples(static_cast<VkFlags>(samples));
}
//----------------------------------------------------------------------------
EImageAspect RHICast(VkImageAspectFlagBits flags) {
    EImageAspect result = Default;
    for (VkImageAspectFlags st = 1; st < VK_IMAGE_ASPECT_METADATA_BIT; st <<= 1) {
        if (not (flags & st))
            continue;

        switch (st) {
        case VK_IMAGE_ASPECT_COLOR_BIT: result |= EImageAspect::Color; break;
        case VK_IMAGE_ASPECT_DEPTH_BIT: result |= EImageAspect::Depth; break;
        case VK_IMAGE_ASPECT_STENCIL_BIT: result |= EImageAspect::Stencil; break;
        case VK_IMAGE_ASPECT_METADATA_BIT: result |= EImageAspect::Metadata; break;

        case VK_IMAGE_ASPECT_PLANE_0_BIT:
        case VK_IMAGE_ASPECT_PLANE_1_BIT:
        case VK_IMAGE_ASPECT_PLANE_2_BIT:

#ifdef VK_EXT_image_drm_format_modifier
        case VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT:
        case VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT:
        case VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT:
        case VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT:
#endif
        case VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM:
        default: AssertNotImplemented();
        }
    }
    return result;
}
//----------------------------------------------------------------------------
EColorSpace RHICast(VkColorSpaceKHR value) {
    switch (value) {
    case VK_COLOR_SPACE_PASS_THROUGH_EXT: return EColorSpace::PASS_THROUGH;
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return EColorSpace::SRGB_NONLINEAR;
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT: return EColorSpace::DISPLAY_P3_NONLINEAR;
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return EColorSpace::EXTENDED_SRGB_LINEAR;
    case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT: return EColorSpace::DISPLAY_P3_LINEAR;
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return EColorSpace::DCI_P3_NONLINEAR;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT: return EColorSpace::BT709_LINEAR;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return EColorSpace::BT709_NONLINEAR;
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return EColorSpace::BT2020_LINEAR;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT: return EColorSpace::HDR10_ST2084;
    case VK_COLOR_SPACE_DOLBYVISION_EXT: return EColorSpace::DOLBYVISION;
    case VK_COLOR_SPACE_HDR10_HLG_EXT: return EColorSpace::HDR10_HLG;
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return EColorSpace::ADOBERGB_LINEAR;
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return EColorSpace::ADOBERGB_NONLINEAR;
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return EColorSpace::EXTENDED_SRGB_NONLINEAR;
    case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: return  EColorSpace::DISPLAY_NATIVE_AMD;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VkPipelineStageFlagBits EResourceState_ToPipelineStages(EResourceState value) {
    switch (value.MemoryAccess) {
    case EResourceAccess::Unknown: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    case EResourceAccess::InputAttachment: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    case EResourceAccess::Transfer: return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case EResourceAccess::ColorAttachment: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case EResourceAccess::Present: return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case EResourceAccess::Host: return VK_PIPELINE_STAGE_HOST_BIT;
    case EResourceAccess::IndirectBuffer: return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    case EResourceAccess::IndexBuffer: return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case EResourceAccess::VertexBuffer: return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case EResourceAccess::ConditionalRendering: return VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
#ifdef VK_NV_device_generated_commands
    case EResourceAccess::CommandProcess: return VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
#elif defined(VK_NVX_device_generated_commands)
    case EResourceAccess::CommandProcess: return VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX;
#endif
#ifdef VK_NV_shading_rate_image
    case EResourceAccess::ShadingRateImage: return VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV;
#endif
#ifdef VK_NV_ray_tracing
    case EResourceAccess::BuildRayTracingAS:
    case EResourceAccess::RTASBuildingBuffer: return VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV;
#endif

    case EResourceAccess::ShaderStorage:
    case EResourceAccess::Uniform:
    case EResourceAccess::ShaderSample: {
        Assert(value.ShaderStages != Zero);
        VkPipelineStageFlagBits result = Zero;
        if (value & EResourceShaderStages::VertexShader) result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        if (value & EResourceShaderStages::TessControlShader) result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
        if (value & EResourceShaderStages::TessEvaluationShader) result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        if (value & EResourceShaderStages::GeometryShader) result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        if (value & EResourceShaderStages::FragmentShader) result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if (value & EResourceShaderStages::ComputeShader) result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef VK_NV_mesh_shader
        if (value & EResourceShaderStages::MeshTaskShader) result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
        if (value & EResourceShaderStages::MeshShader) result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;
#endif
#ifdef VK_NV_ray_tracing
        if (value & EResourceShaderStages::RayTracingShader) result |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
#endif
        return result;
    }

    case EResourceAccess::DepthStencilAttachment: {
        Assert(value ^ (EResourceFlags::EarlyFragmentTests | EResourceFlags::LateFragmentTests));
        VkPipelineStageFlagBits result = Zero;
        if (value & EResourceFlags::EarlyFragmentTests) result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        if (value & EResourceFlags::LateFragmentTests) result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        return result;
    }

    default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkAccessFlagBits EResourceState_ToAccess(EResourceState value) {
    switch (value.OnlyState().Ord()) {
    case EResourceState_Unknown.Ord(): return Zero;

    case EResourceState_UniformRead.Ord(): return VK_ACCESS_UNIFORM_READ_BIT;
    case EResourceState_ShaderSample.Ord():
    case EResourceState_ShaderRead.Ord(): return VK_ACCESS_SHADER_READ_BIT;
    case EResourceState_ShaderWrite.Ord(): return VK_ACCESS_SHADER_WRITE_BIT;
    case EResourceState_ShaderReadWrite.Ord(): return (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
    case EResourceState_InputAttachment.Ord(): return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    case EResourceState_TransferSrc.Ord(): return VK_ACCESS_TRANSFER_READ_BIT;
    case EResourceState_TransferDst.Ord(): return VK_ACCESS_TRANSFER_WRITE_BIT;
    case EResourceState_ColorAttachmentRead.Ord(): return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    case EResourceState_ColorAttachmentWrite.Ord(): return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case EResourceState_ColorAttachmentReadWrite.Ord(): return (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    case EResourceState_DepthStencilAttachmentRead.Ord(): return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case EResourceState_DepthStencilAttachmentWrite.Ord(): return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case EResourceState_DepthStencilAttachmentReadWrite.Ord(): return (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    case EResourceState_HostRead.Ord(): return VK_ACCESS_HOST_READ_BIT;
    case EResourceState_HostWrite.Ord(): return VK_ACCESS_HOST_WRITE_BIT;
    case EResourceState_HostReadWrite.Ord(): return (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT);
    case EResourceState_IndirectBuffer.Ord(): return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case EResourceState_IndexBuffer.Ord(): return VK_ACCESS_INDEX_READ_BIT;
    case EResourceState_VertexBuffer.Ord(): return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case EResourceState_PresentImage.Ord(): return Zero;

#ifdef VK_NV_shading_rate_image
    case EResourceState_ShadingRateImageRead.Ord(): return VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
#endif
#ifdef VK_NV_ray_tracing
    case EResourceState_BuildRayTracingStructRead.Ord(): return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    case EResourceState_BuildRayTracingStructWrite.Ord(): return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
    case EResourceState_BuildRayTracingStructReadWrite.Ord(): return (VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV);
#endif

    case EResourceState_RTASBuildingBufferRead.Ord(): // cache invalidation is not needed for buffers
    case EResourceState_RTASBuildingBufferReadWrite.Ord(): return Zero;
    default:
        if (value == EResourceState_RayTracingShaderRead) return Zero;
        break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkImageLayout EResourceState_ToImageLayout(EResourceState value, VkImageAspectFlags aspect) {
    switch (value.OnlyState().Ord()) {
    case EResourceState_Unknown.Ord(): return VK_IMAGE_LAYOUT_UNDEFINED;

    case EResourceState_ShaderSample.Ord():
    case EResourceState_InputAttachment.Ord(): return (aspect & VK_IMAGE_ASPECT_COLOR_BIT
        ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL );

    case EResourceState_ShaderRead.Ord():
    case EResourceState_ShaderWrite.Ord():
    case EResourceState_ShaderReadWrite.Ord(): return VK_IMAGE_LAYOUT_GENERAL;

    case EResourceState_TransferSrc.Ord(): return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case EResourceState_TransferDst.Ord(): return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    case EResourceState_ColorAttachmentRead.Ord(): return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case EResourceState_ColorAttachmentWrite.Ord():
    case EResourceState_ColorAttachmentReadWrite.Ord(): return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case EResourceState_DepthStencilAttachmentRead.Ord(): return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    case EResourceState_DepthStencilAttachmentWrite.Ord():
    case EResourceState_DepthStencilAttachmentReadWrite.Ord(): return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: use other layouts too

    case EResourceState_HostRead.Ord():
    case EResourceState_HostWrite.Ord():
    case EResourceState_HostReadWrite.Ord(): return VK_IMAGE_LAYOUT_GENERAL;

    case EResourceState_PresentImage.Ord(): return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
VkShadingRatePaletteEntryNV VkCast(EShadingRatePalette value) {
    switch (value) {
    case EShadingRatePalette::NoInvocations: return VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV;
    case EShadingRatePalette::Block_1x1_16: return VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV;
    case EShadingRatePalette::Block_1x1_8: return VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV;
    case EShadingRatePalette::Block_1x1_4: return VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV;
    case EShadingRatePalette::Block_1x1_2: return VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV;
    case EShadingRatePalette::Block_1x1_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV;
    case EShadingRatePalette::Block_2x1_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV;
    case EShadingRatePalette::Block_1x2_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV;
    case EShadingRatePalette::Block_2x2_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV;
    case EShadingRatePalette::Block_4x2_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV;
    case EShadingRatePalette::Block_2x4_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV;
    case EShadingRatePalette::Block_4x4_1: return VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV;

    case EShadingRatePalette::_Count: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_RHIVULKAN_FOREACH_PIXELFORMAT(_EACH) \
    _EACH(EPixelFormat::RGBA16_SNorm, VK_FORMAT_R16G16B16A16_SNORM) \
    _EACH(EPixelFormat::RGBA8_SNorm, VK_FORMAT_R8G8B8A8_SNORM) \
    _EACH(EPixelFormat::RGB16_SNorm, VK_FORMAT_R16G16B16_SNORM) \
    _EACH(EPixelFormat::RGB8_SNorm, VK_FORMAT_R8G8B8_SNORM) \
    _EACH(EPixelFormat::RG16_SNorm, VK_FORMAT_R16G16_SNORM) \
    _EACH(EPixelFormat::RG8_SNorm, VK_FORMAT_R8G8_SNORM) \
    _EACH(EPixelFormat::R16_SNorm, VK_FORMAT_R16_SNORM) \
    _EACH(EPixelFormat::R8_SNorm, VK_FORMAT_R8_SNORM) \
    _EACH(EPixelFormat::RGBA16_UNorm, VK_FORMAT_R16G16B16A16_UNORM) \
    _EACH(EPixelFormat::RGBA8_UNorm, VK_FORMAT_R8G8B8A8_UNORM) \
    _EACH(EPixelFormat::RGB16_UNorm, VK_FORMAT_R16G16B16_UNORM) \
    _EACH(EPixelFormat::RGB8_UNorm, VK_FORMAT_R8G8B8_UNORM) \
    _EACH(EPixelFormat::RG16_UNorm, VK_FORMAT_R16G16_UNORM) \
    _EACH(EPixelFormat::RG8_UNorm, VK_FORMAT_R8G8_UNORM) \
    _EACH(EPixelFormat::R16_UNorm, VK_FORMAT_R16_UNORM) \
    _EACH(EPixelFormat::R8_UNorm, VK_FORMAT_R8_UNORM) \
    _EACH(EPixelFormat::RGB10_A2_UNorm, VK_FORMAT_A2B10G10R10_UNORM_PACK32) \
    _EACH(EPixelFormat::RGBA4_UNorm, VK_FORMAT_R4G4B4A4_UNORM_PACK16) \
    _EACH(EPixelFormat::RGB5_A1_UNorm, VK_FORMAT_R5G5B5A1_UNORM_PACK16) \
    _EACH(EPixelFormat::RGB_5_6_5_UNorm, VK_FORMAT_R5G6B5_UNORM_PACK16) \
    _EACH(EPixelFormat::BGR8_UNorm, VK_FORMAT_B8G8R8_UNORM) \
    _EACH(EPixelFormat::BGRA8_UNorm, VK_FORMAT_B8G8R8A8_UNORM) \
    _EACH(EPixelFormat::sRGB8, VK_FORMAT_R8G8B8_SRGB) \
    _EACH(EPixelFormat::sRGB8_A8, VK_FORMAT_R8G8B8A8_SRGB) \
    _EACH(EPixelFormat::sBGR8, VK_FORMAT_B8G8R8_SRGB) \
    _EACH(EPixelFormat::sBGR8_A8, VK_FORMAT_B8G8R8A8_SRGB) \
    _EACH(EPixelFormat::R8i, VK_FORMAT_R8_SINT) \
    _EACH(EPixelFormat::RG8i, VK_FORMAT_R8G8_SINT) \
    _EACH(EPixelFormat::RGB8i, VK_FORMAT_R8G8B8_SINT) \
    _EACH(EPixelFormat::RGBA8i, VK_FORMAT_R8G8B8A8_SINT) \
    _EACH(EPixelFormat::R16i, VK_FORMAT_R16_SINT) \
    _EACH(EPixelFormat::RG16i, VK_FORMAT_R16G16_SINT) \
    _EACH(EPixelFormat::RGB16i, VK_FORMAT_R16G16B16_SINT) \
    _EACH(EPixelFormat::RGBA16i, VK_FORMAT_R16G16B16A16_SINT) \
    _EACH(EPixelFormat::R32i, VK_FORMAT_R32_SINT) \
    _EACH(EPixelFormat::RG32i, VK_FORMAT_R32G32_SINT) \
    _EACH(EPixelFormat::RGB32i, VK_FORMAT_R32G32B32_SINT) \
    _EACH(EPixelFormat::RGBA32i, VK_FORMAT_R32G32B32A32_SINT) \
    _EACH(EPixelFormat::R8u, VK_FORMAT_R8_UINT) \
    _EACH(EPixelFormat::RG8u, VK_FORMAT_R8G8_UINT) \
    _EACH(EPixelFormat::RGB8u, VK_FORMAT_R8G8B8_UINT) \
    _EACH(EPixelFormat::RGBA8u, VK_FORMAT_R8G8B8A8_UINT) \
    _EACH(EPixelFormat::R16u, VK_FORMAT_R16_UINT) \
    _EACH(EPixelFormat::RG16u, VK_FORMAT_R16G16_UINT) \
    _EACH(EPixelFormat::RGB16u, VK_FORMAT_R16G16B16_UINT) \
    _EACH(EPixelFormat::RGBA16u, VK_FORMAT_R16G16B16A16_UINT) \
    _EACH(EPixelFormat::R32u, VK_FORMAT_R32_UINT) \
    _EACH(EPixelFormat::RG32u, VK_FORMAT_R32G32_UINT) \
    _EACH(EPixelFormat::RGB32u, VK_FORMAT_R32G32B32_UINT) \
    _EACH(EPixelFormat::RGBA32u, VK_FORMAT_R32G32B32A32_UINT) \
    _EACH(EPixelFormat::RGB10_A2u, VK_FORMAT_A2B10G10R10_UINT_PACK32) \
    _EACH(EPixelFormat::R16f, VK_FORMAT_R16_SFLOAT) \
    _EACH(EPixelFormat::RG16f, VK_FORMAT_R16G16_SFLOAT) \
    _EACH(EPixelFormat::RGB16f, VK_FORMAT_R16G16B16_SFLOAT) \
    _EACH(EPixelFormat::RGBA16f, VK_FORMAT_R16G16B16A16_SFLOAT) \
    _EACH(EPixelFormat::R32f, VK_FORMAT_R32_SFLOAT) \
    _EACH(EPixelFormat::RG32f, VK_FORMAT_R32G32_SFLOAT) \
    _EACH(EPixelFormat::RGB32f, VK_FORMAT_R32G32B32_SFLOAT) \
    _EACH(EPixelFormat::RGBA32f, VK_FORMAT_R32G32B32A32_SFLOAT) \
    _EACH(EPixelFormat::RGB_11_11_10f, VK_FORMAT_B10G11R11_UFLOAT_PACK32) \
    _EACH(EPixelFormat::Depth16, VK_FORMAT_D16_UNORM) \
    _EACH(EPixelFormat::Depth24, VK_FORMAT_X8_D24_UNORM_PACK32) \
    _EACH(EPixelFormat::Depth32f, VK_FORMAT_D32_SFLOAT) \
    _EACH(EPixelFormat::Depth16_Stencil8, VK_FORMAT_D16_UNORM_S8_UINT) \
    _EACH(EPixelFormat::Depth24_Stencil8, VK_FORMAT_D24_UNORM_S8_UINT) \
    _EACH(EPixelFormat::Depth32F_Stencil8, VK_FORMAT_D32_SFLOAT_S8_UINT) \
    _EACH(EPixelFormat::BC1_RGB8_UNorm, VK_FORMAT_BC1_RGB_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC1_sRGB8, VK_FORMAT_BC1_RGB_SRGB_BLOCK) \
    _EACH(EPixelFormat::BC1_RGB8_A1_UNorm, VK_FORMAT_BC1_RGBA_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC1_sRGB8_A1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK) \
    _EACH(EPixelFormat::BC2_RGBA8_UNorm, VK_FORMAT_BC2_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC2_sRGB8_A8, VK_FORMAT_BC2_SRGB_BLOCK) \
    _EACH(EPixelFormat::BC3_RGBA8_UNorm, VK_FORMAT_BC3_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC3_sRGB8_A8, VK_FORMAT_BC3_SRGB_BLOCK) \
    _EACH(EPixelFormat::BC4_R8_SNorm, VK_FORMAT_BC4_SNORM_BLOCK) \
    _EACH(EPixelFormat::BC4_R8_UNorm, VK_FORMAT_BC4_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC5_RG8_SNorm, VK_FORMAT_BC5_SNORM_BLOCK) \
    _EACH(EPixelFormat::BC5_RG8_UNorm, VK_FORMAT_BC5_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC7_RGBA8_UNorm, VK_FORMAT_BC7_UNORM_BLOCK) \
    _EACH(EPixelFormat::BC7_sRGB8_A8, VK_FORMAT_BC7_SRGB_BLOCK) \
    _EACH(EPixelFormat::BC6H_RGB16F, VK_FORMAT_BC6H_SFLOAT_BLOCK) \
    _EACH(EPixelFormat::BC6H_RGB16UF, VK_FORMAT_BC6H_UFLOAT_BLOCK) \
    _EACH(EPixelFormat::ETC2_RGB8_UNorm, VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK) \
    _EACH(EPixelFormat::ECT2_sRGB8, VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK) \
    _EACH(EPixelFormat::ETC2_RGB8_A1_UNorm, VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK) \
    _EACH(EPixelFormat::ETC2_sRGB8_A1, VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK) \
    _EACH(EPixelFormat::ETC2_RGBA8_UNorm, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK) \
    _EACH(EPixelFormat::ETC2_sRGB8_A8, VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK) \
    _EACH(EPixelFormat::EAC_R11_SNorm, VK_FORMAT_EAC_R11_SNORM_BLOCK) \
    _EACH(EPixelFormat::EAC_R11_UNorm, VK_FORMAT_EAC_R11_UNORM_BLOCK) \
    _EACH(EPixelFormat::EAC_RG11_SNorm, VK_FORMAT_EAC_R11G11_SNORM_BLOCK) \
    _EACH(EPixelFormat::EAC_RG11_UNorm, VK_FORMAT_EAC_R11G11_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_4x4, VK_FORMAT_ASTC_4x4_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_5x4, VK_FORMAT_ASTC_5x4_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_5x5, VK_FORMAT_ASTC_5x5_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_6x5, VK_FORMAT_ASTC_6x5_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_6x6, VK_FORMAT_ASTC_6x6_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_8x5, VK_FORMAT_ASTC_8x5_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_8x6, VK_FORMAT_ASTC_8x6_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_8x8, VK_FORMAT_ASTC_8x8_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_10x5, VK_FORMAT_ASTC_10x5_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_10x6, VK_FORMAT_ASTC_10x6_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_10x8, VK_FORMAT_ASTC_10x8_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_10x10, VK_FORMAT_ASTC_10x10_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_12x10, VK_FORMAT_ASTC_12x10_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_RGBA_12x12, VK_FORMAT_ASTC_12x12_UNORM_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_4x4, VK_FORMAT_ASTC_4x4_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_5x4, VK_FORMAT_ASTC_5x4_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_5x5, VK_FORMAT_ASTC_5x5_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_6x5, VK_FORMAT_ASTC_6x5_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_6x6, VK_FORMAT_ASTC_6x6_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_8x5, VK_FORMAT_ASTC_8x5_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_8x6, VK_FORMAT_ASTC_8x6_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_8x8, VK_FORMAT_ASTC_8x8_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_10x5, VK_FORMAT_ASTC_10x5_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_10x6, VK_FORMAT_ASTC_10x6_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_10x8, VK_FORMAT_ASTC_10x8_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_10x10, VK_FORMAT_ASTC_10x10_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_12x10, VK_FORMAT_ASTC_12x10_SRGB_BLOCK) \
    _EACH(EPixelFormat::ASTC_sRGB8_A8_12x12, VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
//----------------------------------------------------------------------------
VkFormat VkCast(EPixelFormat value) {
    switch (value) {
#define PIXELFORMAT_TO_VK(_Rhi, _Vk) case _Rhi: return _Vk;
        PPE_RHIVULKAN_FOREACH_PIXELFORMAT(PIXELFORMAT_TO_VK)
#undef PIXELFORMAT_TO_VK

    case EPixelFormat::_Count:
    case EPixelFormat::Unknown: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
EPixelFormat RHICast(VkFormat value) {
    switch (value) {
#define VK_TO_PIXELFORMAT(_Rhi, _Vk) case _Vk: return _Rhi;
        PPE_RHIVULKAN_FOREACH_PIXELFORMAT(VK_TO_PIXELFORMAT)
#undef VK_TO_PIXELFORMAT

    default: break;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
#undef PPE_RHIVULKAN_FOREACH_PIXELFORMAT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
