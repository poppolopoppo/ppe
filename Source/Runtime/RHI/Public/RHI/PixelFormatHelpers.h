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

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, u32 bpp, const uint2& dim, u32 channels, EPixelValueType type, EImageAspect aspect = EImageAspect::Color) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BlockDim(dim), BitsPerBlock0(bpp), Channels(channels)
    {}

    CONSTEXPR FPixelFormatInfo(EPixelFormat fmt, const uint2& bppDepthStencil, EPixelValueType type = EPixelValueType::DepthStencil, EImageAspect aspect = EImageAspect::DepthStencil) NOEXCEPT
        : Format(fmt), AspectMask(aspect), ValueType(type), BitsPerBlock0(bppDepthStencil.x), BitsPerBlock1(bppDepthStencil.y)
    {}
};
//----------------------------------------------------------------------------
PPE_RHI_API FPixelFormatInfo EPixelFormat_Infos(EPixelFormat fmt) NOEXCEPT;
//----------------------------------------------------------------------------
inline u32 EPixelFormat_BitsPerPixel(EPixelFormat fmt, EImageAspect aspect) {
    const auto info = EPixelFormat_Infos(fmt);
    Assert(info.AspectMask ^ aspect);
    return (aspect != EImageAspect::Stencil
        ? info.BitsPerBlock0 / (info.BlockDim.x * info.BlockDim.y)
        : info.BitsPerBlock1 / (info.BlockDim.x * info.BlockDim.y) );
}
//----------------------------------------------------------------------------
inline EImageAspect EPixelFormat_ToImageAspect(EPixelFormat fmt) {
    const auto info = EPixelFormat_Infos(fmt);
    const EImageAspect color = (not (info.ValueType ^ EPixelValueType::DepthStencil) ? EImageAspect::Color : Default);
    const EImageAspect depth = (info.ValueType ^ EPixelValueType::Depth ? EImageAspect::Depth : Default);
    const EImageAspect stencil = (info.ValueType ^ EPixelValueType::Stencil ? EImageAspect::Stencil : Default);
    return (color | depth | stencil);
}
//----------------------------------------------------------------------------
// IsXXX
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsDepth(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::Depth);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::Stencil);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsDepthStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType == EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_IsColor(EPixelFormat fmt) {
    return not (EPixelFormat_Infos(fmt).ValueType ^ EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
// HasXXX
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasDepth(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Depth);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType & EPixelValueType::Stencil);
}
//----------------------------------------------------------------------------
inline bool EPixelFormat_HasDepthOrStencil(EPixelFormat fmt) {
    return (EPixelFormat_Infos(fmt).ValueType ^ EPixelValueType::DepthStencil);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
