// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/TextureSource.h"

#include "Texture/TextureEnums.h"

#include "RHI/PixelFormatHelpers.h"
#include "RHI/ResourceEnums.h"
#include "RHI/RenderStateEnums.h"

#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Thread/ThreadPool.h"
#include "Thread/Task/TaskHelpers.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
u32 FTextureSourceProperties::NumComponents() const NOEXCEPT {
    return ETextureSourceFormat_Components(Format);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::HasAlpha() const NOEXCEPT {
    return (ColorMask & ETextureColorMask::A);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::HasMaskedAlpha() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::MaskedAlpha);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::IsHDR() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::HDR);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::IsLongLatCubemap() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::LongLatCubemap);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::HasPreMultipliedAlpha() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::PreMultipliedAlpha);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::IsTilable() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::Tilable);
}
//----------------------------------------------------------------------------
size_t FTextureSourceProperties::SizeInBytes() const NOEXCEPT {
    return ETextureSourceFormat_SizeInBytes(Format, Dimensions, NumMips, NumSlices);
}
//----------------------------------------------------------------------------
uint3 FTextureSourceProperties::MipDimensions(u32 mipBias) const NOEXCEPT {
    Assert_NoAssume(mipBias < NumMips);

    uint3 mipDimensions = Dimensions;
    forrange(mip, 0, mipBias)
        mipDimensions = NextMipDimensions(mipDimensions);
    return mipDimensions;
}
//----------------------------------------------------------------------------
FBytesRange FTextureSourceProperties::MipRange(u32 mipBias, u32 numMips, u32 sliceIndex) const NOEXCEPT {
    Assert(numMips > 0);
    Assert_NoAssume(mipBias + numMips <= NumMips);
    const FBytesRange sliceRange = ETextureSourceFormat_SliceRange(Format, Dimensions, NumMips, sliceIndex);
    const FBytesRange mipRange = ETextureSourceFormat_MipRange(Format, Dimensions, mipBias, numMips);
    Assert_NoAssume(mipRange.Extent() <= sliceRange.Extent());
    return { sliceRange.First + mipRange.First, sliceRange.First + mipRange.Last };
}
//----------------------------------------------------------------------------
FRawMemory FTextureSourceProperties::MipView(const FRawMemory& textureData, u32 mipBias, u32 numMips, u32 sliceIndex) const NOEXCEPT {
    Assert_NoAssume(textureData.SizeInBytes() == SizeInBytes());
    const FBytesRange range = MipRange(mipBias, numMips, sliceIndex);
    return textureData.SubRange(range.First, range.Extent());
}
//----------------------------------------------------------------------------
FRawMemoryConst FTextureSourceProperties::MipView(const FRawMemoryConst& textureData, u32 mipBias, u32 numMips, u32 sliceIndex) const NOEXCEPT {
    Assert_NoAssume(textureData.SizeInBytes() == SizeInBytes());
    const FBytesRange range = MipRange(mipBias, numMips, sliceIndex);
    return textureData.SubRange(range.First, range.Extent());
}
//----------------------------------------------------------------------------
FBytesRange FTextureSourceProperties::SliceRange(u32 sliceIndex) const NOEXCEPT {
    Assert(sliceIndex < Dimensions.z);
    return ETextureSourceFormat_SliceRange(Format, Dimensions, NumMips, sliceIndex);
}
//----------------------------------------------------------------------------
FRawMemory FTextureSourceProperties::SliceView(const FRawMemory& textureData, u32 sliceIndex) const NOEXCEPT {
    const FBytesRange range = SliceRange(sliceIndex);
    return textureData.SubRange(range.First, range.Extent());
}
//----------------------------------------------------------------------------
FRawMemoryConst FTextureSourceProperties::SliceView(const FRawMemoryConst& textureData, u32 sliceIndex) const NOEXCEPT {
    const FBytesRange range = SliceRange(sliceIndex);
    return textureData.SubRange(range.First, range.Extent());
}
//----------------------------------------------------------------------------
RHI::EPixelFormat FTextureSourceProperties::PixelFormat(const FTextureSourceProperties& properties) NOEXCEPT {
    using namespace RHI;
    switch (properties.Format) {
    case ETextureSourceFormat::RGBA8:
        return (properties.Gamma == EGammaSpace::sRGB
            ? EPixelFormat::sRGB8_A8
            : EPixelFormat::RGBA8_UNorm);
    case ETextureSourceFormat::BGRA8:
        return (properties.Gamma == EGammaSpace::sRGB
            ? EPixelFormat::sBGR8_A8
            : EPixelFormat::BGR8_UNorm);
    case ETextureSourceFormat::BGRE8:
        return EPixelFormat::BGR8_UNorm;
    case ETextureSourceFormat::G16:
        return EPixelFormat::R16_UNorm;
    case ETextureSourceFormat::G8:
        return EPixelFormat::R8_UNorm;
    case ETextureSourceFormat::R16f:
        return EPixelFormat::R16f;
    case ETextureSourceFormat::RG16:
        return EPixelFormat::RG16_UNorm;
    case ETextureSourceFormat::RG8:
        return EPixelFormat::RG8_UNorm;
    case ETextureSourceFormat::RA16:
        return EPixelFormat::RG16_UNorm;
    case ETextureSourceFormat::RA8:
        return EPixelFormat::RG8_UNorm;
    case ETextureSourceFormat::RGBA16:
        return EPixelFormat::RGBA16_UNorm;
    case ETextureSourceFormat::RGBA16f:
        return EPixelFormat::RGBA16f;
    case ETextureSourceFormat::RGBA32f:
        return EPixelFormat::RGBA32f;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::Texture2D(
    u32 width, u32 height,
    u32 numMips,
    ETextureColorMask colorMask,
    ETextureGammaSpace gammaSpace,
    ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, 1 },
        .NumMips = numMips,
        .NumSlices = 1,
        .Gamma = gammaSpace,
        .Flags = Default,
        .Format = format,
        .ColorMask = colorMask,
        .ImageView = ETextureImageView::_2D,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::Texture2DArray(
    u32 width, u32 height,
    u32 numMips,
    u32 numSlices,
    ETextureColorMask colorMask,
    ETextureGammaSpace gammaSpace,
    ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, 1 },
        .NumMips = numMips,
        .NumSlices = numSlices,
        .Gamma = gammaSpace,
        .Flags = Default,
        .Format = format,
        .ColorMask = colorMask,
        .ImageView = ETextureImageView::_2DArray,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::Texture2DWithMipChain(u32 width, u32 height, ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, 1 },
        .NumMips = FTextureSourceProperties::FullMipCount(uint3(width, height, 1)),
        .NumSlices = 1,
        .Gamma = ETextureGammaSpace::sRGB,
        .Flags = Default,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_2D,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::Texture2DArrayWithMipChain(u32 width, u32 height, u32 numSlices, ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, 1 },
        .NumMips = FTextureSourceProperties::FullMipCount(uint3(width, height, 1)),
        .NumSlices = numSlices,
        .Gamma = ETextureGammaSpace::sRGB,
        .Flags = Default,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_2DArray,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::TextureCubeWithMipChain(u32 width, u32 height, ETextureSourceFormat format, bool isLongLatCubemap) NOEXCEPT {
    ETextureSourceFlags flags = Default;
    if (isLongLatCubemap)
        flags += ETextureSourceFlags::LongLatCubemap;

    return {
        .Dimensions = { width, height, 1 },
        .NumMips = FTextureSourceProperties::FullMipCount(uint3(width, height, 1)),
        .NumSlices = 6,
        .Gamma = ETextureGammaSpace::sRGB,
        .Flags = flags,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_Cube,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::TextureCubeArrayWithMipChain(u32 width, u32 height, u32 numSlices, ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, 1 },
        .NumMips = FTextureSourceProperties::FullMipCount(uint3(width, height, 1)),
        .NumSlices = 6 * numSlices,
        .Gamma = ETextureGammaSpace::sRGB,
        .Flags = Default,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_CubeArray,
    };
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureSourceProperties::TextureVolumeWithMipChain(u32 width, u32 height, u32 depth, ETextureSourceFormat format) NOEXCEPT {
    return {
        .Dimensions = { width, height, depth },
        .NumMips = FTextureSourceProperties::FullMipCount(uint3(width, height, depth)),
        .NumSlices = 1,
        .Gamma = ETextureGammaSpace::sRGB,
        .Flags = Default,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_3D,
    };
}
//----------------------------------------------------------------------------
uint3 FTextureSourceProperties::NextMipDimensions(const uint3& dimensions) NOEXCEPT {
    return RHI::FPixelFormatInfo::NextMipDimensions(dimensions);
}
//----------------------------------------------------------------------------
u32 FTextureSourceProperties::FullMipCount(const uint3& dimensions) NOEXCEPT {
    return RHI::FPixelFormatInfo::FullMipCount(dimensions, uint2::One);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FTextureSource::Construct(
    const FTextureSourceProperties& properties,
    Meta::TOptional<FBulkData>&& optionalData) {
    Assert(AllGreater(properties.Dimensions, uint3::Zero));
    Assert(properties.NumMips > 0);
    Assert(properties.ImageView != Default);

    _properties = properties;
    _compression = Default;

    if (ETextureSourceFormat_CanHoldHDR(_properties.Format))
        _properties.Flags += ETextureSourceFlags::HDR;

#if 0 // #TODO: store image as PNG or JPEG instead of raw RGBA8 to save space
    if (ETextureSourceFormat_CanCompressWithJPEG(_properties.Format))
        _compression = ETextureSourceCompression::JPG;
    else if (ETextureSourceFormat_CanCompressWithPNG(_properties.Format))
        _compression = ETextureSourceCompression::PNG;
#endif

    const size_t decompressedSizeInBytes = _properties.SizeInBytes();
    if (_compression == ETextureSourceCompression::None) {
        if (optionalData.has_value())
            _bulkData = std::move(*optionalData);
        else
            _bulkData.Resize_DiscardData(decompressedSizeInBytes);
    }
    else {
        AssertReleaseMessage("can't suppy optional data to copy when compression is enabled", not optionalData.has_value());
    }
}
//----------------------------------------------------------------------------
void FTextureSource::TearDown() {
    _bulkData.Reset();

    _properties = Default;
    _compression = Default;
}
//----------------------------------------------------------------------------
bool FTextureSource::HasFullMipChain2D() const NOEXCEPT {
    const FReaderScope sharedTexture(*this);
    const u32 fullMipCount = FTextureSourceProperties::FullMipCount(sharedTexture.Source._properties.Dimensions);
    return (sharedTexture.Source._properties.NumMips == fullMipCount);
}
//----------------------------------------------------------------------------
FSharedBuffer FTextureSource::LockRead() const {
    return _bulkData.LockRead();
}
//----------------------------------------------------------------------------
void FTextureSource::UnlockRead(FSharedBuffer&& data) const {
    _bulkData.UnlockRead(std::move(data));
}
//----------------------------------------------------------------------------
FUniqueBuffer FTextureSource::LockWrite() {
    return _bulkData.LockWrite();
}
//----------------------------------------------------------------------------
void FTextureSource::UnlockWrite(FUniqueBuffer&& data) {
    _bulkData.UnlockWrite(std::move(data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
