// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/TextureSource.h"

#include "Texture/TextureEnums.h"

#include "RHI/ResourceEnums.h"
#include "RHI/RenderStateEnums.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMaths.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Thread/ThreadPool.h"
#include "Thread/Task/TaskHelpers.h"

// will compile stb_image_resize2 in this translation unit:
#include "stb_image_resize2-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR ::stbir_datatype STBImageDataType_(ETextureSourceFormat fmt, ETextureSourceFlags flags) {
    switch (fmt) {
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::BGRA8:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::RGBA8:
        return (flags ^ ETextureSourceFlags::SRGB ? STBIR_TYPE_UINT8_SRGB : STBIR_TYPE_UINT8);

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
NODISCARD FTextureSourceProperties PrepareTextureSourceResize_(
    const FTextureSourceProperties& input,
    const uint3& dimensions,
    u32 numMips = 0,
    ETextureSourceFormat format = Default,
    ETextureSourceFlags flags = Default) {
    Assert_NoAssume(input.Dimensions != dimensions);

    FTextureSourceProperties output = input;
    output.Dimensions = dimensions;

    if (numMips != 0)
        output.NumMips = numMips;

    if (format != Default) {
        PPE_LOG_CHECK(Texture, ETextureSourceFormat_Components(format) == ETextureSourceFormat_Components(input.Format));
        output.Format = format;
    }

    if (flags != Default)
        output.Flags += flags;

    return output;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FTextureSourceProperties::HasAlpha() const NOEXCEPT {
    return (ColorMask & ETextureColorMask::A);
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
bool FTextureSourceProperties::IsSRGB() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::SRGB);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::IsTiling() const NOEXCEPT {
    return (Flags & ETextureSourceFlags::Tiling);
}
//----------------------------------------------------------------------------
size_t FTextureSourceProperties::SizeInBytes() const NOEXCEPT {
    return ETextureSourceFormat_SizeInBytes(Format, Dimensions, NumMips, NumSlices);
}
//----------------------------------------------------------------------------
FBytesRange FTextureSourceProperties::MipRange(u32 mipBias, u32 numMips, u32 sliceIndex) const NOEXCEPT {
    Assert(numMips > 0);
    Assert_NoAssume(mipBias + numMips <= NumMips);
    const FBytesRange sliceRange = ETextureSourceFormat_SliceRange(Format, Dimensions, NumMips, sliceIndex);
    const FBytesRange mipRange = ETextureSourceFormat_MipRange(Format, Dimensions, mipBias, mipBias + numMips);
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
        .GammaSpace = gammaSpace,
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
        .GammaSpace = gammaSpace,
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
        .GammaSpace = ETextureGammaSpace::sRGB,
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
        .GammaSpace = ETextureGammaSpace::sRGB,
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
        .GammaSpace = ETextureGammaSpace::sRGB,
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
        .GammaSpace = ETextureGammaSpace::sRGB,
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
        .GammaSpace = ETextureGammaSpace::sRGB,
        .Flags = Default,
        .Format = format,
        .ColorMask = ETextureColorMask::All,
        .ImageView = ETextureImageView::_3D,
    };
}
//----------------------------------------------------------------------------
uint3 FTextureSourceProperties::NextMipDimensions(const uint3& dimensions) NOEXCEPT {
    Assert_NoAssume(Meta::IsPow2(dimensions.x));
    Assert_NoAssume(Meta::IsPow2(dimensions.y));
    Assert_NoAssume(Meta::IsPow2(dimensions.z));

    return Max(dimensions / 2_u32, uint3::One);
}
//----------------------------------------------------------------------------
u32 FTextureSourceProperties::FullMipCount(const uint3& dimensions) NOEXCEPT {
    Assert_NoAssume(AllGreater(dimensions, uint3::Zero));

    return (FPlatformMaths::FloorLog2(dimensions.MaxComponent()) + 1);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::ResizeMip2D(
    const uint2& outputDimensions,
    ETextureSourceFormat outputFormat,
    ETextureSourceFlags outputFlags,
    const FRawMemory& outputData,
    const uint2& inputDimensions,
    ETextureSourceFormat inputFormat,
    ETextureSourceFlags inputFlags,
    const FRawMemoryConst& inputData ) {
    Assert_NoAssume(inputData.SizeInBytes() ==
        ETextureSourceFormat_SizeInBytes(inputFormat, (inputDimensions,1_u32)));
    Assert_NoAssume(outputData.SizeInBytes() ==
        ETextureSourceFormat_SizeInBytes(outputFormat, (outputDimensions,1_u32)));

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
        STBImageDataType_(inputFormat, inputFlags));

    // Check for format conversion
    if (outputFormat != inputFormat && outputFlags != inputFlags) {
        ::stbir_set_datatypes(&stbir_resize,
            STBImageDataType_(inputFormat, inputFlags),
            STBImageDataType_(outputFormat, outputFlags));

        PPE_LOG_CHECK(Texture, !!::stbir_set_pixel_layouts(&stbir_resize,
            STBImagePixelLayout_(inputFormat, inputFlags),
            STBImagePixelLayout_(outputFormat, outputFlags)));
    }

    // Enable edge wrapping when tiling is enabled (slower)
    if (outputFlags & ETextureSourceFlags::Tiling) {
        ::stbir_set_edgemodes(&stbir_resize, STBIR_EDGE_WRAP, STBIR_EDGE_WRAP);
    }

    // Retrieve task manager for multi-thread resize
    const FTaskManager& threadPool = FBackgroundThreadPool::Get();

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

    return (numSucceeds == numSplits);
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::GenerateMipChain2D(
    const FTextureSourceProperties& properties,
    const FRawMemory& sliceData) {
    size_t mipOffset = sliceData.SizeInBytes();
    uint3 mipDimensions = properties.Dimensions;

    FRawMemory outputMipData = sliceData.CutBefore(
        ETextureSourceFormat_SizeInBytes(properties.Format, mipDimensions));

    forrange(mipLevel, 1, properties.NumMips) {
        const uint3 previousMipDimensions = mipDimensions;
        const FRawMemoryConst previousMipData = outputMipData;

        mipDimensions = NextMipDimensions(mipDimensions);
        outputMipData = sliceData.SubRange(mipOffset,
            ETextureSourceFormat_SizeInBytes(properties.Format, mipDimensions));

        if (not ResizeMip2D(
            mipDimensions.xy,
            properties.Format,
            properties.Flags,
            outputMipData,
            previousMipDimensions.xy,
            properties.Format,
            properties.Flags,
            previousMipData))
            return false;

        mipOffset += outputMipData.SizeInBytes();
    }

    return true;
}
//----------------------------------------------------------------------------
bool FTextureSourceProperties::ResizeWithMipChain(
    const FTextureSourceProperties& outputProperties,
    const FRawMemory& outputData,
    const FTextureSourceProperties& inputProperties,
    const FRawMemoryConst& inputData) {
    Assert(outputProperties.NumSlices <= inputProperties.NumSlices);

    forrange(sliceIndex, 0, Min(inputProperties.NumSlices, outputProperties.NumSlices)) {
        const FRawMemory outputMipData = outputProperties.MipView(outputData, 0, 1, sliceIndex);

        // resize the top mip:
        if (not ResizeMip2D(
            outputProperties.Dimensions.xy,
            outputProperties.Format,
            outputProperties.Flags,
            outputMipData,
            inputProperties.Dimensions.xy,
            inputProperties.Format,
            inputProperties.Flags,
            inputProperties.MipView(inputData, 0)))
            return false;

        // generate other mip levels from previous mip:
        if (not GenerateMipChain2D(outputProperties, outputMipData))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FTextureSource::Construct(
    const FTextureSourceProperties& properties,
    Meta::TOptional<FUniqueBuffer>&& optionalBuffer) {
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
        if (optionalBuffer.has_value()) {
            FBulkData::FWriteScope writer{_bulkData};
            writer.Buffer = std::move(*optionalBuffer);
        }
        else {
            _bulkData.Resize_DiscardData(decompressedSizeInBytes);
        }
    }
    else {
        AssertReleaseMessage("can't suppy optional data to copy when compression is enabled", not optionalBuffer.has_value());
    }
}
//----------------------------------------------------------------------------
void FTextureSource::TearDown() {
    _bulkData.Reset();

    _properties = Default;
    _compression = Default;
}
//----------------------------------------------------------------------------
bool FTextureSource::GenerateMipChain2D() {
    FWriterScope exclusiveTexture(*this);
    forrange(sliceIndex, 0, _properties.NumSlices) {
        if (not FTextureSourceProperties::GenerateMipChain2D(_properties, exclusiveTexture.SliceData(sliceIndex)))
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
FTextureSource FTextureSource::Resize(const uint3& dimensions, u32 numMips/* = 1 */) const {
    Meta::TOptional<FTextureSource> result = Resize(
        dimensions, numMips, ETextureSourceFormat::Unknown, ETextureSourceFlags::Unknown);
    AssertRelease(result.has_value());
    return std::move(*result);
}
//----------------------------------------------------------------------------
Meta::TOptional<FTextureSource> FTextureSource::Resize(
    const uint3& dimensions,
    u32 numMips/* = 0 */,
    ETextureSourceFormat format/* = Default */,
    ETextureSourceFlags flags/* = Default */) const {
    MEMORYDOMAIN_THREAD_SCOPE(Texture);
    const FReaderScope sharedTexture(*this);

    const FTextureSourceProperties outputProperties = PrepareTextureSourceResize_(
        _properties, dimensions, numMips, format, flags);

    FUniqueBuffer outputBuffer = FUniqueBuffer::Allocate(outputProperties.SizeInBytes());
    PPE_LOG_CHECKEX(Texture, Meta::TOptional<FTextureSource>{}, !!outputBuffer);

    if (not FTextureSourceProperties::ResizeWithMipChain(
        outputProperties, outputBuffer.MakeView(),
        _properties, sharedTexture.Buffer.MakeView()))
        return Meta::TOptional<FTextureSource>{};

    FTextureSource resizedTexture;
    resizedTexture.Construct(outputProperties, std::move(outputBuffer));
    return resizedTexture;
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
