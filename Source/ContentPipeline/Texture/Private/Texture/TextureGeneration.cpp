// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/TextureGeneration.h"

#include "Texture/TextureEnums.h"
#include "Texture/TextureSource.h"
#include "TextureService.h"

#include "Texture/Texture2D.h"
#include "Texture/Texture2DArray.h"
#include "Texture/Texture3D.h"
#include "Texture/TextureCube.h"
#include "Texture/TextureCubeArray.h"

#include "RHI/ImageView.h"
#include "RHI/PixelFormatHelpers.h"
#include "RHI/ResourceEnums.h"
#include "RHI/SamplerEnums.h"

#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Memory/UniqueView.h"
#include "Thread/ThreadPool.h"
#include "Thread/Task/TaskHelpers.h"
#include "Time/TimedScope.h"

#if USE_PPE_LOGGER
#   include "IO/FormatHelpers.h"
#   include "RHI/EnumToString.h"
#   include "Texture/EnumToString.h"
#endif

// will compile stb_image_resize2 in this translation unit:
#include "stb_image_resize2-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR ::stbir_datatype STBImageDataType_(ETextureSourceFormat fmt, ETextureGammaSpace gamma) {
    switch (fmt) {
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::BGRA8:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::RGBA8:
        return (gamma == ETextureGammaSpace::sRGB ? STBIR_TYPE_UINT8_SRGB : STBIR_TYPE_UINT8);

    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::RGBA16:
        return STBIR_TYPE_UINT16;

    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RGBA16f:
        return STBIR_TYPE_HALF_FLOAT;

    case ETextureSourceFormat::RGBA32f:
        return STBIR_TYPE_FLOAT;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return Default;
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR ::stbir_pixel_layout STBImagePixelLayout_(ETextureSourceFormat fmt, ETextureSourceFlags flags) {
    switch (fmt) {
    case ETextureSourceFormat::BGRA8:
        return (flags ^ ETextureSourceFlags::PreMultipliedAlpha ? STBIR_BGRA_PM : STBIR_BGRA);
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::RGBA16:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        return (flags ^ ETextureSourceFlags::PreMultipliedAlpha ? STBIR_RGBA_PM : STBIR_RGBA);
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
        return (flags ^ ETextureSourceFlags::PreMultipliedAlpha ? STBIR_RA_PM : STBIR_RA);

    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::R16f:
        return STBIR_1CHANNEL;
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
        return STBIR_2CHANNEL;
    case ETextureSourceFormat::BGRE8:
        return STBIR_4CHANNEL;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotReached();
    }

    return Default;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextureGeneration::FTextureGeneration(const FTextureSourceProperties& properties) NOEXCEPT {
    if (properties.NumMips != properties.FullMipCount(properties.Dimensions)) {
        bGenerateFullMipChain2D = true;
    }

    if (properties.HasAlpha()) {
        bFloodMipChainWithAlpha = true;

        if (properties.HasMaskedAlpha()) {
            AlphaCutoff = 0.5f;
            bPreserveAlphaTestCoverage2D = true;
        }
    }
}
//----------------------------------------------------------------------------
FTextureGeneration::FTextureGeneration(const ITextureService& service, const FTextureSourceProperties& properties) NOEXCEPT
:   FTextureGeneration(properties) {
    service.TextureCompression(
        MakeAppendable<UTextureCompression>([this](UTextureCompression&& newCompression) {
            if (not Compression || (
                // adopt new compression format if it's yielding to smaller images
                EPixelFormat_BitsPerPixel(newCompression->Format(), RHI::EImageAspect::Color) <
                EPixelFormat_BitsPerPixel(Compression->Format(), RHI::EImageAspect::Color) )) {
                Compression = std::move(newCompression);
            }
        }), properties, *this);
}
//----------------------------------------------------------------------------
FTextureSourceProperties FTextureGeneration::Prepare(const FTextureSourceProperties& source) const NOEXCEPT {
    FTextureSourceProperties newProperties{ source };
    newProperties.Dimensions = ResizeDimensions.value_or(source.Dimensions);
    newProperties.Format = ResizeFormat.value_or(source.Format);
    newProperties.Flags = ResizeFlags.value_or(source.Flags);

    const RHI::FPixelFormatInfo pixelInfo = EPixelFormat_Infos(Compression
        ? Compression->Format()
        : FTextureSourceProperties::PixelFormat(newProperties) );

    if (bGenerateFullMipChain2D)
        newProperties.NumMips = pixelInfo.FullMipCount(newProperties.Dimensions);
    else
        newProperties.NumMips = Min(newProperties.NumMips, pixelInfo.FullMipCount(newProperties.Dimensions));

    return newProperties;
}
//----------------------------------------------------------------------------
PTexture FTextureGeneration::Generate(const FTextureSource& source) const {
    PPE_LOG_CHECK(Texture, Compression.Valid());

    FTextureSource updatedSource;
    TPtrRef actualInput{ source };
    if (ResizeDimensions.has_value() ||
        ResizeFlags.has_value() ||
        ResizeFormat.has_value() ||
        bFloodMipChainWithAlpha ||
        bGenerateAlphaDistanceField2D ||
        bGenerateFullMipChain2D ||
        bPreserveAlphaTestCoverage2D ) {
        PPE_LOG_CHECK(Texture, source.ImageView() == RHI::EImageView_2D);

        const FTextureSource::FReaderScope oldTexture(source);

        // creates a new texture
        {
            FTextureSourceProperties newProperties = Prepare(source._properties);

            PPE_LOG_CHECK(Texture, Compression->SupportsTextureSource(newProperties, *this));

            FBulkData newData;
            newData.AttachSourceFile(source._bulkData.SourceFile());
            newData.Resize_DiscardData(newProperties.SizeInBytes());

            updatedSource.Construct(newProperties, std::move(newData));
        }
        Assert_NoAssume(updatedSource._properties.NumSlices == source._properties.NumSlices);

        FTextureSource::FWriterScope newTexture{ updatedSource };

        if (updatedSource._properties.Dimensions != source._properties.Dimensions ||
            updatedSource._properties.Format != source._properties.Format ||
            updatedSource._properties.Flags != source._properties.Flags ) {
            // resize the top mip while applying format/flags conversion if needed

            std::atomic_int failedSlices{ 0 };
            ParallelFor(0, updatedSource._properties.NumSlices, [&](size_t slice) {
                if (not ResizeMip2D(updatedSource._properties,
                    updatedSource._properties.Dimensions.xy,
                    newTexture.MipData(0, 1, static_cast<u32>(slice)),
                    source._properties.Dimensions.xy,
                    source._properties.Format,
                    source._properties.Flags,
                    source._properties.Gamma,
                    oldTexture.MipData(0, 1, static_cast<u32>(slice)) )) {
                    ++failedSlices;
                }
            });
            PPE_LOG_CHECK(Texture, failedSlices == 0);

        } else {
            // simply copy the top mip

            ParallelFor(0, updatedSource._properties.NumSlices, [&](size_t slice) {
                oldTexture.MipData(0, 1, static_cast<u32>(slice)).CopyTo(
                    newTexture.MipData(0, 1, static_cast<u32>(slice)) );
            });
        }

        if (bGenerateAlphaDistanceField2D) {
            ParallelFor(0, updatedSource._properties.NumSlices, [&](size_t slice) {
                GenerateAlphaDistanceField2D(
                    updatedSource._properties,
                    updatedSource._properties.Dimensions.xy,
                    newTexture.MipData(0, 1, static_cast<u32>(slice)),
                    AlphaSpreadRatio );
            });
        }

        {
            std::atomic_int failedSlices{ 0 };
            ParallelFor(0, updatedSource._properties.NumSlices, [&](size_t slice) {
                if (not GenerateSliceMipChain2D(updatedSource._properties, newTexture.SliceData(static_cast<u32>(slice))) )
                    ++failedSlices;
            });
            PPE_LOG_CHECK(Texture, failedSlices == 0);
        }

        if (bFloodMipChainWithAlpha) {
            PPE_LOG_CHECK(Texture, FloodMipChainWithAlpha(updatedSource._properties, newTexture.MakeView()));
        }

        actualInput = &updatedSource;
    }

    PTexture result;
    BENCHMARK_SCOPE_ARGS(Texture, "CompressTexture",
        {"ImageView", Opaq::Format(actualInput->ImageView())},
        {"Dimensions", Opaq::array_init{actualInput->Dimensions().x, actualInput->Dimensions().y, actualInput->Dimensions().z}},
        {"NumMips", actualInput->NumMips()},
        {"SourceFile", Opaq::Format(actualInput->Data().SourceFile().value_or(FFilename{}))},
        {"Dst", Opaq::object_init{
            {"Format", Opaq::Format(Compression->Format())},
            {"Size", [&result](FTextWriter& oss) {
                oss << Fmt::FSizeInBytes(result ? result->Data().SizeInBytes() : 0);
            }},
        }},
        {"Src", Opaq::object_init{
            {"Format", Opaq::Format(actualInput->Format())},
            {"Flags", Opaq::Format(actualInput->Flags())},
            {"Gamma", Opaq::Format(actualInput->Gamma())},
            {"ColorMask", Opaq::Format(actualInput->ColorMask())},
            {"Size", Opaq::Format(Fmt::FSizeInBytes{ actualInput->Data().SizeInBytes() })}
        }});

    switch (actualInput->ImageView()) {
    case RHI::EImageView::_1D:
        FALLTHROUGH();
    case RHI::EImageView::_1DArray:
        break;

    case RHI::EImageView::_2D:
    {
        FTexture2D dst;
        if (Compression->CompressTexture(dst, *actualInput, *this))
            result = NEW_REF(Texture, FTexture2D, std::move(dst));
    }
        break;
    case RHI::EImageView::_2DArray:
    {
        FTexture2DArray dst;
        if (Compression->CompressTexture(dst, *actualInput, *this))
            result = NEW_REF(Texture, FTexture2DArray, std::move(dst));
    }
        break;
    case RHI::EImageView::_3D:
    {
        FTexture3D dst;
        if (Compression->CompressTexture(dst, *actualInput, *this))
            result = NEW_REF(Texture, FTexture3D, std::move(dst));
    }
        break;
    case RHI::EImageView::_Cube:
    {
        FTextureCube dst;
        if (Compression->CompressTexture(dst, *actualInput, *this))
            result = NEW_REF(Texture, FTextureCube, std::move(dst));
    }
        break;
    case RHI::EImageView::_CubeArray:
    {
        FTextureCubeArray dst;
        if (Compression->CompressTexture(dst, *actualInput, *this))
            result = NEW_REF(Texture, FTextureCubeArray, std::move(dst));
    }
        break;

    case RHI::EImageView::Unknown:
        AssertNotImplemented();
    }

    return result;
}
//----------------------------------------------------------------------------
bool FTextureGeneration::ResizeMip2D(
    const FTextureSourceProperties& properties,
    const uint2& outputDimensions,
    const FRawMemory& outputData,
    const uint2& inputDimensions,
    ETextureSourceFormat inputFormat,
    ETextureSourceFlags inputFlags,
    ETextureGammaSpace inputGamma,
    const FRawMemoryConst& inputData ) const {
    Assert_NoAssume(inputData.SizeInBytes() ==
        ETextureSourceFormat_SizeInBytes(inputFormat, (inputDimensions,1_u32)));
    Assert_NoAssume(outputData.SizeInBytes() ==
        ETextureSourceFormat_SizeInBytes(properties.Format, (outputDimensions,1_u32)));

    // resize the texture with stb_image_resize2
    {
        BENCHMARK_SCOPE_ARGS(Texture, "ResizeMip2D",
            {"Method", Opaq::Format(MipGeneration)},
            {"Output", Opaq::object_init{
                {"Dimensions", Opaq::array_init{outputDimensions.x, outputDimensions.y}},
                {"Format", Opaq::Format(properties.Format)},
                {"Flags", Opaq::Format(properties.Flags)},
                {"Gamma", Opaq::Format(properties.Gamma)},
            }},
            {"Input", Opaq::object_init{
                {"Dimensions", Opaq::array_init{inputDimensions.x, inputDimensions.y}},
                {"Format", Opaq::Format(inputFormat)},
                {"Flags", Opaq::Format(inputFlags)},
                {"Gamma", Opaq::Format(inputGamma)},
            }});

        // First off, you must ALWAYS call stbir_resize_init on your resize structure before any of the other calls!
        ::STBIR_RESIZE stbir_resize;
        ::stbir_resize_init(&stbir_resize,
            // input:
            inputData.data(),
            checked_cast<int>(inputDimensions.x),
            checked_cast<int>(inputDimensions.y),
            0 /* 0 stride -> contiguous in memory */,
            // output:
            outputData.data(),
            checked_cast<int>(outputDimensions.x),
            checked_cast<int>(outputDimensions.y),
            0 /* 0 stride -> contiguous in memory */,
            // layout:
            STBImagePixelLayout_(inputFormat, inputFlags),
            STBImageDataType_(inputFormat, inputGamma));

        // Check for format conversion
        if (properties.Format != inputFormat && properties.Flags != inputFlags) {
            ::stbir_set_datatypes(&stbir_resize,
                STBImageDataType_(inputFormat, inputGamma),
                STBImageDataType_(properties.Format, properties.Gamma));

            PPE_LOG_CHECK(Texture, !!::stbir_set_pixel_layouts(&stbir_resize,
                STBImagePixelLayout_(inputFormat, inputFlags),
                STBImagePixelLayout_(properties.Format, properties.Flags)));
        }

        // Set filtering algorithms
        switch (MipGeneration) {
        case ETextureMipGeneration::Default:
            if (properties.HasAlpha()) // default Mitchell/CatmullRom give bad results with fully transparent pixels
                ::stbir_set_filters(&stbir_resize, STBIR_FILTER_CUBICBSPLINE, STBIR_FILTER_CUBICBSPLINE);
            else
                ::stbir_set_filters(&stbir_resize, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT);
            break;
        case ETextureMipGeneration::Box:
            ::stbir_set_filters(&stbir_resize, STBIR_FILTER_BOX, STBIR_FILTER_BOX);
            break;
        case ETextureMipGeneration::CubicSpine:
            ::stbir_set_filters(&stbir_resize, STBIR_FILTER_CUBICBSPLINE, STBIR_FILTER_CUBICBSPLINE);
            break;
        case ETextureMipGeneration::CatmullRom:
            ::stbir_set_filters(&stbir_resize, STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM);
            break;
        case ETextureMipGeneration::PointSample:
            ::stbir_set_filters(&stbir_resize, STBIR_FILTER_POINT_SAMPLE, STBIR_FILTER_POINT_SAMPLE);
            break;
        case ETextureMipGeneration::GaussianBlur3: FALLTHROUGH();
        case ETextureMipGeneration::GaussianBlur5: FALLTHROUGH();
        case ETextureMipGeneration::GaussianBlur7: FALLTHROUGH();
        case ETextureMipGeneration::GaussianBlur9: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen1: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen2: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen3: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen4: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen5: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen6: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen7: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen8: FALLTHROUGH();
        case ETextureMipGeneration::ContrastAdaptiveSharpen9: FALLTHROUGH();
        case ETextureMipGeneration::MitchellNetrevalli:
            ::stbir_set_filters(&stbir_resize, STBIR_FILTER_MITCHELL, STBIR_FILTER_MITCHELL);
            break;
        }

        // Enable edge wrapping when tiling is enabled (slower)
        if (properties.Flags & ETextureSourceFlags::Tilable) {
            ::stbir_set_edgemodes(&stbir_resize, STBIR_EDGE_WRAP, STBIR_EDGE_WRAP);
        }

        // Retrieve task manager for multi-thread resize
        const FTaskManager& threadPool = FGlobalThreadPool::Get();

        // This will build samplers for threading.
        const int numSplits = ::stbir_build_samplers_with_splits(&stbir_resize,
            checked_cast<int>(threadPool.WorkerCount()));
        DEFERRED {
            ::stbir_free_samplers(&stbir_resize);
        };
        PPE_LOG_CHECK(Texture, numSplits > 0);

        const int numSucceeds = ParallelSum(0, checked_cast<size_t>(numSplits), [&stbir_resize](size_t splitIndex) -> int {
            // This function does a split of the resizing (you call this fuction for each
            // split, on multiple threads). A split is a piece of the output resize pixel space.
            PPE_LOG_CHECK(Texture, !!::stbir_resize_extended_split(&stbir_resize, checked_cast<int>(splitIndex), 1));
            return 1;

        }, ETaskPriority::Normal, threadPool.GlobalContext());
        PPE_LOG_CHECK(Texture, numSucceeds == numSplits);
    }

    // Handle optional post-processing
    switch (MipGeneration) {
    case ETextureMipGeneration::Default: break;
    case ETextureMipGeneration::Box: break;
    case ETextureMipGeneration::CubicSpine: break;
    case ETextureMipGeneration::CatmullRom: break;
    case ETextureMipGeneration::MitchellNetrevalli: break;
    case ETextureMipGeneration::PointSample: break;

    case ETextureMipGeneration::GaussianBlur3:
        GaussianBlur2D(properties, outputDimensions, outputData, 3);
        break;
    case ETextureMipGeneration::GaussianBlur5:
        GaussianBlur2D(properties, outputDimensions, outputData, 5);
        break;
    case ETextureMipGeneration::GaussianBlur7:
        GaussianBlur2D(properties, outputDimensions, outputData, 7);
        break;
    case ETextureMipGeneration::GaussianBlur9:
        GaussianBlur2D(properties, outputDimensions, outputData, 9);
        break;

    case ETextureMipGeneration::ContrastAdaptiveSharpen1:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.1f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen2:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.2f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen3:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.3f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen4:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.4f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen5:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.5f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen6:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.6f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen7:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.7f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen8:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.8f);
        break;
    case ETextureMipGeneration::ContrastAdaptiveSharpen9:
        ContrastAdaptiveSharpening2D(properties, outputDimensions, outputData, 0.9f);
        break;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FTextureGeneration::GenerateSliceMipChain2D(
    const FTextureSourceProperties& properties,
    const FRawMemory& sliceData ) const {
    PPE_LOG_CHECK(Texture,
        Meta::IsPow2(properties.Dimensions.x) &&
        Meta::IsPow2(properties.Dimensions.y) &&
        Meta::IsPow2(properties.Dimensions.z) );

    FRawMemory rawData = sliceData;
    uint3 mipDimensions = properties.Dimensions;
    FRawMemory outputMipData = rawData.Eat(ETextureSourceFormat_SizeInBytes(
        properties.Format, mipDimensions));

    float desiredAlphaTestCoverage = 0.0f;
    if (bPreserveAlphaTestCoverage2D and properties.HasAlpha() and  properties.HasMaskedAlpha() )
        desiredAlphaTestCoverage = AlphaTestCoverage2D(properties, mipDimensions.xy, outputMipData);

    forrange(mipLevel, 1, properties.NumMips) {
        const uint3 previousMipDimensions = mipDimensions;
        const FRawMemoryConst previousMipData = outputMipData;

        mipDimensions = FTextureSourceProperties::NextMipDimensions(mipDimensions);
        outputMipData = rawData.Eat(ETextureSourceFormat_SizeInBytes(
            properties.Format, mipDimensions));

        if (not ResizeMip2D(properties, mipDimensions.xy, outputMipData, previousMipDimensions.xy, properties.Format, properties.Flags, properties.Gamma, previousMipData))
            return false;

        if (desiredAlphaTestCoverage > 0)
            ScaleAlphaToCoverage2D(properties, mipDimensions.xy, outputMipData, desiredAlphaTestCoverage);
    }

    Assert_NoAssume(rawData.empty());
    return true;
}
//----------------------------------------------------------------------------
bool FTextureGeneration::FloodMipChainWithAlpha(
    const FTextureSourceProperties& properties,
    const FRawMemory& textureData ) NOEXCEPT {
    PPE_LOG_CHECK(Texture, properties.NumMips > 1);
    PPE_LOG_CHECK(Texture, properties.NumComponents() > 1);
    PPE_LOG_CHECK(Texture, properties.HasAlpha());

    BENCHMARK_SCOPE_ARGS(Texture, "FloodMipChainWithAlpha",
        {"Dimensions", Opaq::array_init{properties.Dimensions.x, properties.Dimensions.y, properties.Dimensions.z}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"NumMips", properties.NumMips} );

    // https://youtu.be/MKX45_riWQA?si=zyuadh4gH5oD3oSb&t=3026

    const RHI::FPixelFormatInfo pixelInfo = EPixelFormat_Infos(FTextureSourceProperties::PixelFormat(properties));

    RHI::FImageView::subpart_type subPartsSrc[1];
    RHI::FImageView::subpart_type subPartsDst[1];

    forrange(slice, 0, properties.NumSlices) {
        subPartsSrc[0] = properties.MipView(textureData, properties.NumMips - 1, 1, slice);
        RHI::FImageView viewSrc(subPartsSrc,
            properties.MipDimensions(properties.NumMips - 1),
            pixelInfo.Format,
            RHI::EImageAspect::Color,
            properties.IsTilable()  );

        reverseforrange(mip, 0, properties.NumMips - 1) {
            subPartsDst[0] = properties.MipView(textureData, mip, 1, slice);
            RHI::FImageView viewDst(subPartsDst,
                properties.MipDimensions(mip),
                pixelInfo.Format,
                RHI::EImageAspect::Color,
                properties.IsTilable() );

            ParallelFor(0, viewDst.Dimensions().y * viewDst.Dimensions().z, [&](size_t yz) {
                const u32 z = checked_cast<u32>(yz / viewDst.Dimensions().y);
                const u32 y = checked_cast<u32>(yz - z * viewDst.Dimensions().y);

                forrange(x, 0, viewDst.Dimensions().x) {
                    const float3 uvw = viewDst.PointToTexCoords(checked_cast<i32>(uint3{x,y,z}));

                    FRgba32f srcPixel;
                    viewSrc.Load(&srcPixel, uvw, RHI::ETextureFilter::Nearest);

                    FRgba32f dstPixel;
                    viewDst.Load(&dstPixel, uint3{x,y,z});

                    FLinearColor floodColor = AlphaBlend(
                        FLinearColor({srcPixel.xyz, 1}, properties.Gamma),
                        FLinearColor(dstPixel, properties.Gamma));
                    floodColor.A = dstPixel.w; // restore og alpha
                    viewDst.Store(uint3{x, y, z}, floodColor.LinearToGamma(properties.Gamma));
                }
            });

            subPartsSrc[0] = subPartsDst[0];
            viewSrc = RHI::FImageView(subPartsSrc,
                viewDst.Dimensions(),
                pixelInfo.Format,
                RHI::EImageAspect::Color );
        }
    }

    return true;
}
//----------------------------------------------------------------------------
float FTextureGeneration::AlphaTestCoverage2D(
    const FTextureSourceProperties& properties,
    const uint2& dimensions,
    const FRawMemoryConst& mipData,
    const float alphaScale ) const NOEXCEPT {
    BENCHMARK_SCOPE_ARGS(Texture, "AlphaTestCoverage2D",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"AlphaCutoff", AlphaCutoff},
        {"AlphaScale", alphaScale} );

    const RHI::FImageView::subpart_type subParts[1] = { mipData };
    const RHI::FImageView view{ subParts, {dimensions,1}, FTextureSourceProperties::PixelFormat(properties), RHI::EImageAspect::Color,
        properties.IsTilable() };

    std::atomic<float> coverage = 0.f;

    ParallelFor(0, view.Dimensions().y - 1, [&](size_t ly) {
        const u32 y = checked_cast<u32>(ly);

        float rowCoverage = 0.f;
        forrange(x, 0, view.Dimensions().x - 1) {
            FRgba32f pixel00, pixel01, pixel10, pixel11;
            view.Load(&pixel00, uint3{x+0,y+0,0});
            view.Load(&pixel10, uint3{x+1,y+0,0});
            view.Load(&pixel01, uint3{x+0,y+1,0});
            view.Load(&pixel11, uint3{x+1,y+1,0});

            const float4 alpha0123{
                Saturate(pixel00.w * alphaScale),
                Saturate(pixel10.w * alphaScale),
                Saturate(pixel11.w * alphaScale),
                Saturate(pixel01.w * alphaScale),
            };

            constexpr u32 superSampling = 4;
            u32 texelCoverage = 0;
            forrange(sy, 0, superSampling) {
                const float fy = (sy + 0.5f) / superSampling;

                forrange(sx, 0, superSampling) {
                    const float fx = (sx + 0.5f) / superSampling;

                    const float alpha = BilateralLerp(alpha0123.x, alpha0123.y, alpha0123.z, alpha0123.w, fx, fy);
                    if (alpha > AlphaCutoff)
                        texelCoverage++;
                }
            }

            rowCoverage += (texelCoverage / static_cast<float>(superSampling * superSampling));
        }

        coverage += rowCoverage;
    });

    return (coverage / static_cast<float>((dimensions.x - 1) * (dimensions.y - 1)));
}
//----------------------------------------------------------------------------
void FTextureGeneration::ScaleAlphaToCoverage2D(
    const FTextureSourceProperties& properties,
    const uint2& dimensions,
    const FRawMemory& mipData,
    const float desiredCoverage ) const NOEXCEPT {
    float minAlphaScale = 0.0f;
    float maxAlphaScale = 4.0f;
    float alphaScale = 1.0f;
    float bestAlphaScale = 1.0f;
    float bestError = FLT_MAX;
    BENCHMARK_SCOPE_ARGS(Texture, "ScaleAlphaToCoverage2D",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"AlphaCutoff", AlphaCutoff},
        {"DesiredCoverage", desiredCoverage},
        {"BestAlphaScale", bestAlphaScale},
        {"BestError", bestError} );

    // Determine desired scale using a binary search. Hard-coded to 10 steps max.
    for (int i = 0; i < 10; i++) {
        const float currentCoverage = AlphaTestCoverage2D(properties, dimensions, mipData, alphaScale);

        const float error = fabsf(currentCoverage - desiredCoverage);
        if (error < bestError) {
            bestError = error;
            bestAlphaScale = alphaScale;
        }

        if (currentCoverage < desiredCoverage)
            minAlphaScale = alphaScale;
        else if (currentCoverage > desiredCoverage)
            maxAlphaScale = alphaScale;
        else if (NearlyEquals(currentCoverage, desiredCoverage))
            break;

        alphaScale = (minAlphaScale + maxAlphaScale) * 0.5f;
    }

    // Scale alpha channel.
    ScaleBias(properties, {dimensions,1}, mipData, float4(1, 1, 1, bestAlphaScale), float4::Zero);
}
//----------------------------------------------------------------------------
void FTextureGeneration::ScaleBias(
    const FTextureSourceProperties& properties,
    const uint3& dimensions,
    const FRawMemory& mipData,
    const float4& scale,
    const float4& bias ) NOEXCEPT {
    BENCHMARK_SCOPE_ARGS(Texture, "ScaleBias",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y, dimensions.z}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"Scale", Opaq::array_init{scale.x, scale.y, scale.z, scale.w}},
        {"Bias", Opaq::array_init{bias.x, bias.y, bias.z, bias.w}} );

    const RHI::FImageView::subpart_type subParts[1] = { mipData };
    const RHI::FImageView view{ subParts, dimensions, FTextureSourceProperties::PixelFormat(properties), RHI::EImageAspect::Color,
        properties.IsTilable() };

    ParallelFor(0, view.Dimensions().y * view.Dimensions().z, [&](size_t yz) {
        const u32 z = checked_cast<u32>(yz / view.Dimensions().y);
        const u32 y = checked_cast<u32>(yz - z * view.Dimensions().y);

        forrange(x, 0, view.Dimensions().x) {
            FRgba32f pixel;
            view.Load(&pixel, uint3{x,y,z});

            pixel = Saturate(pixel * scale + bias);
            view.Store(uint3{x,y,z}, pixel);
        }
    });
}
//----------------------------------------------------------------------------
void FTextureGeneration::GenerateAlphaDistanceField2D(
    const FTextureSourceProperties& properties,
    const uint2& dimensions,
    const FRawMemory& mipData,
    const float spreadRatio01 ) const {
    Assert(spreadRatio01 >= 0 && spreadRatio01 <= 1);
    const i32 spreadDistance = Max(1, CeilToInt((float2(properties.Dimensions.xy) * Lerp(0.002f, 0.1f, spreadRatio01)).MaxComponent()));

    BENCHMARK_SCOPE_ARGS(Texture, "GenerateAlphaDistanceField2D",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"AlphaCutoff", AlphaCutoff},
        {"SpreadRatio01", spreadRatio01},
        {"SpreadDistance", spreadDistance});

    const RHI::FImageView::subpart_type subPartsDst[1] = { mipData };
    const RHI::FImageView viewDst{ subPartsDst, {dimensions,1}, FTextureSourceProperties::PixelFormat(properties), RHI::EImageAspect::Color,
        properties.IsTilable() };

    const TUniqueArray<u8> mipDataCopy{ NEW_ARRAY(Texture, u8, mipData.SizeInBytes()) };
    mipData.CopyTo(mipDataCopy);

    const RHI::FImageView::subpart_type subPartsSrc[1] = { mipDataCopy };
    const RHI::FImageView viewSrc{ subPartsSrc, {dimensions,1}, viewDst.Format(), RHI::EImageAspect::Color,
        properties.IsTilable() };

    // Brute force approach, brace yourself
    ParallelFor(0, viewDst.Dimensions().y, [&](size_t ly) {
        const u32 y = checked_cast<u32>(ly);

        forrange(x, 0, viewDst.Dimensions().x) {
            FRgba32f current;
            viewSrc.Load(&current, uint3{x,y,0});

            const bool bCurrentInside = (current.w > AlphaCutoff);

            i32 minDistance = spreadDistance;
            forrange(ny, -spreadDistance, spreadDistance+1)
            forrange(nx, -spreadDistance, spreadDistance+1) {
                FRgba32f pixel;
                viewSrc.Load(&pixel, int3{static_cast<i32>(x) + nx, static_cast<i32>(y) + ny, 0});

                const bool bPixelInside = (pixel.w > AlphaCutoff);
                if (bPixelInside == bCurrentInside)
                    continue;

                // use max norm distance (square) instead of euclidian (circle) to completely fill the sampling window
                // file:///C:/Users/bek4b/Downloads/Efficient_Max-Norm_Distance_Computation_for_Reliab.pdf
                const i32 maxNormDistance = Max(Abs(nx), Abs(ny));
                minDistance = Min(maxNormDistance, minDistance);

                if (minDistance == 1)
                    break; // early exit for nearby pixels
            }

            // store signed distance field where negative is outside, then remapped to [0,1]
            // so black is transparent and anything above 0.5 is visible
            // https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
            current.w = static_cast<float>(minDistance) / spreadDistance;
            if (not bCurrentInside)
                current.w = -current.w;
            current.w = Saturate(current.w * 0.5f + 0.5f);

            viewDst.Store(uint3{x,y,0}, current);
        }
    });
}
//----------------------------------------------------------------------------
void FTextureGeneration::ContrastAdaptiveSharpening2D(
    const FTextureSourceProperties& properties,
    const uint2& dimensions,
    const FRawMemory& mipData,
    const float sharpenFactor01 ) const {
    Assert(sharpenFactor01 >= 0 && sharpenFactor01 <= 1);

    BENCHMARK_SCOPE_ARGS(Texture, "ContrastAdaptiveSharpening2D",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"SharpenFactor01", sharpenFactor01},
        {"HasAlpha", properties.HasAlpha()} );

    const RHI::FImageView::subpart_type subPartsDst[1] = { mipData };
    const RHI::FImageView viewDst{ subPartsDst, {dimensions,1}, FTextureSourceProperties::PixelFormat(properties), RHI::EImageAspect::Color,
        properties.IsTilable() };

    const TUniqueArray<u8> mipDataCopy{ NEW_ARRAY(Texture, u8, mipData.SizeInBytes()) };
    mipData.CopyTo(mipDataCopy);

    const RHI::FImageView::subpart_type subPartsSrc[1] = { mipDataCopy };
    const RHI::FImageView viewSrc{ subPartsSrc, {dimensions,1}, viewDst.Format(), RHI::EImageAspect::Color,
        properties.IsTilable() };

    // Contrast Adaptive Sharpening (CAS)
    // Reference: Lou Kramer, FidelityFX CAS, AMD Developer Day 2019,
    // https://gpuopen.com/wp-content/uploads/2019/07/FidelityFX-CAS.pptx
    // McFly: https://www.shadertoy.com/view/tlc3zH
    if (properties.HasAlpha()) {
        ParallelFor(0, viewDst.Dimensions().y, [&](size_t ly) {
            const u32 y = checked_cast<u32>(ly);

            const float peak = Rcp(Lerp(8.f, 5.f, Saturate(sharpenFactor01)));

            forrange(x, 0, viewDst.Dimensions().x) {
                // fetch a 3x3 neighborhood around the pixel 'e',
                //  a b c
                //  d(e)f
                //  g h i
                FRgba32f a, b, c, d, e, f, g, h, i;

                viewSrc.Load(&e, uint3{x,y,0});
                if (e.w < Epsilon || (properties.HasMaskedAlpha() && e.w < AlphaCutoff)) { // quick-reject for transparent pixels
                    viewDst.Store(uint3{x,y,0}, e);
                    continue;
                }

                const float alpha = e.w;

                viewSrc.Load(&a, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&b, int3{static_cast<i32>(x) +  0, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&c, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&d, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) +  0, 0});
                viewSrc.Load(&f, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) +  0, 0});
                viewSrc.Load(&g, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) +  1, 0});
                viewSrc.Load(&h, int3{static_cast<i32>(x) +  0, static_cast<i32>(y) +  1, 0});
                viewSrc.Load(&i, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) +  1, 0});

                a = FLinearColor(a, properties.Gamma);
                b = FLinearColor(b, properties.Gamma);
                c = FLinearColor(c, properties.Gamma);
                d = FLinearColor(d, properties.Gamma);
                e = FLinearColor(e, properties.Gamma);
                f = FLinearColor(f, properties.Gamma);
                g = FLinearColor(g, properties.Gamma);
                h = FLinearColor(h, properties.Gamma);
                i = FLinearColor(i, properties.Gamma);

                if (properties.HasMaskedAlpha()) {
                    a.w = Step(AlphaCutoff, a.w);
                    b.w = Step(AlphaCutoff, b.w);
                    c.w = Step(AlphaCutoff, c.w);
                    d.w = Step(AlphaCutoff, d.w);
                    e.w = Step(AlphaCutoff, e.w);
                    f.w = Step(AlphaCutoff, f.w);
                    g.w = Step(AlphaCutoff, g.w);
                    h.w = Step(AlphaCutoff, h.w);
                    i.w = Step(AlphaCutoff, i.w);
                }

                a.xyz *= a.w;
                b.xyz *= b.w;
                c.xyz *= c.w;
                d.xyz *= d.w;
                e.xyz *= e.w;
                f.xyz *= f.w;
                g.xyz *= g.w;
                h.xyz *= h.w;
                i.xyz *= i.w;

                //McFly: vectorize math, even with scalar gcn hardware this should work
                //out the same, order of operations has not changed

                // Soft min and max.
                //  a b c             b
                //  d e f * 0.5  +  d e f * 0.5
                //  g h i             h
                // These are 2.0x bigger (factored out the extra multiply).

                float3 mnRGB = Min3(Min3(d.xyz, e.xyz, f.xyz), b.xyz, h.xyz);
                const float3 mnRGB2 = Min3(Min3(mnRGB, a.xyz, c.xyz), g.xyz, i.xyz);
                mnRGB += mnRGB2;

                float3 mxRGB = Max3(Max3(d.xyz, e.xyz, f.xyz), b.xyz, h.xyz);
                const float3 mxRGB2 = Max3(Max3(mxRGB, a.xyz, c.xyz), g.xyz, i.xyz);
                mxRGB += mxRGB2;

                if (AnyLess(mxRGB, float3(Epsilon))) {
                    // leave transparent pixels untouched
                    viewSrc.Load(&e, uint3{x, y, 0});
                    viewDst.Store(uint3{x, y, 0}, e);
                    continue;
                }

                // Smooth minimum distance to signal limit divided by smooth max.
                const float3 rcpMRGB = Rcp(mxRGB);
                float3 ampRGB = Saturate(Min(mnRGB, 2.f - mxRGB) * rcpMRGB);

                // Shaping amount of sharpening.
                ampRGB = Sqrt(ampRGB);

                // Filter shape.
                //  0 w 0
                //  w 1 w
                //  0 w 0
                const float3 wRGB = ampRGB * peak;
                const float3 rcpWeightRGB = Rcp(e.w + ((b.w + d.w) + (f.w + h.w)) * wRGB);

                //McFly: less instructions that way
                const float3 window = (b.xyz + d.xyz) + (f.xyz + h.xyz);
                const float3 outColor = Saturate((window * wRGB + e.xyz) * rcpWeightRGB);

                viewDst.Store(uint3{x,y,0}, FLinearColor(outColor, alpha).LinearToGamma(properties.Gamma));
            }
        });
    }
    else {
        ParallelFor(0, viewDst.Dimensions().y, [&](size_t ly) {
            const u32 y = checked_cast<u32>(ly);

            const float peak = Rcp(Lerp(8.f, 5.f, Saturate(sharpenFactor01)));

            forrange(x, 0, viewDst.Dimensions().x) {
                // fetch a 3x3 neighborhood around the pixel 'e',
                //  a b c
                //  d(e)f
                //  g h i
                FRgba32f a, b, c, d, e, f, g, h, i;
                viewSrc.Load(&a, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&b, int3{static_cast<i32>(x) +  0, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&c, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) + -1, 0});
                viewSrc.Load(&d, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) +  0, 0});
                viewSrc.Load(&e, uint3{x,y,0});
                viewSrc.Load(&f, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) +  0, 0});
                viewSrc.Load(&g, int3{static_cast<i32>(x) + -1, static_cast<i32>(y) +  1, 0});
                viewSrc.Load(&h, int3{static_cast<i32>(x) +  0, static_cast<i32>(y) +  1, 0});
                viewSrc.Load(&i, int3{static_cast<i32>(x) +  1, static_cast<i32>(y) +  1, 0});

                a = FLinearColor(a, properties.Gamma);
                b = FLinearColor(b, properties.Gamma);
                c = FLinearColor(c, properties.Gamma);
                d = FLinearColor(d, properties.Gamma);
                e = FLinearColor(e, properties.Gamma);
                f = FLinearColor(f, properties.Gamma);
                g = FLinearColor(g, properties.Gamma);
                h = FLinearColor(h, properties.Gamma);
                i = FLinearColor(i, properties.Gamma);

                //McFly: vectorize math, even with scalar gcn hardware this should work
                //out the same, order of operations has not changed

                // Soft min and max.
                //  a b c             b
                //  d e f * 0.5  +  d e f * 0.5
                //  g h i             h
                // These are 2.0x bigger (factored out the extra multiply).

                float3 mnRGB = Min3(Min3(d.xyz, e.xyz, f.xyz), b.xyz, h.xyz);
                const float3 mnRGB2 = Min3(Min3(mnRGB, a.xyz, c.xyz), g.xyz, i.xyz);
                mnRGB += mnRGB2;

                float3 mxRGB = Max3(Max3(d.xyz, e.xyz, f.xyz), b.xyz, h.xyz);
                const float3 mxRGB2 = Max3(Max3(mxRGB, a.xyz, c.xyz), g.xyz, i.xyz);
                mxRGB += mxRGB2;

                // Smooth minimum distance to signal limit divided by smooth max.
                const float3 rcpMRGB = Rcp(Max(mxRGB, float3(Epsilon)));
                float3 ampRGB = Saturate(Min(mnRGB, 2.f - mxRGB) * rcpMRGB);

                // Shaping amount of sharpening.
                ampRGB = Sqrt(ampRGB);

                // Filter shape.
                //  0 w 0
                //  w 1 w
                //  0 w 0
                const float3 wRGB = ampRGB * peak;
                const float3 rcpWeightRGB = Rcp(1.f + 4.f * wRGB);

                //McFly: less instructions that way
                const float3 window = (b.xyz + d.xyz) + (f.xyz + h.xyz);
                const float3 outColor = Saturate((window * wRGB + e.xyz) * rcpWeightRGB);

                viewDst.Store(uint3{x,y,0}, FLinearColor(outColor, e.w).LinearToGamma(properties.Gamma));
            }
        });
    }
}
//----------------------------------------------------------------------------
void FTextureGeneration::GaussianBlur2D(
    const FTextureSourceProperties& properties,
    const uint2& dimensions,
    const FRawMemory& mipData,
    const u32 windowSize/* = 5 */,
    const float sigma01/* = 1.0f */) const {
    Assert(windowSize > 0);
    Assert(sigma01 >= 0 && sigma01 <= 1);

    BENCHMARK_SCOPE_ARGS(Texture, "GaussianBlur2D",
        {"Dimensions", Opaq::array_init{dimensions.x, dimensions.y}},
        {"Format", Opaq::Format(properties.Format)},
        {"Flags", Opaq::Format(properties.Flags)},
        {"WindowSize", windowSize},
        {"Sigma01", sigma01});

    // prepare 1D Gaussian kernel
    const u32 kernelSize = (windowSize-1)/2;
    VECTORINSITU(Texture, float, 16) kernel(windowSize);

    const float sigma = Lerp(0.f, float(kernelSize), Saturate(sigma01));
    forrange(j, 0, kernelSize+1)
        kernel[kernelSize + j] = kernel[kernelSize - j] = NormPDF(float(j), sigma);

    float kernelZ = 0.f;
    forrange(j, 0, windowSize)
        kernelZ += kernel[j];

    // prepare 2 passes: 1 vertical, then one horizontal
    const RHI::FImageView::subpart_type subPartsA[1] = { mipData };
    const RHI::FImageView viewA{ subPartsA, {dimensions,1}, FTextureSourceProperties::PixelFormat(properties), RHI::EImageAspect::Color,
        properties.IsTilable() };

    const TUniqueArray<u8> mipDataCopy{ NEW_ARRAY(Texture, u8, mipData.SizeInBytes()) };

    const RHI::FImageView::subpart_type subPartsB[1] = { mipDataCopy };
    const RHI::FImageView viewB{ subPartsB, {dimensions,1}, viewA.Format(), RHI::EImageAspect::Color,
        properties.IsTilable() };

    struct FPassParams_ {
        TPtrRef<const FTextureGeneration> Generation;
        TPtrRef<const FTextureSourceProperties> Properties;
        TMemoryView<const float> Kernel;
        float KernelZ;
        i32 KernelSize;
        TPtrRef<const RHI::FImageView> ViewSrc;
        TPtrRef<const RHI::FImageView> ViewDst;
        int2 Direction;
    }   params;
    params.Generation = this;
    params.Properties = properties;
    params.Kernel = kernel;
    params.KernelZ = kernelZ;
    params.KernelSize = static_cast<i32>(kernelSize);

    const auto separableBlur = (properties.HasAlpha()
        ? [](const FPassParams_& params, u32 y) {
            forrange(x, 0, params.ViewSrc->Dimensions().x) {
                float4 color = float4::Zero;

                forrange(i, -params.KernelSize, params.KernelSize) {
                    FRgba32f pixel;
                    params.ViewSrc->Load(&pixel, int3{int2(x, y) + params.Direction * i,  0});

                    float4 lin = float4(FLinearColor(pixel, params.Properties->Gamma));

                    if (params.Properties->HasMaskedAlpha())
                        lin.w = Step(params.Generation->AlphaCutoff, lin.w);

                    lin.xyz *= lin.w; // weight by alpha

                    color += (params.Kernel[params.KernelSize + i] * lin);
                }

                if (color.w < Epsilon) {
                    // leave original colour untouched
                    params.ViewSrc->Load(&color, uint3{x, y, 0});
                    params.ViewDst->Store(uint3{x, y, 0}, color);
                }
                else {
                    color.xyz /= color.w;
                    color.w /= params.KernelZ;

                    params.ViewDst->Store(uint3{x, y, 0}, FLinearColor(color).LinearToGamma(params.Properties->Gamma));
                }
            }
        }
        : [](const FPassParams_& params, u32 y) {
            forrange(x, 0, params.ViewSrc->Dimensions().x) {
                float3 color = float3::Zero;

                forrange(i, -params.KernelSize, params.KernelSize) {
                    FRgba32f pixel;
                    params.ViewSrc->Load(&pixel, int3{int2(x, y) + params.Direction * i,  0});

                    color += params.Kernel[params.KernelSize + i] * float3(FLinearColor(pixel, params.Properties->Gamma));
                }

                color /= params.KernelZ;

                params.ViewDst->Store(uint3{x, y, 0}, FLinearColor(color).LinearToGamma(params.Properties->Gamma));
            }
        });

    // vertical pass
    params.ViewSrc = viewA;
    params.ViewDst = viewB;
    params.Direction = { 0, 1 };
    ParallelFor(0, viewA.Dimensions().y, [&separableBlur, &params](size_t ly) {
        separableBlur(params, checked_cast<u32>(ly));
    });

    // horizontal pass
    params.ViewSrc = viewB;
    params.ViewDst = viewA;
    params.Direction = { 1, 0 };
    ParallelFor(0, viewA.Dimensions().y, [&separableBlur, &params](size_t ly) {
        separableBlur(params, checked_cast<u32>(ly));
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
