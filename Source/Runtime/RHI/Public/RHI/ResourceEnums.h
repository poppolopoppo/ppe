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
};
ENUM_FLAGS(EQueueUsage);
inline CONSTEXPR EQueueUsage EQueueUsage_All{ EQueueUsage::Graphics | EQueueUsage::AsyncCompute | EQueueUsage::AsyncTransfer };
//----------------------------------------------------------------------------
// Device Memory
//----------------------------------------------------------------------------
enum class EMemoryType : u8 {
    Default             = 0, // local in GPU
    HostRead            = 1 << 0,
    HostWrite           = 1 << 1,
    Dedicated           = 1 << 2, // force to use dedicated allocation
};
ENUM_FLAGS(EMemoryType);
inline CONSTEXPR u8 EMemoryType_Last{ u8(EMemoryType::Dedicated) };
inline CONSTEXPR EMemoryType EMemoryType_All{ EMemoryType::HostRead | EMemoryType::HostWrite | EMemoryType::Dedicated };
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

    Unknown             = 0,
};
ENUM_FLAGS(EBufferUsage);
inline CONSTEXPR EBufferUsage EBufferUsage_All{
    EBufferUsage::TransferDst | EBufferUsage::TransferSrc |
    EBufferUsage::UniformTexel | EBufferUsage::StorageTexel |
    EBufferUsage::Uniform | EBufferUsage::Storage |
    EBufferUsage::Index | EBufferUsage::Vertex |
    EBufferUsage::Indirect |
    EBufferUsage::RayTracing |
    EBufferUsage::VertexPplnStore |
    EBufferUsage::FragmentPplnStore |
    EBufferUsage::StorageTexelAtomic };
inline CONSTEXPR EBufferUsage EBufferUsage_Transfer{ EBufferUsage::TransferDst | EBufferUsage::TransferSrc };
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

    Unknown                     = 0,
};
ENUM_FLAGS(EImageUsage);
inline CONSTEXPR EImageUsage EImageUsage_Transfer{ EImageUsage::TransferDst | EImageUsage::TransferSrc };
//----------------------------------------------------------------------------
enum class EImageAspect : u32 {
    Color                       = 1 << 0,
    Depth                       = 1 << 1,
    Stencil                     = 1 << 2,
    Metadata                    = 1 << 3,

    Unknown                     = 0,
};
ENUM_FLAGS(EImageAspect);
inline CONSTEXPR EImageAspect EImageAspect_Auto{~0u};
inline CONSTEXPR EImageAspect EImageAspect_DepthStencil{EImageAspect::Depth | EImageAspect::Stencil};
//----------------------------------------------------------------------------
struct EImageSampler {
    enum EDimension : u8 {
        _1D = 1,
        _1DArray,
        _2D,
        _2DArray,
        _2DMS,
        _2DMSArray,
        _Cube,
        _CubeArray,
        _3D,
    };

    enum EType : u8 {
        _Float = 1,
        _Int,
        _UInt,
    };

    enum EFlags : u8 {
        _Shadow = 1 << 0,
    };
    ENUM_FLAGS_FRIEND(EFlags);

    EPixelFormat    Format{ Default };
    EDimension      Dimension{ Zero };
    EType           Type{ Zero };
    EFlags          Flags{ Zero };

    CONSTEXPR EImageSampler() = default;

    CONSTEXPR EImageSampler(EType type, EDimension dim)
    :   EImageSampler(Zero, dim, type, Zero)
    {}

    CONSTEXPR EImageSampler(EPixelFormat format, EDimension dim, EType type, EFlags flags) {
        Format = format;
        Dimension = dim;
        Type = type;
        Flags = flags;
    }

    CONSTEXPR u32 TypeDim() const {
        return {u32(Type) | (u32(Dimension) << 8u)};
    }

    friend hash_t hash_value(EImageSampler value) NOEXCEPT {
        return hash_as_pod(value);
    }

    CONSTEXPR EImageSampler operator +(EFlags flag) const { return {Format, Dimension, Type, Flags + flag}; }
    CONSTEXPR EImageSampler operator -(EFlags flag) const { return {Format, Dimension, Type, Flags - flag}; }
    CONSTEXPR EImageSampler operator |(EFlags flag) const { return {Format, Dimension, Type, Flags | flag}; }

    CONSTEXPR EImageSampler& operator +=(EFlags flag) { Flags = (Flags + flag); return (*this); }
    CONSTEXPR EImageSampler& operator -=(EFlags flag) { Flags = (Flags - flag); return (*this); }
    CONSTEXPR EImageSampler& operator |=(EFlags flag) { Flags = (Flags | flag); return (*this); }

    CONSTEXPR bool operator &(EFlags flag) const { return (Flags & flag); }
    CONSTEXPR bool operator ^(EFlags flag) const { return (Flags ^ flag); }

    CONSTEXPR bool operator ==(EImageSampler o) const { return (Format == o.Format && Dimension == o.Dimension && Type == o.Type && Flags == o.Flags); }
    CONSTEXPR bool operator !=(EImageSampler o) const { return (not operator ==(o)); }
};
STATIC_ASSERT(sizeof(u32) == sizeof(EImageSampler));
inline CONSTEXPR EImageSampler EImageSampler_Float1D        { EImageSampler::_Float, EImageSampler::_1D };
inline CONSTEXPR EImageSampler EImageSampler_Float1DArray   { EImageSampler::_Float, EImageSampler::_1DArray };
inline CONSTEXPR EImageSampler EImageSampler_Float2D        { EImageSampler::_Float, EImageSampler::_2D };
inline CONSTEXPR EImageSampler EImageSampler_Float2DArray   { EImageSampler::_Float, EImageSampler::_2DArray };
inline CONSTEXPR EImageSampler EImageSampler_Float2DMS      { EImageSampler::_Float, EImageSampler::_2DMS };
inline CONSTEXPR EImageSampler EImageSampler_Float2DMSArray { EImageSampler::_Float, EImageSampler::_2DMSArray };
inline CONSTEXPR EImageSampler EImageSampler_FloatCube      { EImageSampler::_Float, EImageSampler::_Cube };
inline CONSTEXPR EImageSampler EImageSampler_FloatCubeArray { EImageSampler::_Float, EImageSampler::_CubeArray };
inline CONSTEXPR EImageSampler EImageSampler_Float3D        { EImageSampler::_Float, EImageSampler::_3D };
inline CONSTEXPR EImageSampler EImageSampler_Int1D          { EImageSampler::_Int, EImageSampler::_1D };
inline CONSTEXPR EImageSampler EImageSampler_Int1DArray     { EImageSampler::_Int, EImageSampler::_1DArray };
inline CONSTEXPR EImageSampler EImageSampler_Int2D          { EImageSampler::_Int, EImageSampler::_2D };
inline CONSTEXPR EImageSampler EImageSampler_Int2DArray     { EImageSampler::_Int, EImageSampler::_2DArray };
inline CONSTEXPR EImageSampler EImageSampler_Int2DMS        { EImageSampler::_Int, EImageSampler::_2DMS };
inline CONSTEXPR EImageSampler EImageSampler_Int2DMSArray   { EImageSampler::_Int, EImageSampler::_2DMSArray };
inline CONSTEXPR EImageSampler EImageSampler_IntCube        { EImageSampler::_Int, EImageSampler::_Cube };
inline CONSTEXPR EImageSampler EImageSampler_IntCubeArray   { EImageSampler::_Int, EImageSampler::_CubeArray };
inline CONSTEXPR EImageSampler EImageSampler_Int3D          { EImageSampler::_Int, EImageSampler::_3D };
inline CONSTEXPR EImageSampler EImageSampler_UInt1D         { EImageSampler::_UInt, EImageSampler::_1D };
inline CONSTEXPR EImageSampler EImageSampler_UInt1DArray    { EImageSampler::_UInt, EImageSampler::_1DArray };
inline CONSTEXPR EImageSampler EImageSampler_UInt2D         { EImageSampler::_UInt, EImageSampler::_2D };
inline CONSTEXPR EImageSampler EImageSampler_UInt2DArray    { EImageSampler::_UInt, EImageSampler::_2DArray };
inline CONSTEXPR EImageSampler EImageSampler_UInt2DMS       { EImageSampler::_UInt, EImageSampler::_2DMS };
inline CONSTEXPR EImageSampler EImageSampler_UInt2DMSArray  { EImageSampler::_UInt, EImageSampler::_2DMSArray };
inline CONSTEXPR EImageSampler EImageSampler_UIntCube       { EImageSampler::_UInt, EImageSampler::_Cube };
inline CONSTEXPR EImageSampler EImageSampler_UIntCubeArray  { EImageSampler::_UInt, EImageSampler::_CubeArray };
inline CONSTEXPR EImageSampler EImageSampler_UInt3D         { EImageSampler::_UInt, EImageSampler::_3D };
//----------------------------------------------------------------------------
inline CONSTEXPR EImageSampler EImageSampler_FromPixelFormat(EPixelFormat fmt) {
    return {fmt, Zero, Zero, Zero};
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
};
ENUM_FLAGS(EDebugFlags);
inline CONSTEXPR EDebugFlags EDebugFlags_Verbose{
    EDebugFlags::LogTasks |
    EDebugFlags::LogBarriers |
    EDebugFlags::LogResourceUsage |
    EDebugFlags::VisTasks |
    EDebugFlags::VisDrawTasks |
    EDebugFlags::VisResources |
    EDebugFlags::VisBarriers
};
#if USE_PPE_RHIDEBUG
inline CONSTEXPR EDebugFlags EDebugFlags_Default = EDebugFlags_Verbose;
#else
inline CONSTEXPR EDebugFlags EDebugFlags_Default = EDebugFlags_Unknown;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
