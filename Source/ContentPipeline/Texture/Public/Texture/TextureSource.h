#pragma once

#include "Texture_fwd.h"

#include "TextureEnums.h"

#include "Color/Color.h"
#include "IO/BulkData.h"
#include "Maths/Range.h"
#include "Memory/MemoryView.h"
#include "Memory/SharedBuffer.h"
#include "Meta/Optional.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextureSourceProperties {
public:
    uint3 Dimensions{ 0 };

    u32 NumMips{ 0 };
    u32 NumSlices{ 0 };

    ETextureGammaSpace GammaSpace{ Default };
    ETextureSourceFlags Flags{ Default };
    ETextureSourceFormat Format{ Default };

    ETextureColorMask ColorMask{ Default };
    ETextureImageView ImageView{ Default };

    NODISCARD u32 Width() const { return Dimensions.x; }
    NODISCARD u32 Height() const { return Dimensions.y; }
    NODISCARD u32 Depth() const { return Dimensions.z; }

    NODISCARD PPE_TEXTURE_API bool HasAlpha() const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API bool HasPreMultipliedAlpha() const NOEXCEPT;

    NODISCARD PPE_TEXTURE_API bool IsHDR() const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API bool IsLongLatCubemap() const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API bool IsSRGB() const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API bool IsTiling() const NOEXCEPT;

    NODISCARD PPE_TEXTURE_API size_t SizeInBytes() const NOEXCEPT;

    NODISCARD PPE_TEXTURE_API FBytesRange MipRange(u32 mipBias, u32 numMips = 1, u32 sliceIndex = 0) const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API FRawMemory MipView(const FRawMemory& textureData, u32 mipBias, u32 numMips = 1, u32 sliceIndex = 0) const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API FRawMemoryConst MipView(const FRawMemoryConst& textureData, u32 mipBias, u32 numMips = 1, u32 sliceIndex = 0) const NOEXCEPT;

    NODISCARD PPE_TEXTURE_API FBytesRange SliceRange(u32 sliceIndex) const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API FRawMemory SliceView(const FRawMemory& textureData, u32 sliceIndex) const NOEXCEPT;
    NODISCARD PPE_TEXTURE_API FRawMemoryConst SliceView(const FRawMemoryConst& textureData, u32 sliceIndex) const NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties Texture2D(
        u32 width, u32 height,
        u32 numMips,
        ETextureColorMask colorMask,
        ETextureGammaSpace gammaSpace,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties Texture2DArray(
        u32 width, u32 height,
        u32 numMips,
        u32 numSlices,
        ETextureColorMask colorMask,
        ETextureGammaSpace gammaSpace,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties Texture2DWithMipChain(
        u32 width, u32 height,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties Texture2DArrayWithMipChain(
        u32 width, u32 height,
        u32 numSlices,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties TextureCubeWithMipChain(
        u32 width, u32 height,
        ETextureSourceFormat format,
        bool isLongLatCubemap) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties TextureCubeArrayWithMipChain(
        u32 width, u32 height,
        u32 numSlices,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static FTextureSourceProperties TextureVolumeWithMipChain(
        u32 width, u32 height, u32 depth,
        ETextureSourceFormat format) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static uint3 NextMipDimensions(const uint3& dimensions) NOEXCEPT;
    NODISCARD PPE_TEXTURE_API static u32 FullMipCount(const uint3& dimensions) NOEXCEPT;

    NODISCARD PPE_TEXTURE_API static bool ResizeMip2D(
        const uint2& outputDimensions,
        ETextureSourceFormat outputFormat,
        ETextureSourceFlags outputFlags,
        const FRawMemory& outputData,
        const uint2& inputDimensions,
        ETextureSourceFormat inputFormat,
        ETextureSourceFlags inputFlags,
        const FRawMemoryConst& inputData);

    NODISCARD PPE_TEXTURE_API static bool GenerateMipChain2D(
        const FTextureSourceProperties& properties,
        const FRawMemory& sliceData );

    NODISCARD PPE_TEXTURE_API static bool ResizeWithMipChain(
        const FTextureSourceProperties& outputProperties,
        const FRawMemory& outputData,
        const FTextureSourceProperties& inputProperties,
        const FRawMemoryConst& inputData);
};
//----------------------------------------------------------------------------
class FTextureSource final : public FRefCountable {
public:
    FTextureSource() = default;
    FTextureSource(FTextureSource&& ) = default;

    NODISCARD const FBulkData& BulkData() const { return _bulkData; }

    NODISCARD const uint3& Dimensions() const { return _properties.Dimensions; }
    NODISCARD u32 NumMips() const { return _properties.NumMips; }
    NODISCARD u32 NumSlices() const { return _properties.NumSlices; }

    NODISCARD ETextureGammaSpace GammaSpace() const { return _properties.GammaSpace; }
    NODISCARD ETextureSourceFlags Flags() const { return _properties.Flags; }
    NODISCARD ETextureSourceFormat Format() const { return _properties.Format; }

    NODISCARD ETextureColorMask ColorMask() const { return _properties.ColorMask; }
    NODISCARD ETextureImageView ImageView() const { return _properties.ImageView; }

    NODISCARD u32 Width() const { return _properties.Width(); }
    NODISCARD u32 Height() const { return _properties.Height(); }
    NODISCARD u32 Depth() const { return _properties.Depth(); }

    NODISCARD bool HasAlpha() const { return _properties.HasAlpha(); }
    NODISCARD bool HasPreMultipliedAlpha() const { return _properties.HasPreMultipliedAlpha(); }

    NODISCARD bool IsHDR() const { return _properties.IsHDR(); }
    NODISCARD bool IsLongLatCubemap() const { return _properties.IsLongLatCubemap(); }
    NODISCARD bool IsSRGB() const { return _properties.IsSRGB(); }

    NODISCARD ETextureSourceCompression Compression() const { return _compression; }
    NODISCARD const FTextureSourceProperties& Properties() const { return _properties; }

    PPE_TEXTURE_API void Construct(
        const FTextureSourceProperties& properties,
        Meta::TOptional<FUniqueBuffer>&& optionalBuffer = {});

    void Construct(
        ETextureImageView imageView,
        u32 width, u32 height, u32 depth,
        u32 numMips,
        u32 numSlices,
        ETextureColorMask colorMask,
        ETextureGammaSpace gammaSpace,
        ETextureSourceFlags flags,
        ETextureSourceFormat format,
        Meta::TOptional<FUniqueBuffer>&& optionalBuffer = {}) {
        return Construct(FTextureSourceProperties{
            .Dimensions = {width, height, depth},
            .NumMips = numMips,
            .NumSlices = numSlices,
            .GammaSpace = gammaSpace,
            .Flags = flags,
            .Format = format,
            .ColorMask = colorMask,
            .ImageView = imageView,
        }, std::move(optionalBuffer));
    }

    void Construct2D(
        u32 width, u32 height,
        u32 numMips,
        ETextureColorMask colorMask,
        ETextureGammaSpace gammaSpace,
        ETextureSourceFormat format,
        Meta::TOptional<FUniqueBuffer>&& optionalBuffer = {}) {
        Construct(FTextureSourceProperties::Texture2D(width, height, numMips, colorMask, gammaSpace, format), std::move(optionalBuffer));
    }

    void Construct2DArray(
        u32 width, u32 height,
        u32 numMips,
        u32 numSlices,
        ETextureColorMask colorMask,
        ETextureGammaSpace gammaSpace,
        ETextureSourceFormat format,
        Meta::TOptional<FUniqueBuffer>&& optionalBuffer = {}) {
        Construct(FTextureSourceProperties::Texture2DArray(width, height, numMips, numSlices, colorMask, gammaSpace, format), std::move(optionalBuffer));
    }

    void Construct2DWithMipChain(
        u32 width, u32 height,
        ETextureSourceFormat format) {
        Construct(FTextureSourceProperties::Texture2DWithMipChain(width, height, format));
    }

    void Construct2DArrayWithMipChain(
        u32 width, u32 height,
        u32 numSlices,
        ETextureSourceFormat format) {
        Construct(FTextureSourceProperties::Texture2DArrayWithMipChain(width, height, numSlices, format));
    }

    void ConstructCubeWithMipChain(
        u32 width, u32 height,
        ETextureSourceFormat format,
        bool isLongLatCubemap) {
        Construct(FTextureSourceProperties::TextureCubeWithMipChain(width, height, format, isLongLatCubemap));
    }

    void ConstructCubeArrayWithMipChain(
        u32 width, u32 height,
        u32 numSlices,
        ETextureSourceFormat format) {
        Construct(FTextureSourceProperties::TextureCubeArrayWithMipChain(width, height, numSlices, format));
    }

    void ConstructVolumeWithMipChain(
        u32 width, u32 height, u32 depth,
        ETextureSourceFormat format) {
        Construct(FTextureSourceProperties::TextureVolumeWithMipChain(width, height, depth, format));
    }

    PPE_TEXTURE_API void TearDown();

    void AttachSourceFile(const FFilename& sourceFile) { _bulkData.AttachSourceFile(sourceFile); }

    NODISCARD PPE_TEXTURE_API bool GenerateMipChain2D();

    NODISCARD PPE_TEXTURE_API FTextureSource Resize(const uint3& dimensions, u32 numMips = 1) const;
    NODISCARD PPE_TEXTURE_API Meta::TOptional<FTextureSource> Resize(
        const uint3& dimensions,
        u32 numMips = 0,
        ETextureSourceFormat format = Default,
        ETextureSourceFlags flags = Default) const;

    NODISCARD PPE_TEXTURE_API FSharedBuffer LockRead() const;
    PPE_TEXTURE_API void UnlockRead(FSharedBuffer&& data) const;

    NODISCARD PPE_TEXTURE_API FUniqueBuffer LockWrite();
    PPE_TEXTURE_API void UnlockWrite(FUniqueBuffer&& data);

    struct FReaderScope : Meta::FNonCopyableNorMovable {
        explicit FReaderScope(const FTextureSource& source) : Source(source) {
            Buffer = Source.LockRead();
        }

        ~FReaderScope() {
            Source.UnlockRead(std::move(Buffer));
        }

        FRawMemoryConst MakeView() const { return Buffer.MakeView(); }

        FRawMemoryConst MipData(u32 mipBias, u32 numMips = 1, u32 sliceIndex = 0) const {
            return Source._properties.MipView(Buffer.MakeView(), mipBias, numMips, sliceIndex);
        }

        FRawMemoryConst SliceData(u32 sliceIndex) const {
            return Source._properties.SliceView(Buffer.MakeView(), sliceIndex);
        }

        const FTextureSource& Source;
        FSharedBuffer Buffer;
    };

    struct FWriterScope : Meta::FNonCopyableNorMovable {
        explicit FWriterScope(FTextureSource& source) : Source(source) {
            Buffer = Source.LockWrite();
        }

        ~FWriterScope() {
            Source.UnlockWrite(std::move(Buffer));
        }

        FRawMemory MakeView() { return Buffer.MakeView(); }

        FRawMemory MipData(u32 mipBias, u32 numMips = 1, u32 sliceIndex = 0) {
            return Source._properties.MipView(Buffer.MakeView(), mipBias, numMips, sliceIndex);
        }

        FRawMemory SliceData(u32 sliceIndex) {
            return Source._properties.SliceView(Buffer.MakeView(), sliceIndex);
        }

        FTextureSource& Source;
        FUniqueBuffer Buffer;
    };

private:
    FBulkData _bulkData;

    FTextureSourceProperties _properties = Default;
    ETextureSourceCompression _compression = Default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
