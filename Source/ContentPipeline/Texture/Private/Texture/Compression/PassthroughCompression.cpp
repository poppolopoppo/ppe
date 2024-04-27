// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/Compression/PassthroughCompression.h"

#include "Texture/TextureEnums.h"
#include "Texture/Texture.h"
#include "Texture/Texture2D.h"
#include "Texture/Texture2DArray.h"
#include "Texture/Texture3D.h"
#include "Texture/TextureCube.h"
#include "Texture/TextureCubeArray.h"
#include "Texture/TextureSource.h"

#include "RHI/PixelFormatHelpers.h"
#include "RHI/ResourceEnums.h"
#include "RHI/RenderStateEnums.h"

#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Thread/Task.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static ETextureAddressMode PassthroughCompression_AddressMode_(const FTextureSource& src) NOEXCEPT {
    if (src.IsTilable())
        return ETextureAddressMode::Repeat;
    else
        return ETextureAddressMode::Unknown;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
static bool PassthroughCompression_CreateTextureData_(
    FBulkData* outBulk,
    FTextureProperties* outProperties,
    const TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>& compression,
    const FTextureSource& src,
    const FTextureCompressionSettings& settings ) {
    PPE_LOG_CHECK(Texture, compression.SupportsTextureSource(src.Properties(), settings));

    const RHI::FPixelFormatInfo pixelInfo = RHI::EPixelFormat_Infos(_PixelFormat);
    Assert(pixelInfo.BlockDim == uint2{4,4});

    outProperties->Format = _PixelFormat;
    outProperties->Gamma = src.Gamma();
    outProperties->ImageView = src.ImageView();
    outProperties->NumMips = checked_cast<u8>(src.NumMips());

    outBulk->AttachSourceFile(src.Data().SourceFile());
    outBulk->Resize_DiscardData(pixelInfo.SizeInBytes(
        RHI::EImageAspect::Color,
        src.Dimensions(),
        src.NumMips(),
        src.NumSlices()));

    FUniqueBuffer exclusiveDst = outBulk->LockWrite();
    DEFERRED{ outBulk->UnlockWrite(std::move(exclusiveDst)); };
    const FRawMemory dstView = exclusiveDst.MakeView();

    const FTextureSource::FReaderScope sharedSrc{ src };
    const FRawMemoryConst srcView = sharedSrc.MakeView();

    PPE_LOG_CHECK(Texture, srcView.SizeInBytes() == dstView.SizeInBytes());
    FPlatformMemory::MemcpyLarge(dstView.data(), srcView.data(), srcView.SizeInBytes());

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::SupportsTextureSource(const FTextureSourceProperties& src, const FTextureCompressionSettings& ) const NOEXCEPT {
    if (src.Format == _SourceFormat && src.Gamma == _GammaSpace) {
        return (RHI::EColorMask_NumChannels(src.ColorMask) >= ETextureSourceFormat_Components(src.Format)); // #TODO: support color mask?
    }
    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::CompressTexture(FTexture2D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_2D);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (PassthroughCompression_CreateTextureData_(&textureData, &textureProperties, *this, src, settings)) {
        const ETextureAddressMode addressMode = PassthroughCompression_AddressMode_(src);
        dst = FTexture2D(src.Dimensions().xy, textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::CompressTexture(FTexture2DArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_2DArray);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (PassthroughCompression_CreateTextureData_(&textureData, &textureProperties, *this, src, settings)) {
        const ETextureAddressMode addressMode = PassthroughCompression_AddressMode_(src);
        dst = FTexture2DArray(src.Dimensions().xy, src.NumSlices(), textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::CompressTexture(FTexture3D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_3D);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (PassthroughCompression_CreateTextureData_(&textureData, &textureProperties, *this, src, settings)) {
        const ETextureAddressMode addressMode = PassthroughCompression_AddressMode_(src);
        dst = FTexture3D(src.Dimensions(), textureProperties, std::move(textureData), addressMode, addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::CompressTexture(FTextureCube& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_Cube);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (PassthroughCompression_CreateTextureData_(&textureData, &textureProperties, *this, src, settings)) {
        const ETextureAddressMode addressMode = PassthroughCompression_AddressMode_(src);
        dst = FTextureCube(src.Dimensions().xy, textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat, ETextureSourceFormat _SourceFormat, ETextureGammaSpace _GammaSpace>
bool TPassthroughCompression<_PixelFormat, _SourceFormat, _GammaSpace>::CompressTexture(FTextureCubeArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_CubeArray);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (PassthroughCompression_CreateTextureData_(&textureData, &textureProperties, *this, src, settings)) {
        const ETextureAddressMode addressMode = PassthroughCompression_AddressMode_(src);
        dst = FTextureCubeArray(src.Dimensions().xy, src.NumSlices(), textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
// Template instantiations
//----------------------------------------------------------------------------
#define PPE_PASSTHROUGH_COMPRESSION_DEF(_PIXEL_FORMAT, _SOURCE_FORMAT, _GAMMA_SPACE) \
    template class TPassthroughCompression<RHI::EPixelFormat::_PIXEL_FORMAT, ETextureSourceFormat::_SOURCE_FORMAT, ETextureGammaSpace::_GAMMA_SPACE>;
//----------------------------------------------------------------------------
PPE_PASSTHROUGH_COMPRESSION_DEF(BGRA8_UNorm,    BGRA8,      Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(sBGR8_A8,       BGRA8,      sRGB    )
PPE_PASSTHROUGH_COMPRESSION_DEF(R16_UNorm,      G16,        Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(R8_UNorm,       G8,         Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(R16f,           R16f,       Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RG16_UNorm,     RG16,       Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RG8_UNorm,      RG8,        Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RGBA16_UNorm,   RGBA16,     Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RGBA16f,        RGBA16f,    Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RGBA32f,        RGBA32f,    Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(RGBA8_UNorm,    RGBA8,      Linear  )
PPE_PASSTHROUGH_COMPRESSION_DEF(sRGB8_A8,       RGBA8,      sRGB    )
//----------------------------------------------------------------------------
#undef PPE_PASSTHROUGH_COMPRESSION_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
