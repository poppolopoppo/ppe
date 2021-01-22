#pragma once

#include "RHI_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBlendFactor : u32 {
    // src - from shader
    // dst - from render target
    // result = srcColor * srcBlend [blendOp] dstColor * dstBlend;
    Zero,                // 0
    One,                 // 1
    SrcColor,            // src
    OneMinusSrcColor,    // 1 - src
    DstColor,            // dst
    OneMinusDstColor,    // 1 - dst
    SrcAlpha,            // src.a
    OneMinusSrcAlpha,    // 1 - src.a
    DstAlpha,            // dst.a
    OneMinusDstAlpha,    // 1 - dst.a
    ConstColor,          // cc
    OneMinusConstColor,  // 1 - cc
    ConstAlpha,          // cc.a
    OneMinusConstAlpha,  // 1 - cc.a
    SrcAlphaSaturate,    // rgb * min( src.a, dst.a ), a * 1

    Src1Color,           //
    OneMinusSrc1Color,   //
    Src1Alpha,           //
    OneMinusSrc1Alpha,   //

    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class EBlendOp : u32 {
    // src - from shader
    // dst - from render target
    // result = srcColor * srcBlend [blendOp] dstColor * dstBlend;
    Add,                // S+D
    Sub,                // S-D
    RevSub,             // D-S
    Min,                // min(S,D)
    Max,                // max(S,D)
    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class ELogicOp : u32 {
    None,               // disabled
    Clear,              // 0
    Set,                // 1
    Copy,               // S
    CopyInverted,       // ~S
    NoOp,               // D
    Invert,             // ~D
    And,                // S & D
    NotAnd,             // ~ ( S & D )
    Or,                 // S | D
    NotOr,              // ~ ( S | D )
    Xor,                // S ^ D
    Equiv,              // ~ ( S ^ D )
    AndReverse,         // S & ~D
    AndInverted,        // ~S & D
    OrReverse,          // S | ~D
    OrInverted,         // ~S | D
    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class EColorMask : u8 {
    R                   = 1<<0,
    G                   = 1<<1,
    B                   = 1<<2,
    A                   = 1<<3,
    RGBA                = R|G|B|A,
    All                 = RGBA,
    Default             = All,
    Unknown             = 0,
};
ENUM_FLAGS(EColorMask);
//----------------------------------------------------------------------------
enum class ECompareOp : u8 {
    Never,              // false
    Less,               // <
    Equal,              // ==
    LessEqual,          // <=
    Greater,            // >
    NotEqual,           // !=
    GreaterEqual,       // >=
    Always,             // true
    Unknown = u8(~0),
};
//----------------------------------------------------------------------------
enum class EStencilOp : u8 {
    Keep,               // s
    Zero,               // 0
    Replace,            // ref
    Incr,               // min( ++s, 0 )
    IncrWrap,           // ++s & maxvalue
    Decr,               // max( --s, 0 )
    DecrWrap,           // --s & maxvalue
    Invert,             // ~s
    Unknown = u8(~0),
};
//----------------------------------------------------------------------------
enum class EPolygonMode : u32 {
    Point,
    Line,
    Fill,
    Unknown	= ~0u,
};
//----------------------------------------------------------------------------
enum class EPrimitiveTopology : u32 {
    Point,

    LineList,
    LineStrip,
    LineListAdjacency,
    LineStripAdjacency,

    TriangleList,
    TriangleStrip,
    TriangleFan,
    TriangleListAdjacency,
    TriangleStripAdjacency,

    Patch,

    _Count,
    Unknown = ~0u,
};
//----------------------------------------------------------------------------
enum class ECullMode : u8
{
    None                    = 0,
    Front                   = 1	<< 0,
    Back                    = 1 << 1,
    FontAndBack             = Front | Back,
    Unknown                 = None,
};
ENUM_FLAGS(ECullMode);
//----------------------------------------------------------------------------
enum class EPipelineDynamicState : u8 {
    Unknown                 = 0,
    Viewport                = 1 << 0,
    Scissor                 = 1 << 1,
    StencilCompareMask      = 1 << 2,
    StencilWriteMask        = 1 << 3,
    StencilReference        = 1 << 4,
    ShadingRatePalette      = 1 << 5,
    _Last,

    All                     = ((_Last-1) << 1) - 1,
    RasterizerMask          = All,
    Default                 = Viewport | Scissor,
};
ENUM_FLAGS(EPipelineDynamicState);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
