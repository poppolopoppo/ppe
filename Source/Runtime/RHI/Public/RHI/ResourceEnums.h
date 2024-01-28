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
enum class EQueueType : u8 {
    Graphics,           // also supports compute and transfer commands
    AsyncCompute,       // separate compute queue
    AsyncTransfer,      // separate transfer queue
    _Count,
    Unknown             = UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EQueueUsage : u8 {
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
enum class EMemoryType : u8 {
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
    //ShaderAccess        = 1 << 10,  // shader device address // #TODO: not supported yet

    // special flags for IsSupported() method
    VertexPplnStore     = 1 << 11,  // same as 'Storage', storage buffer store and atomic operations in vertex, geometry, tessellation shaders
    FragmentPplnStore   = 1 << 12,  // same as 'Storage', storage buffer store and atomic operations in fragment shader
    StorageTexelAtomic  = 1 << 13,  // same as 'StorageTexel', atomic ops on imageBuffer
    _Last,

    All                 = ((_Last-1) << 1) - 1,
    Transfer            = TransferDst | TransferSrc,
    Unknown             = 0,
};
ENUM_FLAGS(EBufferUsage);
//----------------------------------------------------------------------------
// Device attachment
//----------------------------------------------------------------------------
enum class EAttachmentLoadOp : u8 {
    Invalidate,
    Load,
    Clear,
    Keep,
    Unknown = UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EAttachmentStoreOp : u8 {
    Invalidate,
    Store,
    Keep,
    Unknown = UINT8_MAX,
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
enum class EPixelFormat : u8 {
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
    Unknown = UINT8_MAX,
};
//----------------------------------------------------------------------------
enum class EColorSpace : u8 {
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

    Unknown = UINT8_MAX,
};
//----------------------------------------------------------------------------
// Device Image
//----------------------------------------------------------------------------
enum class EImageDim : u8 {
    _1D,
    _2D,
    _3D,

    Unknown                 = 0xFFu,
};
inline CONSTEXPR EImageDim EImageDim_1D = EImageDim::_1D;
inline CONSTEXPR EImageDim EImageDim_2D = EImageDim::_2D;
inline CONSTEXPR EImageDim EImageDim_3D = EImageDim::_3D;
//----------------------------------------------------------------------------
enum class EImageView : u8 {
    _1D,
    _2D,
    _3D,
    _1DArray,
    _2DArray,
    _Cube,
    _CubeArray,

    Unknown                 = 0xFFu,
};
inline CONSTEXPR EImageView EImageView_1D = EImageView::_1D;
inline CONSTEXPR EImageView EImageView_2D = EImageView::_2D;
inline CONSTEXPR EImageView EImageView_3D = EImageView::_3D;
inline CONSTEXPR EImageView EImageView_1DArray = EImageView::_1DArray;
inline CONSTEXPR EImageView EImageView_2DArray = EImageView::_2DArray;
inline CONSTEXPR EImageView EImageView_Cube = EImageView::_Cube;
inline CONSTEXPR EImageView EImageView_CubeArray = EImageView::_CubeArray;
//----------------------------------------------------------------------------
enum class EImageFlags : u8 {
    MutableFormat               = 1 << 0,   // allows to change image format
    Array2DCompatible           = 1 << 1,   // allows to create 2D Array view from 3D image
    BlockTexelViewCompatible    = 1 << 2,   // allows to create view with uncompressed format for compressed image
    CubeCompatible              = 1 << 3,   // allows to create CubeMap and CubeMapArray from 2D Array
    _Last,

    Unknown                     = 0,
};
ENUM_FLAGS(EImageFlags);
//----------------------------------------------------------------------------
enum class EImageUsage : u32 {
    TransferSrc                 = 1 << 0,   // for all copy operations
    TransferDst                 = 1 << 1,   // for all copy operations
    Sampled                     = 1 << 2,   // access in shader as texture
    Storage                     = 1 << 3,   // access in shader as image
    ColorAttachment             = 1 << 4,   // color or resolve attachment
    DepthStencilAttachment      = 1 << 5,   // depth/stencil attachment
    TransientAttachment         = 1 << 6,   // color, resolve, depth/stencil, input attachment
    InputAttachment             = 1 << 7,   // input attachment in shader
    ShadingRate                 = 1 << 8,
    //FragmentDensityMap        = 1 << 9,   // #TODO: not supported yet

    // special flags for IsSupported() method
    StorageAtomic               = 1 << 10,  // same as 'Storage', atomic operations on image
    ColorAttachmentBlend        = 1 << 11,  // same as 'ColorAttachment', blend operations on render target
    SampledMinMax               = 1 << 13,  // same as 'Sampled'
    _Last,

    All                         = ((_Last-1) << 1) - 1,
    Transfer                    = TransferDst | TransferSrc,
    Unknown                     = 0,
};
ENUM_FLAGS(EImageUsage);
//----------------------------------------------------------------------------
enum class EImageAspect : u32 {
    Color                       = 1 << 0,
    Depth                       = 1 << 1,
    Stencil                     = 1 << 2,
    Metadata                    = 1 << 3,

    DepthStencil                = Depth | Stencil,
    Auto                        = ~0u,
    Unknown                     = 0,
};
ENUM_FLAGS(EImageAspect);
//----------------------------------------------------------------------------
enum class EImageSampler : u32 {
    // dimension
    _DimOffset                  = FGenericPlatformMaths::FloorLog2(static_cast<u32>(EPixelFormat::_Count)) + 1,
    _DimMask                    = 0xF << _DimOffset,
    _1D                         = 1 << _DimOffset,
    _1DArray                    = 2 << _DimOffset,
    _2D                         = 3 << _DimOffset,
    _2DArray                    = 4 << _DimOffset,
    _2DMS                       = 5 << _DimOffset,
    _2DMSArray                  = 6 << _DimOffset,
    _Cube                       = 7 << _DimOffset,
    _CubeArray                  = 8 << _DimOffset,
    _3D                         = 9 << _DimOffset,

    // type
    _TypeOffset                 = _DimOffset + 4,
    _TypeMask                   = 0xF << _TypeOffset,
    _Float                      = 1 << _TypeOffset,
    _Int                        = 2 << _TypeOffset,
    _UInt                       = 3 << _TypeOffset,

    // flags
    _FlagsOffset                = _TypeOffset + 4,
    _FlagsMask                  = 0xF << _FlagsOffset,
    _Shadow                     = 1 << _FlagsOffset,

    // format
    _FormatMask                 = (1u << _DimOffset) - 1,

    // default
    Float1D                     = _Float | _1D,
    Float1DArray                = _Float | _1DArray,
    Float2D                     = _Float | _2D,
    Float2DArray                = _Float | _2DArray,
    Float2DMS                   = _Float | _2DMS,
    Float2DMSArray              = _Float | _2DMSArray,
    FloatCube                   = _Float | _Cube,
    FloatCubeArray              = _Float | _CubeArray,
    Float3D                     = _Float | _3D,

    Int1D                       = _Int | _1D,
    Int1DArray                  = _Int | _1DArray,
    Int2D                       = _Int | _2D,
    Int2DArray                  = _Int | _2DArray,
    Int2DMS                     = _Int | _2DMS,
    Int2DMSArray                = _Int | _2DMSArray,
    IntCube                     = _Int | _Cube,
    IntCubeArray                = _Int | _CubeArray,
    Int3D                       = _Int | _3D,

    UInt1D                      = _UInt | _1D,
    UInt1DArray                 = _UInt | _1DArray,
    UInt2D                      = _UInt | _2D,
    UInt2DArray                 = _UInt | _2DArray,
    UInt2DMS                    = _UInt | _2DMS,
    UInt2DMSArray               = _UInt | _2DMSArray,
    UIntCube                    = _UInt | _Cube,
    UIntCubeArray               = _UInt | _CubeArray,
    UInt3D                      = _UInt | _3D,

    Unknown                     = ~0u,
};
ENUM_FLAGS(EImageSampler);
//----------------------------------------------------------------------------
CONSTEXPR EImageSampler EImageSampler_FromPixelFormat(EPixelFormat fmt) {
    Assert_NoAssume(Meta::EnumAnd(static_cast<EImageSampler>(Meta::EnumOrd(fmt)), EImageSampler::_FormatMask) ==
        static_cast<EImageSampler>(Meta::EnumOrd(fmt)));
    return static_cast<EImageSampler>(Meta::EnumOrd(fmt));
}
//----------------------------------------------------------------------------
CONSTEXPR EImageSampler operator |(EImageSampler sampler, EPixelFormat fmt) {
    return (sampler | EImageSampler_FromPixelFormat(fmt));
}
//----------------------------------------------------------------------------
CONSTEXPR EPixelFormat EPixelFormat_FromImageSampler(EImageSampler sampler) {
    return static_cast<EPixelFormat>(Meta::EnumAnd(sampler, EImageSampler::_FormatMask));
}
//----------------------------------------------------------------------------
// Fragment output
//----------------------------------------------------------------------------
enum class EFragmentOutput : u8 {
    Unknown                     = static_cast<u32>(EPixelFormat::Unknown),
    Int4                        = static_cast<u32>(EPixelFormat::RGBA32i),
    UInt4                       = static_cast<u32>(EPixelFormat::RGBA32u),
    Float4                      = static_cast<u32>(EPixelFormat::RGBA32f),
};
//----------------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------------
enum class EDebugFlags : u32 {
    LogTasks                    = 1 << 0, //
    LogBarriers                 = 1 << 1, //
    LogResourceUsage            = 1 << 2, //

    VisTasks                    = 1 << 10,
    VisDrawTasks                = 1 << 11,
    VisResources                = 1 << 12,
    VisBarriers                 = 1 << 13,
    VisBarrierLabels            = 1 << 14,
    VisTaskDependencies         = 1 << 15,

    FullBarrier                 = 1u << 30,	// use global memory barrier additionally to per-resource barriers
    QueueSync                   = 1u << 31,	// after each submit wait until queue complete execution

    Unknown                     = 0,

    Verbose                     = LogTasks | LogBarriers | LogResourceUsage |
                                  VisTasks | VisDrawTasks | VisResources | VisBarriers,

#if USE_PPE_RHIDEBUG
    Default                     = Verbose,
#else
    Default                     = Unknown,
#endif
};
ENUM_FLAGS(EDebugFlags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
