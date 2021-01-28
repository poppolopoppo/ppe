#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Device Queue
//----------------------------------------------------------------------------
enum class EQueueType : u32 {
    Graphics,           // also supports compute and transfer commands
    AsyncCompute,       // separate compute queue
    AsyncTransfer,      // separate transfer queue
    _Count,
    Unknown             = ~0u,
};
//----------------------------------------------------------------------------
enum class EQueueUsage : u32 {
    Unknown = 0,
    Graphics            = u32(1) << u32(EQueueType::Graphics),
    AsyncCompute        = u32(1) << u32(EQueueType::AsyncCompute),
    AsyncTransfer       = u32(1) << u32(EQueueType::AsyncTransfer),
    _Last,
    All                 = ((_Last-1) << 1) - 1,
};
ENUM_FLAGS(EQueueUsage);
//----------------------------------------------------------------------------
// Device Memory
//----------------------------------------------------------------------------
enum class EMemoryType : u32 {
    Default             = 0, // local in GPU
    HostRead            = 1 << 0,
    HostWrite           = 1 << 1,
    Dedicated           = 1 << 2, // force to use dedicated allocation
    _Last
};
ENUM_FLAGS(EMemoryType);
//----------------------------------------------------------------------------
// Device Buffer
//----------------------------------------------------------------------------
enum class EBufferUsage : u32 {
    TransferSrc         = 1 << 0,
    TransferDst         = 1 << 1,
    UniformTexel        = 1 << 2,   // glsl: 'uniform samplerBuffer'
    StorageTexel        = 1 << 3,   // glsl: 'uniform imageBuffer'
    Uniform             = 1 << 4,   // uniform buffer
    Storage             = 1 << 5,   // shader storage buffer
    Index               = 1 << 6,   // index buffer
    Vertex              = 1 << 7,   // vertex buffer
    Indirect            = 1 << 8,   // indirect buffer for draw and dispatch
    RayTracing          = 1 << 9,   // for scratch buffer, instance data, shader binding table
    VertexPplnStore     = 1 << 10,  // storage buffer store and atomic operations in vertex, geometry, tessellation shaders
    FragmentPplnStore   = 1 << 11,  // storage buffer store and atomic operations in fragment shader
    StorageTexelAtomic  = 1 << 12,  // atomic ops on imageBuffer
    _Last,

    All                 = ((_Last-1) << 1) - 1,
    Transfer            = TransferDst | TransferSrc,
    Unknown             = 0,
};
ENUM_FLAGS(EBufferUsage);
//----------------------------------------------------------------------------
// Device Image
//----------------------------------------------------------------------------
enum class EImageType : u32 {
    Tex1D               = 0,
    Tex1DArray,
    Tex2D,
    Tex2DArray,
    Tex2DMS,
    Tex2DMSArray,
    TexCube,
    TexCubeArray,
    Tex3D,
    Unknown             = ~0u,
};
//----------------------------------------------------------------------------
enum class EImageUsage : u32 {
    TransferSrc             = 1 << 0,   // for all copy operations
    TransferDst             = 1 << 1,   // for all copy operations
    Sampled                 = 1 << 2,   // access in shader as texture
    Storage                 = 1 << 3,   // access in shader as image
    ColorAttachment         = 1 << 4,   // color or resolve attachment
    ColorAttachmentBlend    = 1 << 5,   // same as 'ColorAttachment'
    DepthStencilAttachment  = 1 << 6,   // depth/stencil attachment
    TransientAttachment     = 1 << 7,   // color, resolve, depth/stencil, input attachment
    InputAttachment         = 1 << 8,   // input attachment in shader
    ShadingRate             = 1 << 9,
    StorageAtomic           = 1 << 10,  // same as 'Storage'
    _Last,

    All                     = ((_Last-1) << 1) - 1,
    Transfer                = TransferDst | TransferSrc,
    Unknown                 = 0,
};
ENUM_FLAGS(EImageUsage);
//----------------------------------------------------------------------------
enum class EImageAspect : u32 {
    Color               = 1 << 0,
    Depth               = 1 << 1,
    Stencil             = 1 << 2,
    Metadata            = 1 << 3,

    DepthStencil        = Depth | Stencil,
    Auto                = ~0u,
    Unknown             = 0,
};
ENUM_FLAGS(EImageAspect);
//----------------------------------------------------------------------------
enum class ESwapchainImage : u32 {
    Primary,

    // for VR:
    LeftEye,
    RightEye,
};
//----------------------------------------------------------------------------
// Device attachment
//----------------------------------------------------------------------------
enum class EAttachmentLoadOp : u32 {
    Invalidate,
    Load,
    Clear,
    Unknown = ~0u,
};
//----------------------------------------------------------------------------
enum class EAttachmentStoreOp : u32 {
    Invalidate,
    Store,
    Unknown = ~0u,
};

//----------------------------------------------------------------------------
enum class ERenderTargetID : u32 {
    Color0              = 0,
    Color1              = 1,
    Color2              = 2,
    Color3              = 3,
    _LastColor          = MaxColorBuffers - 1,

    DepthStencil        = MaxColorBuffers,
    Depth               = DepthStencil,

    Unkwnown            = ~0u,
};
//----------------------------------------------------------------------------
enum class EShadingRatePalette : u8 {
    NoInvocations       = 0,
    Block_1x1_16        = 1,    // 16 invocations per 1x1 pixel block
    Block_1x1_8         = 2,    //  8 invocations per 1x1 pixel block
    Block_1x1_4         = 3,    //  4 invocations per 1x1 pixel block
    Block_1x1_2         = 4,    //  2 invocations per 1x1 pixel block
    Block_1x1_1         = 5,    //  1 invocation  per 1x1 pixel block
    Block_2x1_1         = 6,    //  ...
    Block_1x2_1         = 7,
    Block_2x2_1         = 8,
    Block_4x2_1         = 9,
    Block_2x4_1         = 10,
    Block_4x4_1         = 11,
    _Count
};
//----------------------------------------------------------------------------
// Pixel Formats
//----------------------------------------------------------------------------
enum class EPixelFormat : u32 {
    // signed normalized
    RGBA16_SNorm,
    RGBA8_SNorm,
    RGB16_SNorm,
    RGB8_SNorm,
    RG16_SNorm,
    RG8_SNorm,
    R16_SNorm,
    R8_SNorm,

    // unsigned normalized
    RGBA16_UNorm,
    RGBA8_UNorm,
    RGB16_UNorm,
    RGB8_UNorm,
    RG16_UNorm,
    RG8_UNorm,
    R16_UNorm,
    R8_UNorm,
    RGB10_A2_UNorm,
    RGBA4_UNorm,
    RGB5_A1_UNorm,
    RGB_5_6_5_UNorm,

    // BGRA
    BGR8_UNorm,
    BGRA8_UNorm,

    // sRGB
    sRGB8,
    sRGB8_A8,
    sBGR8,
    sBGR8_A8,

    // signed integer
    R8i,
    RG8i,
    RGB8i,
    RGBA8i,
    R16i,
    RG16i,
    RGB16i,
    RGBA16i,
    R32i,
    RG32i,
    RGB32i,
    RGBA32i,

    // unsigned integer
    R8u,
    RG8u,
    RGB8u,
    RGBA8u,
    R16u,
    RG16u,
    RGB16u,
    RGBA16u,
    R32u,
    RG32u,
    RGB32u,
    RGBA32u,
    RGB10_A2u,

    // float
    R16f,
    RG16f,
    RGB16f,
    RGBA16f,
    R32f,
    RG32f,
    RGB32f,
    RGBA32f,
    RGB_11_11_10f,

    // depth stencil
    Depth16,
    Depth24,
    Depth32f,
    Depth16_Stencil8,
    Depth24_Stencil8,
    Depth32F_Stencil8,

    // compressed
    BC1_RGB8_UNorm,
    BC1_sRGB8,
    BC1_RGB8_A1_UNorm,
    BC1_sRGB8_A1,
    BC2_RGBA8_UNorm,
    BC2_sRGB8_A8,
    BC3_RGBA8_UNorm,
    BC3_sRGB8,
    BC4_R8_SNorm,
    BC4_R8_UNorm,
    BC5_RG8_SNorm,
    BC5_RG8_UNorm,
    BC7_RGBA8_UNorm,
    BC7_sRGB8_A8,
    BC6H_RGB16F,
    BC6H_RGB16UF,
    ETC2_RGB8_UNorm,
    ECT2_sRGB8,
    ETC2_RGB8_A1_UNorm,
    ETC2_sRGB8_A1,
    ETC2_RGBA8_UNorm,
    ETC2_sRGB8_A8,
    EAC_R11_SNorm,
    EAC_R11_UNorm,
    EAC_RG11_SNorm,
    EAC_RG11_UNorm,
    ASTC_RGBA_4x4,
    ASTC_RGBA_5x4,
    ASTC_RGBA_5x5,
    ASTC_RGBA_6x5,
    ASTC_RGBA_6x6,
    ASTC_RGBA_8x5,
    ASTC_RGBA_8x6,
    ASTC_RGBA_8x8,
    ASTC_RGBA_10x5,
    ASTC_RGBA_10x6,
    ASTC_RGBA_10x8,
    ASTC_RGBA_10x10,
    ASTC_RGBA_12x10,
    ASTC_RGBA_12x12,
    ASTC_sRGB8_A8_4x4,
    ASTC_sRGB8_A8_5x4,
    ASTC_sRGB8_A8_5x5,
    ASTC_sRGB8_A8_6x5,
    ASTC_sRGB8_A8_6x6,
    ASTC_sRGB8_A8_8x5,
    ASTC_sRGB8_A8_8x6,
    ASTC_sRGB8_A8_8x8,
    ASTC_sRGB8_A8_10x5,
    ASTC_sRGB8_A8_10x6,
    ASTC_sRGB8_A8_10x8,
    ASTC_sRGB8_A8_10x10,
    ASTC_sRGB8_A8_12x10,
    ASTC_sRGB8_A8_12x12,

    _Count,
    Unknown = ~0u,
};
//----------------------------------------------------------------------------
enum class EColorSpace : u32 {
    PASS_THROUGH = 0,
    SRGB_NONLINEAR,
    DISPLAY_P3_NONLINEAR,
    EXTENDED_SRGB_LINEAR,
    DISPLAY_P3_LINEAR,
    DCI_P3_NONLINEAR,
    BT709_LINEAR,
    BT709_NONLINEAR,
    BT2020_LINEAR,
    HDR10_ST2084,
    DOLBYVISION,
    HDR10_HLG,
    ADOBERGB_LINEAR,
    ADOBERGB_NONLINEAR,
    EXTENDED_SRGB_NONLINEAR,
    DISPLAY_NATIVE_AMD,

    Unknown = ~0u,
};
//----------------------------------------------------------------------------
// Fragment output
//----------------------------------------------------------------------------
enum class EFragmentOutput : u32 {
    Unknown     = u32(EPixelFormat::Unknown),
    Int4        = u32(EPixelFormat::RGBA32i),
    UInt4       = u32(EPixelFormat::RGBA32u),
    Float4      = u32(EPixelFormat::RGBA32f),
};
//----------------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------------
enum class EDebugFlags : u32 {
    LogTasks            = 1 << 0, //
    LogBarriers         = 1 << 1, //
    LogResourceUsage    = 1 << 2, //

    VisTasks            = 1 << 10,
    VisDrawTasks        = 1 << 11,
    VisResources        = 1 << 12,
    VisBarriers         = 1 << 13,
    VisBarrierLabels    = 1 << 14,
    VisTaskDependencies = 1 << 15,

    FullBarrier         = 1u << 30,	// use global memory barrier additionally to per-resource barriers
    QueueSync           = 1u << 31,	// after each submit wait until queue complete execution

    Unknown             = 0,

    Default             =   LogTasks | LogBarriers | LogResourceUsage |
                            VisTasks | VisDrawTasks | VisResources | VisBarriers,
};
ENUM_FLAGS(EDebugFlags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
