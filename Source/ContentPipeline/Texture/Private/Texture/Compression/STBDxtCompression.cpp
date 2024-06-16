// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/Compression/STBDxtCompression.h"

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

#include "Container/Vector.h"
#include "Container/SparseArray.h"
#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Thread/Task.h"

// will compile stb_dxt_write in this translation unit:
#include "stb_dxt-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static ETextureAddressMode STBDxtCompression_AddressMode(const FTextureSource& src) NOEXCEPT {
    if (src.IsTilable())
        return ETextureAddressMode::Repeat;
    else
        return ETextureAddressMode::Unknown;
}
//----------------------------------------------------------------------------
static int STBDxtCompression_QualityMode(const FTextureCompressionSettings& settings) NOEXCEPT {
    if (settings.Quality == ETextureCompressionQuality::High)
        return STB_DXT_HIGHQUAL;
    else
        return STB_DXT_NORMAL;
}
//----------------------------------------------------------------------------
static bool STBDxtCompression_SupportsImageView(RHI::EImageView view) NOEXCEPT {
    switch (view) {
    case RHI::EImageView::_1D:
        FALLTHROUGH();
    case RHI::EImageView::_1DArray:
        return false;

    case RHI::EImageView::_2D:
        FALLTHROUGH();
    case RHI::EImageView::_2DArray:
        FALLTHROUGH();
    case RHI::EImageView::_3D:
        FALLTHROUGH();
    case RHI::EImageView::_Cube:
        FALLTHROUGH();
    case RHI::EImageView::_CubeArray:
        return true;

    case RHI::EImageView::Unknown:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
static bool STBDxtCompression_SupportsSource(RHI::EPixelFormat dst, const FTextureSourceProperties& src, const FTextureCompressionSettings& settings) NOEXCEPT {
    Unused(settings);

    // check image dimensions (2D vs 3D vs Cube vs *Array)
    if (not STBDxtCompression_SupportsImageView(src.ImageView))
        return false;

    // check component type
    if (not ETextureSourceCompression_IsNorm8(src.Format))
        return false;

    // check number of channels
    const RHI::FPixelFormatInfo dstInfo = EPixelFormat_Infos(dst);
    const u32 srcChannels = Min(ETextureSourceFormat_Components(src.Format), EColorMask_NumChannels(src.ColorMask));
    if (dstInfo.Channels != srcChannels)
        return false;

    // check color space
    switch (src.Gamma) {
    /** No gamma correction is applied to this space, the incoming colors are assumed to already be in linear space. */
    case ETextureGammaSpace::Linear:
        if (dstInfo.ValueType.Flags ^ RHI::EPixelValueType::sRGB)
            return false;
        break;
    /** A simplified sRGB gamma correction is applied, pow(1/2.2). */
    case ETextureGammaSpace::Pow22:
        FALLTHROUGH(); // `david good enough`
    /** Use the standard sRGB conversion. */
    case ETextureGammaSpace::sRGB:
        if (not (dstInfo.ValueType.Flags ^ RHI::EPixelValueType::sRGB))
            return false;
        break;
    /** Use the new ACES standard for HDR values. */
    case ETextureGammaSpace::ACES:
        if (not ETextureSourceFormat_CanHoldHDR(src.Format))
            return false;
        if (not (dstInfo.ValueType.Types & RHI::EPixelValueType::AnyFloat))
            return false;
        return false; // not supported by STB (need BC6h)
    }

    return true;
}
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment specifier
template <RHI::EPixelFormat _PixelFormat>
static bool STBDxtCompression_CompressTextureData(
    FBulkData* outBulk,
    FTextureProperties* outProperties,
    const FTextureSource& src,
    const FTextureCompressionSettings& settings) {
    PPE_LOG_CHECK(Texture, STBDxtCompression_SupportsSource(_PixelFormat, src.Properties(), settings));

    const RHI::FPixelFormatInfo pixelInfo = RHI::EPixelFormat_Infos(_PixelFormat);
    Assert(pixelInfo.BlockDim == uint2{4,4});

    outProperties->Format = _PixelFormat;
    outProperties->Gamma = src.Gamma();
    outProperties->ImageView = src.ImageView();
    outProperties->NumMips = checked_cast<u8>(Min(src.NumMips(), pixelInfo.FullMipCount(src.Dimensions())));

    outBulk->AttachSourceFile(src.Data().SourceFile());
    outBulk->Resize_DiscardData(pixelInfo.SizeInBytes(
        RHI::EImageAspect::Color,
        src.Dimensions(),
        outProperties->NumMips,
        src.NumSlices()));

    FUniqueBuffer exclusiveDst = outBulk->LockWrite();
    DEFERRED{ outBulk->UnlockWrite(std::move(exclusiveDst)); };
    FRawMemory dstData = exclusiveDst.MakeView();
    const size_t dstSlicePitch = pixelInfo.SlicePitch(RHI::EImageAspect::Color, src.Dimensions(), outProperties->NumMips);

    const FTextureSource::FReaderScope sharedSrc{ src };
    FRawMemoryConst srcData = sharedSrc.MakeView();
    const size_t srcSlicePitch = ETextureSourceFormat_SizeInBytes(src.Format(), src.Dimensions(), src.NumMips());

    const struct FGlobalData_ {
        TPtrRef<const FTextureCompressionSettings> Settings;
        u32 DstBlockPitch;
        u32 SrcBlockPitch;
    }   global{
        .Settings = settings,
        .DstBlockPitch = (pixelInfo.BitsPerBlock(RHI::EImageAspect::Color) / 8),
        .SrcBlockPitch = checked_cast<u32>(ETextureSourceFormat_SizeInBytes(src.Format(), {pixelInfo.BlockDim.x, 1, 1})),
    };

    struct CACHELINE_ALIGNED FTaskArguments_ {
        FRawMemory DstRow;
        FRawMemoryConst SrcRow[4];
    };

    SPARSEARRAY(Texture, FTaskArguments_) args;
    VECTOR(Texture, FTaskFunc) tasks;

    forrange(slice, 0, src.NumSlices()) {
        FRawMemory sliceDst = dstData.Eat(dstSlicePitch);
        FRawMemoryConst sliceSrc = srcData.Eat(srcSlicePitch);

        uint3 mipDimensions = src.Dimensions();
        forrange(mip, 0, u32(outProperties->NumMips)) {
            const uint2 numBlocks = IntDivCeil(mipDimensions.xy, pixelInfo.BlockDim);
            Assert(AllGreater(numBlocks, uint2::Zero));

            FRawMemory mipDst = sliceDst.Eat(pixelInfo.SizeInBytes(RHI::EImageAspect::Color, mipDimensions));
            const size_t dstRowPitch = pixelInfo.RowPitch(RHI::EImageAspect::Color, mipDimensions);
            Assert_NoAssume(mipDst.SizeInBytes() == numBlocks.y * dstRowPitch);
            Assert_NoAssume(dstRowPitch == global.DstBlockPitch * numBlocks.x);

            FRawMemoryConst mipSrc = sliceSrc.Eat(ETextureSourceFormat_SizeInBytes(src.Format(), mipDimensions));
            const size_t srcRowPitch = ETextureSourceFormat_SizeInBytes(src.Format(), {mipDimensions.x, 1, 1});
            Assert_NoAssume(mipSrc.SizeInBytes() == numBlocks.y * srcRowPitch * pixelInfo.BlockDim.y);
            Assert_NoAssume(srcRowPitch == global.SrcBlockPitch * numBlocks.x);

            forrange(by, 0, numBlocks.y) {
                FTaskArguments_& job = args.Add();
                job.DstRow = mipDst.Eat(dstRowPitch);
                job.SrcRow[0] = mipSrc.Eat(srcRowPitch);
                job.SrcRow[1] = mipSrc.Eat(srcRowPitch);
                job.SrcRow[2] = mipSrc.Eat(srcRowPitch);
                job.SrcRow[3] = mipSrc.Eat(srcRowPitch);

                tasks.emplace_back([&job, &global, numBlocksX{numBlocks.x}](ITaskContext&) {
                    forrange(bx, 0, numBlocksX) {
                            TSTBDxtCompression<_PixelFormat>::CompressDxtBlock(
                            job.DstRow.Eat(global.DstBlockPitch),
                            {
                                job.SrcRow[0].Eat(global.SrcBlockPitch),
                                job.SrcRow[1].Eat(global.SrcBlockPitch),
                                job.SrcRow[2].Eat(global.SrcBlockPitch),
                                job.SrcRow[3].Eat(global.SrcBlockPitch),
                            },
                            global.Settings);
                    }

                    Assert_NoAssume(job.DstRow.empty());
                    Assert_NoAssume(job.SrcRow[0].empty());
                    Assert_NoAssume(job.SrcRow[1].empty());
                    Assert_NoAssume(job.SrcRow[2].empty());
                    Assert_NoAssume(job.SrcRow[3].empty());
                });
            }

            Assert_NoAssume(mipDst.empty());
            Assert_NoAssume(mipSrc.empty());
            mipDimensions = RHI::FPixelFormatInfo::NextMipDimensions(mipDimensions);
        }

        Assert_NoAssume(sliceDst.empty());
    }

    Assert_NoAssume(dstData.empty());
    Assert_NoAssume(srcData.empty());

    FGlobalThreadPool::Get().RunAndWaitFor(tasks);
    return true;
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::SupportsTextureSource(const FTextureSourceProperties& src, const FTextureCompressionSettings& settings) const NOEXCEPT {
    if (STBDxtCompression_SupportsSource(_PixelFormat, src, settings)) {
        switch (src.ImageView) {
        case RHI::EImageView::_1D:
            FALLTHROUGH();
        case RHI::EImageView::_1DArray:
            return false;

        case RHI::EImageView::_2D:
            FALLTHROUGH();
        case RHI::EImageView::_2DArray:
            FALLTHROUGH();
        case RHI::EImageView::_3D:
            FALLTHROUGH();
        case RHI::EImageView::_Cube:
            FALLTHROUGH();
        case RHI::EImageView::_CubeArray:
            break;

        case RHI::EImageView::Unknown:
            AssertNotImplemented();
        }

        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::CompressTexture(FTexture2D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_2D);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (STBDxtCompression_CompressTextureData<_PixelFormat>(&textureData, &textureProperties, src, settings)) {
        const ETextureAddressMode addressMode = STBDxtCompression_AddressMode(src);
        dst = FTexture2D(src.Dimensions().xy, textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::CompressTexture(FTexture2DArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_2DArray);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (STBDxtCompression_CompressTextureData<_PixelFormat>(&textureData, &textureProperties, src, settings)) {
        const ETextureAddressMode addressMode = STBDxtCompression_AddressMode(src);
        dst = FTexture2DArray(src.Dimensions().xy, src.NumSlices(), textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::CompressTexture(FTexture3D& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_3D);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (STBDxtCompression_CompressTextureData<_PixelFormat>(&textureData, &textureProperties, src, settings)) {
        const ETextureAddressMode addressMode = STBDxtCompression_AddressMode(src);
        dst = FTexture3D(src.Dimensions(), textureProperties, std::move(textureData), addressMode, addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::CompressTexture(FTextureCube& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_Cube);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (STBDxtCompression_CompressTextureData<_PixelFormat>(&textureData, &textureProperties, src, settings)) {
        const ETextureAddressMode addressMode = STBDxtCompression_AddressMode(src);
        dst = FTextureCube(src.Dimensions().xy, textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <RHI::EPixelFormat _PixelFormat>
bool TSTBDxtCompression<_PixelFormat>::CompressTexture(FTextureCubeArray& dst, const FTextureSource& src, const FTextureCompressionSettings& settings) const {
    PPE_LOG_CHECK(Texture, src.ImageView() == RHI::EImageView_CubeArray);

    FBulkData textureData;
    FTextureProperties textureProperties;
    if (STBDxtCompression_CompressTextureData<_PixelFormat>(&textureData, &textureProperties, src, settings)) {
        const ETextureAddressMode addressMode = STBDxtCompression_AddressMode(src);
        dst = FTextureCubeArray(src.Dimensions().xy, src.NumSlices(), textureProperties, std::move(textureData), addressMode, addressMode);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// BC1_RGB8_UNorm
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC1_RGB8_UNorm>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& settings) NOEXCEPT {
    u32 inputBlock[4][4] = {};
    const size_t bytesPerPixel = src[0].SizeInBytes() / 4;

    forrange(y, 0, 4)
    forrange(x, 0, 4) {
        FPlatformMemory::Memcpy(&inputBlock[y][x], src[y].data() + bytesPerPixel * x, bytesPerPixel);
    }

    constexpr int no_alpha = 0;
    ::stb_compress_dxt_block(dst.data(), MakeRawView(inputBlock).data(), no_alpha,
        STBDxtCompression_QualityMode(settings) );
}
//----------------------------------------------------------------------------
// BC1_sRGB8
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC1_sRGB8>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& settings) NOEXCEPT {
    u32 inputBlock[4][4] = {};
    const size_t bytesPerPixel = src[0].SizeInBytes() / 4;

    forrange(y, 0, 4)
    forrange(x, 0, 4) {
        FPlatformMemory::Memcpy(&inputBlock[y][x], src[y].data() + bytesPerPixel * x, bytesPerPixel);
    }

    constexpr int no_alpha = 0;
    ::stb_compress_dxt_block(dst.data(), MakeRawView(inputBlock).data(), no_alpha,
        STBDxtCompression_QualityMode(settings) );
}
//----------------------------------------------------------------------------
// BC3_RGBA8_UNorm
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC3_RGBA8_UNorm>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& settings) NOEXCEPT {
    u32 inputBlock[4][4];
    forrange(y, 0, 4)
        src[y].CopyTo(MakeRawView(inputBlock[y]));

    constexpr int with_alpha = 1;
    ::stb_compress_dxt_block(dst.data(), MakeRawView(inputBlock).data(), with_alpha,
        STBDxtCompression_QualityMode(settings) );
}
//----------------------------------------------------------------------------
// BC3_sRGB8_A8
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC3_sRGB8_A8>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& settings) NOEXCEPT {
    u32 inputBlock[4][4];
    forrange(y, 0, 4)
        src[y].CopyTo(MakeRawView(inputBlock[y]));

    constexpr int with_alpha = 1;
    ::stb_compress_dxt_block(dst.data(), MakeRawView(inputBlock).data(), with_alpha,
        STBDxtCompression_QualityMode(settings) );
}
//----------------------------------------------------------------------------
// BC4_R8_UNorm
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC4_R8_UNorm>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& ) NOEXCEPT {
    u8 inputBlock[4][4];
    forrange(y, 0, 4)
        src[y].CopyTo(MakeRawView(inputBlock[y]));

    ::stb_compress_bc4_block(dst.data(), MakeRawView(inputBlock).data());
}
//----------------------------------------------------------------------------
// BC5_RG8_SNorm
//----------------------------------------------------------------------------
template <>
void TSTBDxtCompression<RHI::EPixelFormat::BC5_RG8_SNorm>::CompressDxtBlock(
    const FRawMemory& dst, const FRawMemoryConst (&src)[4],
    const FTextureCompressionSettings& ) NOEXCEPT {
    u16 inputBlock[4][4];
    forrange(y, 0, 4)
        src[y].CopyTo(MakeRawView(inputBlock[y]));

    ::stb_compress_bc5_block(dst.data(), MakeRawView(inputBlock).data());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_STB_DXT_COMPRESSION_DEF(_PIXEL_FORMAT) \
    template class TSTBDxtCompression<RHI::EPixelFormat::_PIXEL_FORMAT>;
//----------------------------------------------------------------------------
PPE_STB_DXT_COMPRESSION_DEF(BC1_RGB8_UNorm     )
PPE_STB_DXT_COMPRESSION_DEF(BC1_sRGB8          )
PPE_STB_DXT_COMPRESSION_DEF(BC3_RGBA8_UNorm    )
PPE_STB_DXT_COMPRESSION_DEF(BC3_sRGB8_A8       )
PPE_STB_DXT_COMPRESSION_DEF(BC4_R8_UNorm       )
PPE_STB_DXT_COMPRESSION_DEF(BC5_RG8_SNorm      )
//----------------------------------------------------------------------------
#undef PPE_STB_DXT_COMPRESSION_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
