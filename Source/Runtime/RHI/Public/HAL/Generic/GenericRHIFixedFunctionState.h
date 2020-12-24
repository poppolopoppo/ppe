#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Container/Vector.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"
#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericBlendFactor : u32 {
    Zero = 0,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
};
//----------------------------------------------------------------------------
enum class EGenericBlendOp : u32 {
    Add = 0,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};
//----------------------------------------------------------------------------
enum class EGenericColorComponentMask : u32 {
    None = 0,

    R = 1<<0,
    G = 1<<1,
    B = 1<<2,
    A = 1<<3,

    RA = R|A,
    RG = R|G,
    RGB = R|G|B,
    RGBA = R|G|B|A,
};
ENUM_FLAGS(EGenericColorComponentMask);
//----------------------------------------------------------------------------
enum class EGenericLogicOp : u32 {
    Clear = 0,
    And,
    AndReverse,
    Copy,
    AndInverted,
    NoOp,
    Xor,
    Or,
    Nor,
    Equivalent,
    Invert,
    OrReverse,
    CopyInverted,
    OrInverted,
    Nand,
    Set,
};
//----------------------------------------------------------------------------
struct FGenericBlendAttachmentState {
    bool EnableBlend{ false };

    EGenericBlendFactor SrcColorBlendFactor{ EGenericBlendFactor::SrcAlpha };
    EGenericBlendFactor DstColorBlendFactor{ EGenericBlendFactor::OneMinusSrcAlpha };
    EGenericBlendOp ColorBlendOp{ EGenericBlendOp::Add };
    EGenericBlendFactor SrcAlphaBlendFactor{ EGenericBlendFactor::One };
    EGenericBlendFactor DstAlphaBlendFactor{ EGenericBlendFactor::Zero };
    EGenericBlendOp AlphaBlendOp{ EGenericBlendOp::Add };

    EGenericColorComponentMask ColorWriteMask{ EGenericColorComponentMask::RGBA };
};
//----------------------------------------------------------------------------
struct FGenericBlendState {
    bool EnableLogicOp{ false };

    EGenericLogicOp LogicOp{ EGenericLogicOp::Copy };
    float4 BlendConstants{ float4::Zero };

    VECTORINSITU(RHIState, FGenericBlendAttachmentState, 1) Attachments;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericCompareOp : u32 {
    Never = 0,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};
//----------------------------------------------------------------------------
enum class EGenericStencilOp : u32 {
    Keep = 0,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap,
};
//----------------------------------------------------------------------------
struct FGenericStencilOpState {
    EGenericStencilOp FailOp;
    EGenericStencilOp PassOp;
    EGenericStencilOp DepthFailOp;
    EGenericStencilOp CompareOp;

    u32 CompareMask{ 0 };
    u32 WriteMask{ 0 };
    u32 Reference{ 0 };
};
//----------------------------------------------------------------------------
struct FGenericDepthStencilState {
    bool EnableDepthBoundsTest{ false };
    bool EnableDepthClipTest{ false };
    bool EnableDepthStencilTest{ false };
    bool EnableDepthTest{ false };
    bool EnableDepthWrite{ false };

    EGenericCompareOp DepthCompareOp{ EGenericCompareOp::LessOrEqual };

    FGenericStencilOpState Front;
    FGenericStencilOpState Back;

    float MinDepthBounds{ 0.f };
    float MaxDepthBounds{ 1.f };

    VECTORINSITU(RHIState, float4, 1) ClipPlanes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericMultisampleState {
    bool EnableSampleShading{ false };
    bool EnableAlphaToCoverage{ false };
    bool EnableAlphaToOne{ false };

    float MinSampleShading{ 1.f };

    VECTORINSITU(RHIState, u32, 4) SampleMasks;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericCullMode : u32 {
    None = 0,
    Back,
    Front,
    FrontAndBack,
};
//----------------------------------------------------------------------------
enum class EGenericFrontFace : u32 {
    CounterClockwise = 0,
    Clockwise,
};
//----------------------------------------------------------------------------
enum class EGenericPolygonMode : u32 {
    Fill = 0,
    Line,
    Point,
    FillRectangle,
};
//----------------------------------------------------------------------------
enum class EGenericConservativeRasterizationMode : u32 {
    Disabled = 0,
    OverEstimate,
    UnderEstimate,
};
//----------------------------------------------------------------------------
struct FGenericRasterizerState {
    EGenericCullMode CullMode{ EGenericCullMode::Back };
    EGenericFrontFace FrontFace{ EGenericFrontFace::CounterClockwise };
    EGenericPolygonMode PolygonMode{ EGenericPolygonMode::Fill };
    EGenericConservativeRasterizationMode ConservativeMode{ EGenericConservativeRasterizationMode::Disabled };

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
enum class EGenericDynamicState : u32 {
    None                     = 0,
    Viewport                 = 1<<0,
    Scissor                  = 1<<1,
    LineWidth                = 1<<2,
    DepthBias                = 1<<3,
    BlendConstants           = 1<<4,
    DepthBounds              = 1<<5,
    StencilCompareMask       = 1<<6,
    StencilWriteMask         = 1<<7,
    StencilReference         = 1<<8,
};
ENUM_FLAGS(EGenericDynamicState);
//----------------------------------------------------------------------------
struct FGenericFixedFunctionState {
    EGenericDynamicState DynamicStates{ EGenericDynamicState::None };

    FGenericBlendState Blend;
    FGenericDepthStencilState DepthStencil;
    FGenericMultisampleState MultiSample;
    FGenericRasterizerState Rasterizer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
