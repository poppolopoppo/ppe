#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EPixelValueType : u32 {
    SFloat          = 1 << 0,
    UFloat          = 1 << 1,
    UNorm           = 1 << 2,
    SNorm           = 1 << 3,
    AnyFloat        = SFloat|UFloat|UNorm|SNorm,
    Int             = 1 << 4,
    UInt            = 1 << 5,
    Depth           = 1 << 6,
    Stencil         = 1 << 7,
    DepthStencil    = Depth | Stencil,
    _ValueMask      = 0xFFFF,

    // flags
    sRGB            = 1 << 16,

    Unknown         = 0
};
ENUM_FLAGS(EPixelValueType);
//----------------------------------------------------------------------------
struct FPixelFormatInfo {

    EPixelFormat Format{ Default };
    EImageAspect AspectMask{ Default };
    EPixelValueType ValueType{ Default };
    uint2 BlockDim{ 1 };
    u32 BitsPerBlock0{ 0 }; // color / depth
    u32 BitsPerBlock1{ 0 }; // stencil
    u32 Channels{ 0 };

    FPixelFormatInfo() = default;

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color)
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, const uint2& dim, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color)
        : Format(fmt), AspectMask(aspect), ValueType(type), BlockDim(dim), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, const uint2& bppDepthStencil, EPixelValueType type = EPixelValueType::DepthStencil, EImageAspect aspect = EImageAspect::DepthStencil)
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bppDepthStencil.x), BitsPerBlock1(bppDepthStencil.y)
    {}
};
//----------------------------------------------------------------------------
CONSTEXPR FPixelFormatInfo EPixelFormat_Infos(EPixelFormat fmt) {
    using EType = EPixelValueType;
    switch (fmt) {
    case EPixelFormat::RGBA16_SNorm:                return { fmt,16*4,  4, EType::SNorm };
    case EPixelFormat::RGBA8_SNorm:                 return { fmt,8*4,   4, EType::SNorm };
    case EPixelFormat::RGB16_SNorm:                 return { fmt,16*3,  3, EType::SNorm };
    case EPixelFormat::RGB8_SNorm:                  return { fmt,8*3,   3, EType::SNorm };
    case EPixelFormat::RG16_SNorm:                  return { fmt,16*2,  2, EType::SNorm };
    case EPixelFormat::RG8_SNorm:                   return { fmt,8*2,   2, EType::SNorm };
    case EPixelFormat::R16_SNorm:                   return { fmt,16*1,  1, EType::SNorm };
    case EPixelFormat::R8_SNorm:                    return { fmt,8*1,   1, EType::SNorm };
    case EPixelFormat::RGBA16_UNorm:                return { fmt,16*4,  4, EType::UNorm };
    case EPixelFormat::RGBA8_UNorm:                 return { fmt,8*4,   4, EType::UNorm };
    case EPixelFormat::RGB16_UNorm:                 return { fmt,16*3,  3, EType::UNorm };
    case EPixelFormat::RGB8_UNorm:                  return { fmt,8*3,   3, EType::UNorm };
    case EPixelFormat::RG16_UNorm:                  return { fmt,16*2,  2, EType::UNorm };
    case EPixelFormat::RG8_UNorm:                   return { fmt,8*2,   2, EType::UNorm };
    case EPixelFormat::R16_UNorm:                   return { fmt,16*1,  1, EType::UNorm };
    case EPixelFormat::R8_UNorm:                    return { fmt,8*1,   1, EType::UNorm };
    case EPixelFormat::RGB10_A2_UNorm:              return { fmt,10*3+2,4, EType::UNorm };
    case EPixelFormat::RGBA4_UNorm:                 return { fmt,4*4,   4, EType::UNorm };
    case EPixelFormat::RGB5_A1_UNorm:               return { fmt,5*3+1, 4, EType::UNorm };
    case EPixelFormat::RGB_5_6_5_UNorm:             return { fmt,5+6+5, 3, EType::UNorm };
    case EPixelFormat::BGR8_UNorm:                  return { fmt,8*3,   3, EType::UNorm };
    case EPixelFormat::BGRA8_UNorm:                 return { fmt,8*4,   4, EType::UNorm };
    case EPixelFormat::sRGB8:                       return { fmt,8*3,   3, EType::UNorm | EType::sRGB };
    case EPixelFormat::sRGB8_A8:                    return { fmt,8*4,   4, EType::UNorm | EType::sRGB };
    case EPixelFormat::sBGR8:                       return { fmt,8*3,   3, EType::UNorm | EType::sRGB };
    case EPixelFormat::sBGR8_A8:                    return { fmt,8*4,   4, EType::UNorm | EType::sRGB };
    case EPixelFormat::R8i:                         return { fmt,8*1,   1, EType::Int };
    case EPixelFormat::RG8i:                        return { fmt,8*2,   2, EType::Int };
    case EPixelFormat::RGB8i:                       return { fmt,8*3,   3, EType::Int };
    case EPixelFormat::RGBA8i:                      return { fmt,8*4,   4, EType::Int };
    case EPixelFormat::R16i:                        return { fmt,16*1,  1, EType::Int };
    case EPixelFormat::RG16i:                       return { fmt,16*2,  2, EType::Int };
    case EPixelFormat::RGB16i:                      return { fmt,16*3,  3, EType::Int };
    case EPixelFormat::RGBA16i:                     return { fmt,16*4,  4, EType::Int };
    case EPixelFormat::R32i:                        return { fmt,32*1,  1, EType::Int };
    case EPixelFormat::RG32i:                       return { fmt,32*2,  2, EType::Int };
    case EPixelFormat::RGB32i:                      return { fmt,32*3,  3, EType::Int };
    case EPixelFormat::RGBA32i:                     return { fmt,32*4,  4, EType::Int };
    case EPixelFormat::R8u:                         return { fmt,8*1,   1, EType::UInt };
    case EPixelFormat::RG8u:                        return { fmt,8*2,   2, EType::UInt };
    case EPixelFormat::RGB8u:                       return { fmt,8*3,   3, EType::UInt };
    case EPixelFormat::RGBA8u:                      return { fmt,8*4,   4, EType::UInt };
    case EPixelFormat::R16u:                        return { fmt,16*1,  1, EType::UInt };
    case EPixelFormat::RG16u:                       return { fmt,16*2,  2, EType::UInt };
    case EPixelFormat::RGB16u:                      return { fmt,16*3,  3, EType::UInt };
    case EPixelFormat::RGBA16u:                     return { fmt,16*4,  4, EType::UInt };
    case EPixelFormat::R32u:                        return { fmt,32*1,  1, EType::UInt };
    case EPixelFormat::RG32u:                       return { fmt,32*2,  2, EType::UInt };
    case EPixelFormat::RGB32u:                      return { fmt,32*3,  3, EType::UInt };
    case EPixelFormat::RGBA32u:                     return { fmt,32*4,  4, EType::UInt };
    case EPixelFormat::RGB10_A2u:                   return { fmt,10*3+2,4, EType::UInt };
    case EPixelFormat::R16f:                        return { fmt,16*1,  1, EType::SFloat };
    case EPixelFormat::RG16f:                       return { fmt,16*2,  2, EType::SFloat };
    case EPixelFormat::RGB16f:                      return { fmt,16*3,  3, EType::SFloat };
    case EPixelFormat::RGBA16f:                     return { fmt,16*4,  4, EType::SFloat };
    case EPixelFormat::R32f:                        return { fmt,32*1,  1, EType::SFloat };
    case EPixelFormat::RG32f:                       return { fmt,32*2,  2, EType::SFloat };
    case EPixelFormat::RGB32f:                      return { fmt,32*3,  3, EType::SFloat };
    case EPixelFormat::RGBA32f:                     return { fmt,32*4,  4, EType::SFloat };
    case EPixelFormat::RGB_11_11_10f:               return { fmt,11+11+10,3, EType::SFloat };
    case EPixelFormat::Depth16:                     return { fmt,{16, 0}, EType::UNorm  | EType::Depth, EImageAspect::Depth };
    case EPixelFormat::Depth24:                     return { fmt,{24, 0}, EType::UNorm  | EType::Depth, EImageAspect::Depth };
    case EPixelFormat::Depth32f:                    return { fmt,{32, 0}, EType::SFloat | EType::Depth, EImageAspect::Depth };
    case EPixelFormat::Depth16_Stencil8:            return { fmt,{16, 8}, EType::UNorm  | EType::DepthStencil, EImageAspect::DepthStencil };
    case EPixelFormat::Depth24_Stencil8:            return { fmt,{24, 8}, EType::UNorm  | EType::DepthStencil, EImageAspect::DepthStencil };
    case EPixelFormat::Depth32F_Stencil8:           return { fmt,{32, 8}, EType::SFloat | EType::DepthStencil, EImageAspect::DepthStencil };
    case EPixelFormat::BC1_RGB8_UNorm:              return { fmt,64,    {4,4},  3, EType::UNorm };
    case EPixelFormat::BC1_sRGB8:                   return { fmt,64,    {4,4},  3, EType::UNorm | EType::sRGB };
    case EPixelFormat::BC1_RGB8_A1_UNorm:           return { fmt,64,    {4,4},  4, EType::UNorm };
    case EPixelFormat::BC1_sRGB8_A1:                return { fmt,64,    {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::BC2_RGBA8_UNorm:             return { fmt,128,   {4,4},  4, EType::UNorm };
    case EPixelFormat::BC2_sRGB8_A8:                return { fmt,128,   {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::BC3_RGBA8_UNorm:             return { fmt,128,   {4,4},  4, EType::UNorm };
    case EPixelFormat::BC3_sRGB8:                   return { fmt,128,   {4,4},  3, EType::UNorm | EType::sRGB };
    case EPixelFormat::BC4_R8_SNorm:                return { fmt,64,    {4,4},  1, EType::SNorm };
    case EPixelFormat::BC4_R8_UNorm:                return { fmt,64,    {4,4},  1, EType::UNorm };
    case EPixelFormat::BC5_RG8_SNorm:               return { fmt,128,   {4,4},  2, EType::SNorm };
    case EPixelFormat::BC5_RG8_UNorm:               return { fmt,128,   {4,4},  2, EType::UNorm };
    case EPixelFormat::BC7_RGBA8_UNorm:             return { fmt,128,   {4,4},  4, EType::UNorm };
    case EPixelFormat::BC7_sRGB8_A8:                return { fmt,128,   {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::BC6H_RGB16F:                 return { fmt,128,   {4,4},  3, EType::SFloat };
    case EPixelFormat::BC6H_RGB16UF:                return { fmt,128,   {4,4},  3, EType::UInt };
    case EPixelFormat::ETC2_RGB8_UNorm:             return { fmt,64,    {4,4},  3, EType::UNorm };
    case EPixelFormat::ECT2_sRGB8:                  return { fmt,64,    {4,4},  3, EType::UNorm | EType::sRGB };
    case EPixelFormat::ETC2_RGB8_A1_UNorm:          return { fmt,64,    {4,4},  4, EType::UNorm };
    case EPixelFormat::ETC2_sRGB8_A1:               return { fmt,64,    {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ETC2_RGBA8_UNorm:            return { fmt,128,   {4,4},  4, EType::UNorm };
    case EPixelFormat::ETC2_sRGB8_A8:               return { fmt,128,   {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::EAC_R11_SNorm:               return { fmt,64,    {4,4},  1, EType::SNorm };
    case EPixelFormat::EAC_R11_UNorm:               return { fmt,64,    {4,4},  1, EType::UNorm };
    case EPixelFormat::EAC_RG11_SNorm:              return { fmt,128,   {4,4},  2, EType::SNorm };
    case EPixelFormat::EAC_RG11_UNorm:              return { fmt,128,   {4,4},  2, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_4x4:               return { fmt,128,   {4,4},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_5x4:               return { fmt,128,   {5,4},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_5x5:               return { fmt,128,   {5,5},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_6x5:               return { fmt,128,   {6,5},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_6x6:               return { fmt,128,   {6,6},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_8x5:               return { fmt,128,   {8,5},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_8x6:               return { fmt,128,   {8,6},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_8x8:               return { fmt,128,   {8,8},  4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_10x5:              return { fmt,128,   {10,5}, 4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_10x6:              return { fmt,128,   {10,6}, 4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_10x8:              return { fmt,128,   {10,8}, 4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_10x10:             return { fmt,128,   {10,10},4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_12x10:             return { fmt,128,   {12,10},4, EType::UNorm };
    case EPixelFormat::ASTC_RGBA_12x12:             return { fmt,128,   {12,12},4, EType::UNorm };
    case EPixelFormat::ASTC_sRGB8_A8_4x4:           return { fmt,128,   {4,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_5x4:           return { fmt,128,   {5,4},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_5x5:           return { fmt,128,   {5,5},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_6x5:           return { fmt,128,   {6,5},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_6x6:           return { fmt,128,   {6,6},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_8x5:           return { fmt,128,   {8,5},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_8x6:           return { fmt,128,   {8,6},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_8x8:           return { fmt,128,   {8,8},  4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_10x5:          return { fmt,128,   {10,5}, 4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_10x6:          return { fmt,128,   {10,6}, 4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_10x8:          return { fmt,128,   {10,8}, 4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_10x10:         return { fmt,128,   {10,10},4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_12x10:         return { fmt,128,   {12,10},4, EType::UNorm | EType::sRGB };
    case EPixelFormat::ASTC_sRGB8_A8_12x12:         return { fmt,128,   {12,12},4, EType::UNorm | EType::sRGB };

    default: return { EPixelFormat::Unknown, 0,0, EType::Unknown };
    }
}
//----------------------------------------------------------------------------
CONSTEXPR u32 EPixelFormat_BitsPerPixel(EPixelFormat fmt, EImageAspect aspect) {
    const auto info = EPixelFormat_Infos(fmt);
    Assert(info.AspectMask ^ aspect);
    return (aspect != EImageAspect::Stencil
        ? info.BitsPerBlock0 / (info.BlockDim.x * info.BlockDim.y)
        : info.BitsPerBlock1 / (info.BlockDim.x * info.BlockDim.y) );
}
//----------------------------------------------------------------------------
CONSTEXPR EImageAspect EPixelFormat_ToImageAspect(EPixelFormat fmt) {
    const auto info = EPixelFormat_Infos(fmt);
    const EImageAspect color = (not (info.ValueType ^ EPixelValueType::DepthStencil) ? EImageAspect::Color : Default);
    const EImageAspect depth = (info.ValueType ^ EPixelValueType::Depth ? EImageAspect::Depth : Default);
    const EImageAspect stencil = (info.ValueType ^ EPixelValueType::Stencil ? EImageAspect::Stencil : Default);
    return (color | depth | stencil);
}
//----------------------------------------------------------------------------
// IsXXX
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_IsDepth(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::Depth);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_IsStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::Stencil);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_IsDepthStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_IsColor(EPixelFormat fmt) {
    return not (EPixelFormat_Infos(fmt).ValueType ^ EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
// HasXXX
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_HasDepth(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Depth);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_HasStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Stencil);
}
//----------------------------------------------------------------------------
CONSTEXPR bool EPixelFormat_HasDepthOrStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType ^ EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FPixelDecodeRGBA32f = void (*)(FRgba32f* __restrict, const FRawMemoryConst&) NOEXCEPT;
using FPixelDecodeRGBA32i = void (*)(FRgba32i* __restrict, const FRawMemoryConst&) NOEXCEPT;
using FPixelDecodeRGBA32u = void (*)(FRgba32u* __restrict, const FRawMemoryConst&) NOEXCEPT;
//----------------------------------------------------------------------------
using FPixelEncodeRGBA32f = void (*)(FRawMemory, const FRgba32f&) NOEXCEPT;
using FPixelEncodeRGBA32i = void (*)(FRawMemory, const FRgba32i&) NOEXCEPT;
using FPixelEncodeRGBA32u = void (*)(FRawMemory, const FRgba32u&) NOEXCEPT;
//----------------------------------------------------------------------------
struct FPixelFormatEncoding {
    u32 BitsPerPixel{ 0 };

    FPixelDecodeRGBA32f DecodeRGBA32f{ nullptr };
    FPixelDecodeRGBA32i DecodeRGBA32i{ nullptr };
    FPixelDecodeRGBA32u DecodeRGBA32u{ nullptr };

    FPixelEncodeRGBA32f EncodeRGBA32f{ nullptr };
    FPixelEncodeRGBA32i EncodeRGBA32i{ nullptr };
    FPixelEncodeRGBA32u EncodeRGBA32u{ nullptr };
};
//----------------------------------------------------------------------------
PPE_RHI_API FPixelFormatEncoding EPixelFormat_Encoding(EPixelFormat format, EImageAspect aspect) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
