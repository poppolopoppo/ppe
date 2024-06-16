#pragma once

#include "Texture_fwd.h"

#include "TextureCompression.h"
#include "TextureEnums.h"

#include "Meta/Optional.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETextureMipGeneration : u8 {
    Default                     = 0,    // Use best filter for down/up-sampling automatically
    Unknown                     = Default,

    Box                         ,       // A trapezoid w/1-pixel wide ramps, same result as box for integer scale ratios
    CubicSpine                  ,       // The cubic b-spline (aka Mitchell-Netrevalli with B=1,C=0), gaussian-esque
    CatmullRom                  ,       // An interpolating cubic spline
    MitchellNetrevalli          ,       // Mitchell-Netrevalli filter with B=1/3, C=1/3
    PointSample                 ,       // Simple point sampling

    GaussianBlur3               ,       // Mitchell-Netrevalli + Gaussian blur with kernel size = 3
    GaussianBlur5               ,       // Mitchell-Netrevalli + Gaussian blur with kernel size = 5
    GaussianBlur7               ,       // Mitchell-Netrevalli + Gaussian blur with kernel size = 7
    GaussianBlur9               ,       // Mitchell-Netrevalli + Gaussian blur with kernel size = 9

    ContrastAdaptiveSharpen1    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.1
    ContrastAdaptiveSharpen2    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.2
    ContrastAdaptiveSharpen3    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.3
    ContrastAdaptiveSharpen4    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.4
    ContrastAdaptiveSharpen5    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.5
    ContrastAdaptiveSharpen6    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.6
    ContrastAdaptiveSharpen7    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.7
    ContrastAdaptiveSharpen8    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.8
    ContrastAdaptiveSharpen9    ,       // Mitchell-Netrevalli + Contrast adaptive sharpen with sharpen = 0.9
};
//----------------------------------------------------------------------------
class FTextureGeneration : public FTextureCompressionSettings {
public:
    UTextureCompression Compression;

    Meta::TOptional<uint3> ResizeDimensions;
    Meta::TOptional<ETextureSourceFlags> ResizeFlags;
    Meta::TOptional<ETextureSourceFormat> ResizeFormat;

    float AlphaCutoff{ 0.5f };
    float AlphaSpreadRatio{ 0.2f };

    ETextureMipGeneration MipGeneration{ Default };

    bool bFloodMipChainWithAlpha{ false };
    bool bGenerateAlphaDistanceField2D{ false };
    bool bGenerateFullMipChain2D{ false };
    bool bPreserveAlphaTestCoverage2D{ false };

    FTextureGeneration() = default;

    PPE_TEXTURE_API explicit FTextureGeneration(const FTextureSourceProperties& properties) NOEXCEPT;
    PPE_TEXTURE_API explicit FTextureGeneration(const ITextureService& service, const FTextureSourceProperties& properties) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API FTextureSourceProperties Prepare(const FTextureSourceProperties& source) const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API PTexture Generate(const FTextureSource& source) const;

private:
    NODISCARD bool ResizeMip2D(
        const FTextureSourceProperties& properties,
        const uint2& outputDimensions,
        const FRawMemory& outputData,
        const uint2& inputDimensions,
        ETextureSourceFormat inputFormat,
        ETextureSourceFlags inputFlags,
        ETextureGammaSpace inputGamma,
        const FRawMemoryConst& inputData ) const;

    NODISCARD bool GenerateSliceMipChain2D(
        const FTextureSourceProperties& properties,
        const FRawMemory& sliceData ) const;

    NODISCARD static bool FloodMipChainWithAlpha(
        const FTextureSourceProperties& properties,
        const FRawMemory& textureData ) NOEXCEPT;

    NODISCARD float AlphaTestCoverage2D(
        const FTextureSourceProperties& properties,
        const uint2& dimensions,
        const FRawMemoryConst& mipData,
        const float alphaScale = 1.0f ) const NOEXCEPT;

    void ScaleAlphaToCoverage2D(
        const FTextureSourceProperties& properties,
        const uint2& dimensions,
        const FRawMemory& mipData,
        const float desiredCoverage ) const NOEXCEPT;

    static void ScaleBias(
        const FTextureSourceProperties& properties,
        const uint3& dimensions,
        const FRawMemory& mipData,
        const float4& scale,
        const float4& bias ) NOEXCEPT;

    void GenerateAlphaDistanceField2D(
        const FTextureSourceProperties& properties,
        const uint2& dimensions,
        const FRawMemory& mipData,
        const float spreadRatio01 = 0.1f ) const;

    void ContrastAdaptiveSharpening2D(
        const FTextureSourceProperties& properties,
        const uint2& dimensions,
        const FRawMemory& mipData,
        const float sharpenFactor01 = 1.0f ) const;

    void GaussianBlur2D(
        const FTextureSourceProperties& properties,
        const uint2& dimensions,
        const FRawMemory& mipData,
        const u32 windowSize = 5,
        const float sigma01 = 1.0f ) const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
