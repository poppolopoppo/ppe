// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/TextureEnums.h"

#include "TextureModule.h"
#include "Texture/TextureSource.h"

#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"
#include "RHI/SamplerEnums.h"

// #include "RTTI/Macros-impl.h"

#include "IO/ConstNames.h"
#include "IO/Extname.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, EImageFormat)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_VALUE(PNG)
// RTTI_ENUM_VALUE(BMP)
// RTTI_ENUM_VALUE(TGA)
// RTTI_ENUM_VALUE(JPG)
// RTTI_ENUM_VALUE(HDR)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureSourceCompression)
// RTTI_ENUM_VALUE(None)
// RTTI_ENUM_VALUE(JPG)
// RTTI_ENUM_VALUE(PNG)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_FLAGS_BEGIN(Texture, ETextureSourceFlags)
// RTTI_ENUM_VALUE(HDR)
// RTTI_ENUM_VALUE(LongLatCubemap)
// RTTI_ENUM_VALUE(PreMultipliedAlpha)
// RTTI_ENUM_VALUE(SRGB)
// RTTI_ENUM_VALUE(Tilable)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureSourceFormat)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_VALUE(BGRA8)
// RTTI_ENUM_VALUE(BGRE8)
// RTTI_ENUM_VALUE(G16)
// RTTI_ENUM_VALUE(G8)
// RTTI_ENUM_VALUE(R16f)
// RTTI_ENUM_VALUE(RG16)
// RTTI_ENUM_VALUE(RG8)
// RTTI_ENUM_VALUE(RA16)
// RTTI_ENUM_VALUE(RA8)
// RTTI_ENUM_VALUE(RGBA16)
// RTTI_ENUM_VALUE(RGBA16f)
// RTTI_ENUM_VALUE(RGBA32f)
// RTTI_ENUM_VALUE(RGBA8)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
FExtname EImageFormat_Extname(EImageFormat value) NOEXCEPT {
    switch (value) {
    case EImageFormat::PNG: return FFS::Png();
    case EImageFormat::BMP: return FFS::Bmp();
    case EImageFormat::TGA: return FFS::Tga();
    case EImageFormat::JPG: return FFS::Jpg();
    case EImageFormat::HDR: return FFS::Hdr();

    case EImageFormat::Unknown:
        AssertNotImplemented();
    }

    return Default;
}
//----------------------------------------------------------------------------
EImageFormat EImageFormat_FromExtname(const FExtname& value) NOEXCEPT {
    if (value == FFS::Png()) return EImageFormat::PNG;
    if (value == FFS::Bmp()) return EImageFormat::BMP;
    if (value == FFS::Tga()) return EImageFormat::TGA;
    if (value == FFS::Jpg()) return EImageFormat::JPG;
    if (value == FFS::Hdr()) return EImageFormat::HDR;

    return EImageFormat::Unknown;
}
//----------------------------------------------------------------------------
ETextureColorMask ETextureSourceFormat_ColorMask(ETextureSourceFormat fmt) NOEXCEPT {
    Assert(fmt < ETextureSourceFormat::_Last);
    switch (fmt) {
    case ETextureSourceFormat::G8:      return ETextureColorMask::G;
    case ETextureSourceFormat::G16:     return ETextureColorMask::G;
    case ETextureSourceFormat::R16f:    return ETextureColorMask::R;
    case ETextureSourceFormat::RA8:     return ETextureColorMask::RA;
    case ETextureSourceFormat::RG8:     return ETextureColorMask::R|ETextureColorMask::G;
    case ETextureSourceFormat::RA16:    return ETextureColorMask::RA;
    case ETextureSourceFormat::RG16:    return ETextureColorMask::R|ETextureColorMask::G;
    case ETextureSourceFormat::RGBA8:   return ETextureColorMask::RGB;
    case ETextureSourceFormat::BGRA8:   return ETextureColorMask::RGBA;
    case ETextureSourceFormat::BGRE8:   return ETextureColorMask::RGBA;
    case ETextureSourceFormat::RGBA16:  return ETextureColorMask::RGBA;
    case ETextureSourceFormat::RGBA16f: return ETextureColorMask::RGBA;
    case ETextureSourceFormat::RGBA32f: return ETextureColorMask::RGBA;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }

    return ETextureColorMask::Unknown;
}
//----------------------------------------------------------------------------
FBytesRange ETextureSourceFormat_MipRange(
    ETextureSourceFormat fmt,
    const uint3& dimensions,
    u32 mipBias, u32 numMips) NOEXCEPT {
    Assert(numMips > 0);
    FBytesRange mipRange = FBytesRange::Zero();

    uint3 mipDimensions{ dimensions };
    forrange(mipLevel, 0, mipBias + numMips) {
        const size_t mipSizeInBytes = ETextureSourceFormat_SizeInBytes(fmt, mipDimensions);

        if (mipLevel < mipBias)
            mipRange.First += mipSizeInBytes;
        else
            mipRange.Last += mipSizeInBytes;

        mipDimensions = FTextureSourceProperties::NextMipDimensions(mipDimensions);
    }

    mipRange.Last += mipRange.First;
    return mipRange;
}
//----------------------------------------------------------------------------
size_t ETextureSourceFormat_SizeInBytes(ETextureSourceFormat fmt, const uint3& dimensions, u32 numMips, u32 numSlices) NOEXCEPT {
    Assert(numMips > 0);
    Assert(numSlices > 0);
    size_t totalSize = 0;

    for (uint3 mipDim{ dimensions }; numMips > 0; --numMips) {
        totalSize += ETextureSourceFormat_SizeInBytes(fmt, mipDim);
        mipDim = FTextureSourceProperties::NextMipDimensions(mipDim);
    }

    return (totalSize * numSlices);
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RHI enums binding for RTTI (#TODO: move to another module?)
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureGammaSpace)
// RTTI_ENUM_VALUE(Linear)
// RTTI_ENUM_VALUE(Pow22)
// RTTI_ENUM_VALUE(sRGB)
// RTTI_ENUM_VALUE(ACES)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureAddressMode)
// RTTI_ENUM_VALUE(Repeat)
// RTTI_ENUM_VALUE(MirrorRepeat)
// RTTI_ENUM_VALUE(ClampToEdge)
// RTTI_ENUM_VALUE(ClampToBorder)
// RTTI_ENUM_VALUE(MirrorClampToEdge)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_FLAGS_BEGIN(Texture, ETextureColorMask)
// RTTI_ENUM_VALUE(R)
// RTTI_ENUM_VALUE(G)
// RTTI_ENUM_VALUE(B)
// RTTI_ENUM_VALUE(A)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureImageView)
// RTTI_ENUM_VALUE(_1D)
// RTTI_ENUM_VALUE(_2D)
// RTTI_ENUM_VALUE(_3D)
// RTTI_ENUM_VALUE(_1DArray)
// RTTI_ENUM_VALUE(_2DArray)
// RTTI_ENUM_VALUE(_Cube)
// RTTI_ENUM_VALUE(_CubeArray)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureMipmapFilter)
// RTTI_ENUM_VALUE(Nearest)
// RTTI_ENUM_VALUE(Linear)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETextureSampleFilter)
// RTTI_ENUM_VALUE(Nearest)
// RTTI_ENUM_VALUE(Linear)
// RTTI_ENUM_VALUE(Cubic)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
// RTTI_ENUM_BEGIN(Texture, ETexturePixelFormat)
// RTTI_ENUM_VALUE(RGBA16_SNorm)
// RTTI_ENUM_VALUE(RGBA8_SNorm)
// RTTI_ENUM_VALUE(RGB16_SNorm)
// RTTI_ENUM_VALUE(RGB8_SNorm)
// RTTI_ENUM_VALUE(RG16_SNorm)
// RTTI_ENUM_VALUE(RG8_SNorm)
// RTTI_ENUM_VALUE(R16_SNorm)
// RTTI_ENUM_VALUE(R8_SNorm)
// RTTI_ENUM_VALUE(RGBA16_UNorm)
// RTTI_ENUM_VALUE(RGBA8_UNorm)
// RTTI_ENUM_VALUE(RGB16_UNorm)
// RTTI_ENUM_VALUE(RGB8_UNorm)
// RTTI_ENUM_VALUE(RG16_UNorm)
// RTTI_ENUM_VALUE(RG8_UNorm)
// RTTI_ENUM_VALUE(R16_UNorm)
// RTTI_ENUM_VALUE(R8_UNorm)
// RTTI_ENUM_VALUE(RGB10_A2_UNorm)
// RTTI_ENUM_VALUE(RGBA4_UNorm)
// RTTI_ENUM_VALUE(RGB5_A1_UNorm)
// RTTI_ENUM_VALUE(RGB_5_6_5_UNorm)
// RTTI_ENUM_VALUE(BGR8_UNorm)
// RTTI_ENUM_VALUE(BGRA8_UNorm)
// RTTI_ENUM_VALUE(sRGB8)
// RTTI_ENUM_VALUE(sRGB8_A8)
// RTTI_ENUM_VALUE(sBGR8)
// RTTI_ENUM_VALUE(sBGR8_A8)
// RTTI_ENUM_VALUE(R8i)
// RTTI_ENUM_VALUE(RG8i)
// RTTI_ENUM_VALUE(RGB8i)
// RTTI_ENUM_VALUE(RGBA8i)
// RTTI_ENUM_VALUE(R16i)
// RTTI_ENUM_VALUE(RG16i)
// RTTI_ENUM_VALUE(RGB16i)
// RTTI_ENUM_VALUE(RGBA16i)
// RTTI_ENUM_VALUE(R32i)
// RTTI_ENUM_VALUE(RG32i)
// RTTI_ENUM_VALUE(RGB32i)
// RTTI_ENUM_VALUE(RGBA32i)
// RTTI_ENUM_VALUE(R8u)
// RTTI_ENUM_VALUE(RG8u)
// RTTI_ENUM_VALUE(RGB8u)
// RTTI_ENUM_VALUE(RGBA8u)
// RTTI_ENUM_VALUE(R16u)
// RTTI_ENUM_VALUE(RG16u)
// RTTI_ENUM_VALUE(RGB16u)
// RTTI_ENUM_VALUE(RGBA16u)
// RTTI_ENUM_VALUE(R32u)
// RTTI_ENUM_VALUE(RG32u)
// RTTI_ENUM_VALUE(RGB32u)
// RTTI_ENUM_VALUE(RGBA32u)
// RTTI_ENUM_VALUE(RGB10_A2u)
// RTTI_ENUM_VALUE(R16f)
// RTTI_ENUM_VALUE(RG16f)
// RTTI_ENUM_VALUE(RGB16f)
// RTTI_ENUM_VALUE(RGBA16f)
// RTTI_ENUM_VALUE(R32f)
// RTTI_ENUM_VALUE(RG32f)
// RTTI_ENUM_VALUE(RGB32f)
// RTTI_ENUM_VALUE(RGBA32f)
// RTTI_ENUM_VALUE(RGB_11_11_10f)
// RTTI_ENUM_VALUE(Depth16)
// RTTI_ENUM_VALUE(Depth24)
// RTTI_ENUM_VALUE(Depth32f)
// RTTI_ENUM_VALUE(Depth16_Stencil8)
// RTTI_ENUM_VALUE(Depth24_Stencil8)
// RTTI_ENUM_VALUE(Depth32F_Stencil8)
// RTTI_ENUM_VALUE(BC1_RGB8_UNorm)
// RTTI_ENUM_VALUE(BC1_sRGB8)
// RTTI_ENUM_VALUE(BC1_RGB8_A1_UNorm)
// RTTI_ENUM_VALUE(BC1_sRGB8_A1)
// RTTI_ENUM_VALUE(BC2_RGBA8_UNorm)
// RTTI_ENUM_VALUE(BC2_sRGB8_A8)
// RTTI_ENUM_VALUE(BC3_RGBA8_UNorm)
// RTTI_ENUM_VALUE(BC3_sRGB8)
// RTTI_ENUM_VALUE(BC4_R8_SNorm)
// RTTI_ENUM_VALUE(BC4_R8_UNorm)
// RTTI_ENUM_VALUE(BC5_RG8_SNorm)
// RTTI_ENUM_VALUE(BC5_RG8_UNorm)
// RTTI_ENUM_VALUE(BC7_RGBA8_UNorm)
// RTTI_ENUM_VALUE(BC7_sRGB8_A8)
// RTTI_ENUM_VALUE(BC6H_RGB16F)
// RTTI_ENUM_VALUE(BC6H_RGB16UF)
// RTTI_ENUM_VALUE(ETC2_RGB8_UNorm)
// RTTI_ENUM_VALUE(ECT2_sRGB8)
// RTTI_ENUM_VALUE(ETC2_RGB8_A1_UNorm)
// RTTI_ENUM_VALUE(ETC2_sRGB8_A1)
// RTTI_ENUM_VALUE(ETC2_RGBA8_UNorm)
// RTTI_ENUM_VALUE(ETC2_sRGB8_A8)
// RTTI_ENUM_VALUE(EAC_R11_SNorm)
// RTTI_ENUM_VALUE(EAC_R11_UNorm)
// RTTI_ENUM_VALUE(EAC_RG11_SNorm)
// RTTI_ENUM_VALUE(EAC_RG11_UNorm)
// RTTI_ENUM_VALUE(ASTC_RGBA_4x4)
// RTTI_ENUM_VALUE(ASTC_RGBA_5x4)
// RTTI_ENUM_VALUE(ASTC_RGBA_5x5)
// RTTI_ENUM_VALUE(ASTC_RGBA_6x5)
// RTTI_ENUM_VALUE(ASTC_RGBA_6x6)
// RTTI_ENUM_VALUE(ASTC_RGBA_8x5)
// RTTI_ENUM_VALUE(ASTC_RGBA_8x6)
// RTTI_ENUM_VALUE(ASTC_RGBA_8x8)
// RTTI_ENUM_VALUE(ASTC_RGBA_10x5)
// RTTI_ENUM_VALUE(ASTC_RGBA_10x6)
// RTTI_ENUM_VALUE(ASTC_RGBA_10x8)
// RTTI_ENUM_VALUE(ASTC_RGBA_10x10)
// RTTI_ENUM_VALUE(ASTC_RGBA_12x10)
// RTTI_ENUM_VALUE(ASTC_RGBA_12x12)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_4x4)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_5x4)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_5x5)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_6x5)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_6x6)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_8x5)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_8x6)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_8x8)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_10x5)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_10x6)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_10x8)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_10x10)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_12x10)
// RTTI_ENUM_VALUE(ASTC_sRGB8_A8_12x12)
// RTTI_ENUM_VALUE(Unknown)
// RTTI_ENUM_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
