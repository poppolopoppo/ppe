#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIFixedFunctionState.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanBlendFactor : u32 {
    Zero = VK_BLEND_FACTOR_ZERO,
    One = VK_BLEND_FACTOR_ONE,
    SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
    OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    DstColor = VK_BLEND_FACTOR_DST_COLOR,
    OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
    OneMinusSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
    OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    ConstantColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
    OneMinusConstantColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    ConstantAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
    OneMinusConstantAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    SrcAlphaSaturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
};
//----------------------------------------------------------------------------
enum class EVulkanBlendOp : u32 {
    Add = VK_BLEND_OP_ADD,
    Subtract = VK_BLEND_OP_SUBTRACT,
    ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
    Min = VK_BLEND_OP_MIN,
    Max = VK_BLEND_OP_MAX,
};
//----------------------------------------------------------------------------
enum class EVulkanColorComponentMask : u32 {
    None = 0,

    R = VK_COLOR_COMPONENT_R_BIT,
    G = VK_COLOR_COMPONENT_G_BIT,
    B = VK_COLOR_COMPONENT_B_BIT,
    A = VK_COLOR_COMPONENT_A_BIT,

    RA = R|A,
    RG = R|G,
    RGB = R|G|B,
    RGBA = R|G|B|A,
};
ENUM_FLAGS(EVulkanColorComponentMask);
//----------------------------------------------------------------------------
enum class EVulkanLogicOp : u32 {
    Clear = VK_LOGIC_OP_CLEAR,
    And = VK_LOGIC_OP_AND,
    AndReverse = VK_LOGIC_OP_AND_REVERSE,
    Copy = VK_LOGIC_OP_COPY,
    AndInverted = VK_LOGIC_OP_AND_INVERTED,
    NoOp = VK_LOGIC_OP_NO_OP,
    Xor = VK_LOGIC_OP_XOR,
    Or = VK_LOGIC_OP_OR,
    Nor = VK_LOGIC_OP_NOR,
    Equivalent = VK_LOGIC_OP_EQUIVALENT,
    Invert = VK_LOGIC_OP_INVERT,
    OrReverse = VK_LOGIC_OP_OR_REVERSE,
    CopyInverted = VK_LOGIC_OP_COPY_INVERTED,
    OrInverted = VK_LOGIC_OP_OR_INVERTED,
    Nand = VK_LOGIC_OP_NAND,
    Set = VK_LOGIC_OP_SET,
};
//----------------------------------------------------------------------------
struct FVulkanBlendAttachmentState {
    bool EnableBlend{ false };

    EVulkanBlendFactor SrcColorBlendFactor{ EVulkanBlendFactor::SrcAlpha };
    EVulkanBlendFactor DstColorBlendFactor{ EVulkanBlendFactor::OneMinusSrcAlpha };
    EVulkanBlendOp ColorBlendOp{ EVulkanBlendOp::Add };
    EVulkanBlendFactor SrcAlphaBlendFactor{ EVulkanBlendFactor::One };
    EVulkanBlendFactor DstAlphaBlendFactor{ EVulkanBlendFactor::Zero };
    EVulkanBlendOp AlphaBlendOp{ EVulkanBlendOp::Add };

    EVulkanColorComponentMask ColorWriteMask{ EVulkanColorComponentMask::RGBA };
};
//----------------------------------------------------------------------------
struct FVulkanBlendState {
    bool EnableLogicOp{ false };

    EVulkanLogicOp LogicOp{ EVulkanLogicOp::Copy };
    float4 BlendConstants{ float4::Zero };

    VECTORINSITU(RHIState, FVulkanBlendAttachmentState, 1) Attachments;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanCompareOp : u32 {
    Never = VK_COMPARE_OP_NEVER,
    Less = VK_COMPARE_OP_LESS,
    Equal = VK_COMPARE_OP_EQUAL,
    LessOrEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
    Greater = VK_COMPARE_OP_GREATER,
    NotEqual = VK_COMPARE_OP_NOT_EQUAL,
    GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
    Always = VK_COMPARE_OP_ALWAYS,
};
//----------------------------------------------------------------------------
enum class EVulkanStencilOp : u32 {
    Keep = VK_STENCIL_OP_KEEP,
    Zero = VK_STENCIL_OP_ZERO,
    Replace = VK_STENCIL_OP_REPLACE,
    IncrementAndClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    DecrementAndClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    Invert = VK_STENCIL_OP_INVERT,
    IncrementAndWrap = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    DecrementAndWrap = VK_STENCIL_OP_DECREMENT_AND_WRAP,
};
//----------------------------------------------------------------------------
struct FVulkanStencilOpState {
    EVulkanStencilOp FailOp;
    EVulkanStencilOp PassOp;
    EVulkanStencilOp DepthFailOp;
    EVulkanStencilOp CompareOp;

    u32 CompareMask{ 0 };
    u32 WriteMask{ 0 };
    u32 Reference{ 0 };
};
//----------------------------------------------------------------------------
struct FVulkanDepthStencilState {
    bool EnableDepthBoundsTest{ false };
    bool EnableDepthClipTest{ false };
    bool EnableDepthStencilTest{ false };
    bool EnableDepthTest{ false };
    bool EnableDepthWrite{ false };

    EVulkanCompareOp DepthCompareOp{ EVulkanCompareOp::LessOrEqual };

    FVulkanStencilOpState Front;
    FVulkanStencilOpState Back;

    float MinDepthBounds{ 0.f };
    float MaxDepthBounds{ 1.f };

    VECTORINSITU(RHIState, float4, 1) ClipPlanes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanMultisampleState {
    bool EnableSampleShading{ false };
    bool EnableAlphaToCoverage{ false };
    bool EnableAlphaToOne{ false };

    float MinSampleShading{ 1.f };

    VECTORINSITU(RHIState, u32, 4) SampleMasks;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanCullMode : u32 {
    None = VK_CULL_MODE_NONE,
    Back = VK_CULL_MODE_BACK_BIT,
    Front = VK_CULL_MODE_FRONT_BIT,
    FrontAndBack = VK_CULL_MODE_BACK_BIT|VK_CULL_MODE_FRONT_BIT,
};
//----------------------------------------------------------------------------
enum class EVulkanFrontFace : u32 {
    CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    Clockwise = VK_FRONT_FACE_CLOCKWISE,
};
//----------------------------------------------------------------------------
enum class EVulkanPolygonMode : u32 {
    Fill = VK_POLYGON_MODE_FILL,
    Line = VK_POLYGON_MODE_LINE,
    Point = VK_POLYGON_MODE_POINT,
    FillRectangle = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
};
//----------------------------------------------------------------------------
enum class EVulkanConservativeRasterizationMode : u32 {
    Disabled = VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT,
    OverEstimate = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT,
    UnderEstimate = VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT,
};
//----------------------------------------------------------------------------
struct FVulkanRasterizerState {
    EVulkanCullMode CullMode{ EVulkanCullMode::Back };
    EVulkanFrontFace FrontFace{ EVulkanFrontFace::CounterClockwise };
    EVulkanPolygonMode PolygonMode{ EVulkanPolygonMode::Fill };
    EVulkanConservativeRasterizationMode ConservativeMode{ EVulkanConservativeRasterizationMode::Disabled };

    bool EnableDepthBias{ false };;
    bool EnableDepthClamp{ false };
    bool EnableRasterizerDiscard{ false };
    bool EnableOutOfOrderRasterization{ false };

    float DepthBiasConstantFactor{ 0.f };
    float DepthBiasClamp{ 0.f };
    float DepthBiasSlopeFactor{ 0.f };
    float ExtraPrimitiveOverEstimationSize{ 0.f };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanDynamicState : u32 {
    None                     = 0,
    Viewport                 = 1<<VK_DYNAMIC_STATE_VIEWPORT,
    Scissor                  = 1<<VK_DYNAMIC_STATE_SCISSOR,
    LineWidth                = 1<<VK_DYNAMIC_STATE_LINE_WIDTH,
    DepthBias                = 1<<VK_DYNAMIC_STATE_DEPTH_BIAS,
    BlendConstants           = 1<<VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    DepthBounds              = 1<<VK_DYNAMIC_STATE_DEPTH_BOUNDS,
    StencilCompareMask       = 1<<VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
    StencilWriteMask         = 1<<VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
    StencilReference         = 1<<VK_DYNAMIC_STATE_STENCIL_REFERENCE,
};
ENUM_FLAGS(EVulkanDynamicState);
//----------------------------------------------------------------------------
struct FVulkanFixedFunctionState {
    EVulkanDynamicState DynamicStates{ EVulkanDynamicState::None };

    FVulkanBlendState Blend;
    FVulkanDepthStencilState DepthStencil;
    FVulkanMultisampleState MultiSample;
    FVulkanRasterizerState Rasterizer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
