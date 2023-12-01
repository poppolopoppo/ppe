#pragma once

#include "Texture_fwd.h"

// #include "RTTI/Macros.h"

#include "Color/Color.h"
#include "IO/FileSystem_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Aliases from RHI for RTTI:
using ETextureGammaSpace = EGammaSpace;
using ETextureAddressMode = RHI::EAddressMode;
using ETextureColorMask = RHI::EColorMask;
using ETextureImageView = RHI::EImageView;
using ETextureMipmapFilter = RHI::EMipmapFilter;
using ETexturePixelFormat = RHI::EPixelFormat;
using ETextureSampleFilter = RHI::ETextureFilter;

// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureGammaSpace);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureAddressMode);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureColorMask);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureImageView);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureMipmapFilter);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETexturePixelFormat);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureSampleFilter);
//----------------------------------------------------------------------------
enum class EImageFormat : u8 {
    Unknown = 0,

    PNG,
    BMP,
    TGA,
    JPG,
    HDR,
};
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, EImageFormat);
//----------------------------------------------------------------------------
NODISCARD PPE_TEXTURE_API FExtname EImageFormat_Extname(EImageFormat value) NOEXCEPT;
NODISCARD PPE_TEXTURE_API EImageFormat EImageFormat_FromExtname(const FExtname& extname) NOEXCEPT;
//----------------------------------------------------------------------------
enum class ETextureSourceCompression : u8 {
    None = 0,

    JPG,
    PNG,

    Unknown = None,
};
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureSourceCompression);
//----------------------------------------------------------------------------
enum class ETextureSourceFlags : u8 {
    HDR                 = 1 << 0,
    LongLatCubemap      = 1 << 1,
    PreMultipliedAlpha  = 1 << 2,
    SRGB                = 1 << 3,
    Tiling              = 1 << 4,

    Unknown = 0,
};
ENUM_FLAGS(ETextureSourceFlags);
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureSourceFlags);
//----------------------------------------------------------------------------
enum class ETextureSourceFormat : u8 {
    Unknown = 0,

    BGRA8,
    BGRE8,
    G16,
    G8,
    R16f,
    RG16,
    RG8,
    RA16,
    RA8,
    RGBA16,
    RGBA16f,
    RGBA32f,
    RGBA8,

    _Last,
};
// RTTI_ENUM_HEADER(PPE_TEXTURE_API, ETextureSourceFormat);
//----------------------------------------------------------------------------
CONSTEXPR u32 ETextureSourceFormat_BytesPerPixel(ETextureSourceFormat fmt) {
    Assert(fmt < ETextureSourceFormat::_Last);
    switch (fmt) {
    case ETextureSourceFormat::G8:      return 1;
    case ETextureSourceFormat::G16:     return 2;
    case ETextureSourceFormat::RA8:     return 2;
    case ETextureSourceFormat::RG8:     return 2;
    case ETextureSourceFormat::R16f:    return 2;
    case ETextureSourceFormat::RA16:    return 4;
    case ETextureSourceFormat::RG16:    return 4;
    case ETextureSourceFormat::RGBA8:   return 4;
    case ETextureSourceFormat::BGRA8:   return 4;
    case ETextureSourceFormat::BGRE8:   return 4;
    case ETextureSourceFormat::RGBA16:  return 8;
    case ETextureSourceFormat::RGBA16f: return 8;
    case ETextureSourceFormat::RGBA32f: return 16;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }

    return 0;
}
//----------------------------------------------------------------------------
CONSTEXPR size_t ETextureSourceFormat_SizeInBytes(ETextureSourceFormat fmt, const uint3& dimensions) {
    const u32 bytesPerPixel = ETextureSourceFormat_BytesPerPixel(fmt);
    return (bytesPerPixel * static_cast<size_t>(dimensions.x) * static_cast<size_t>(dimensions.y) * static_cast<size_t>(dimensions.z));
}
//----------------------------------------------------------------------------
PPE_TEXTURE_API size_t ETextureSourceFormat_SizeInBytes(
    ETextureSourceFormat fmt,
    const uint3& dimensions,
    u32 numMips,
    u32 numSlices = 1) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_TEXTURE_API FBytesRange ETextureSourceFormat_MipRange(
    ETextureSourceFormat fmt,
    const uint3& dimensions,
    u32 mipBias, u32 numMips = 1) NOEXCEPT;
//----------------------------------------------------------------------------
inline FBytesRange ETextureSourceFormat_SliceRange(
    ETextureSourceFormat fmt,
    const uint3& dimensions,
    u32 numMips,
    u32 sliceIndex ) NOEXCEPT {
    const size_t strideInBytes = ETextureSourceFormat_SizeInBytes(fmt, dimensions, numMips);
    return { strideInBytes * sliceIndex, strideInBytes * (sliceIndex + 1) };
}
//----------------------------------------------------------------------------
CONSTEXPR u32 ETextureSourceFormat_Components(ETextureSourceFormat fmt) {
    Assert(fmt < ETextureSourceFormat::_Last);
    switch (fmt) {
    case ETextureSourceFormat::G8:      return 1;
    case ETextureSourceFormat::G16:     return 1;
    case ETextureSourceFormat::R16f:    return 1;
    case ETextureSourceFormat::RA8:     return 2;
    case ETextureSourceFormat::RG8:     return 2;
    case ETextureSourceFormat::RA16:    return 2;
    case ETextureSourceFormat::RG16:    return 2;
    case ETextureSourceFormat::RGBA8:   return 4;
    case ETextureSourceFormat::BGRA8:   return 4;
    case ETextureSourceFormat::BGRE8:   return 4;
    case ETextureSourceFormat::RGBA16:  return 4;
    case ETextureSourceFormat::RGBA16f: return 4;
    case ETextureSourceFormat::RGBA32f: return 4;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }

    return 0;
}
//----------------------------------------------------------------------------
CONSTEXPR bool ETextureSourceCompression_IsFloat(ETextureSourceFormat fmt) {
    Assert(fmt < ETextureSourceFormat::_Last);
    switch (fmt) {
    case ETextureSourceFormat::G8:      return false;
    case ETextureSourceFormat::G16:     return false;
    case ETextureSourceFormat::RA8:     return false;
    case ETextureSourceFormat::RG8:     return false;
    case ETextureSourceFormat::R16f:    return true;
    case ETextureSourceFormat::RA16:    return false;
    case ETextureSourceFormat::RG16:    return false;
    case ETextureSourceFormat::RGBA8:   return false;
    case ETextureSourceFormat::BGRA8:   return false;
    case ETextureSourceFormat::BGRE8:   return false;
    case ETextureSourceFormat::RGBA16:  return false;
    case ETextureSourceFormat::RGBA16f: return false;
    case ETextureSourceFormat::RGBA32f: return true;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }

    return false;
}
//----------------------------------------------------------------------------
CONSTEXPR bool ETextureSourceFormat_CanCompressWithJPEG(ETextureSourceFormat fmt) {
    switch (fmt) {
    case ETextureSourceFormat::BGRA8:
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
CONSTEXPR bool ETextureSourceFormat_CanCompressWithPNG(ETextureSourceFormat fmt) {
    switch (fmt) {
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
    case ETextureSourceFormat::RGBA16:
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
CONSTEXPR bool ETextureSourceFormat_CanHoldHDR(ETextureSourceFormat fmt) {
    switch (fmt) {
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
