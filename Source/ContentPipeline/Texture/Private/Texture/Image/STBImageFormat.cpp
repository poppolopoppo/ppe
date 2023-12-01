// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture/Image/STBImageFormat.h"

#include "Texture/TextureEnums.h"
#include "Texture/TextureSource.h"

#include "RHI/EnumToString.h"
#include "RHI/ResourceEnums.h"
#include "RHI/RenderStateEnums.h"

#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"

// will compile stb_image in this translation unit:
#include "stb_image-impl.h"
// will compile stb_image_write in this translation unit:
#include "stb_image_write-impl.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR const ::stbi_io_callbacks GSTBStreamReaderIOCallbacks_ = ::stbi_io_callbacks{
    // fill 'data' with 'size' bytes.  return number of bytes actually read
    .read = [](void* user, char* data, int size) -> int {
        auto* const stream_reader = static_cast<IStreamReader*>(user);
        return checked_cast<int>(stream_reader->ReadSome(data, 1, size));
    },
    // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
    .skip = [](void* user, int n) -> void {
        auto* const stream_reader = static_cast<IStreamReader*>(user);
        stream_reader->SeekI(n, ESeekOrigin::Relative);
    },
    // returns nonzero if we are at end of file/data
    .eof = [](void* user) -> int {
        const auto* const stream_reader = static_cast<IStreamReader*>(user);
        return stream_reader->Eof();
    }
};
//----------------------------------------------------------------------------
static void STBImageWriteFunc_(void* context, void* data, int size) {
    VerifyRelease(static_cast<IStreamWriter*>(context)->Write(data, size));
}
//----------------------------------------------------------------------------
NODISCARD static bool STBImageInitSourceProperties_(
    FTextureSourceProperties* outProperties,
    RHI::EImageView imageView,
    RHI::EColorMask colorMask,
    const uint2& dimensions,
    ETextureSourceFlags flags,
    ETextureSourceFormat format) {
    Assert(outProperties);
    PPE_LOG_CHECK(Texture, AllGreater(dimensions, uint2::Zero));

    *outProperties = FTextureSourceProperties{};
    outProperties->NumMips = 1;
    outProperties->NumSlices = 1;
    outProperties->Flags = flags;
    outProperties->Format = format;
    outProperties->ImageView = imageView;
    outProperties->ColorMask = colorMask;
    outProperties->GammaSpace = EGammaSpace::sRGB;

    if (outProperties->IsHDR())
        outProperties->GammaSpace = EGammaSpace::Linear;
    else
        outProperties->Flags |= ETextureSourceFlags::SRGB;

    const auto fillNumberOfVerticalSlices = [&](u32* outSlices) -> bool {
        PPE_LOG_CHECK(Texture, (dimensions.y % dimensions.x) == 0);
        *outSlices = (dimensions.y / dimensions.x);
        return true;
    };

    switch (imageView) {
    case RHI::EImageView::_1D:
        PPE_LOG_CHECK(Texture, dimensions.y == 1);
        outProperties->Dimensions = uint3(dimensions.x, 1, 1);
        return true;

    case RHI::EImageView::_2D:
        outProperties->Dimensions = uint3(dimensions, 1);
        return true;

    case RHI::EImageView::_3D:
        // 3D texture imports a 2D texture and split it vertically in width x width depth slices
        outProperties->Dimensions = uint3(dimensions, 1);
        return fillNumberOfVerticalSlices(&outProperties->Dimensions.z);

    case RHI::EImageView::_1DArray:
        // 1D array imports as 2D texture and split each row in slices
        outProperties->Dimensions = uint3(dimensions.x, 1, 1);
        outProperties->NumSlices = dimensions.y;
        return true;

    case RHI::EImageView::_2DArray:
        // 2D array imports a 2D texture and split it vertically in width x width slices
        outProperties->Dimensions = uint3(dimensions.xx, 1);
        return fillNumberOfVerticalSlices(&outProperties->NumSlices);

    case RHI::EImageView::_Cube:
        // Cube texture imports a 2D texture and split it vertically in 6 x width x width faces
        outProperties->Dimensions = uint3(dimensions.xx, 1);
        if (not fillNumberOfVerticalSlices(&outProperties->NumSlices))
            return false;
        PPE_LOG_CHECK(Texture, outProperties->NumSlices == 6);
        return true;

    case RHI::EImageView::_CubeArray:
        // Cube array imports a 2D texture and split it vertically in 6 x width x width cube maps
        outProperties->Dimensions = uint3(dimensions.xx, 1);
        if (not fillNumberOfVerticalSlices(&outProperties->NumSlices))
            return false;
        PPE_LOG_CHECK(Texture, (outProperties->NumSlices % 6) == 0);
        return true;

    case RHI::EImageView::Unknown:
        AssertNotReached();
    }

    return false;
}
//----------------------------------------------------------------------------
NODISCARD static FTextureImporterResult STBImageLoadFromStream_(
    FTextureSourceProperties* outProperties,
    RHI::EImageView imageView,
    IStreamReader& input) {

    ETextureSourceFormat format;
    int2 dimensions;
    int components = 0;
    RHI::EColorMask colorMask = Default;
    ETextureSourceFlags flags = Default;
    PPE_LOG_CHECKEX(Texture, FTextureImporterResult{}, ::stbi_info_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input,
        &dimensions.x, &dimensions.y, &components));

    switch (components) {
    case 1: colorMask = RHI::EColorMask::G; break;
    case 2: colorMask = RHI::EColorMask::RA; break;
    case 3: colorMask = RHI::EColorMask::RGB; break;
    case 4: colorMask = RHI::EColorMask::RGBA; break;
    default:
        PPE_LOG_CHECKEX(Texture, FTextureImporterResult{}, components >= 0 && components <= 4);
        break;
    }

    void* stbi_data;
    if (!!::stbi_is_hdr_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input)) {
        components = 4; /* always RGBA32f */
        flags += ETextureSourceFlags::HDR;
        format = ETextureSourceFormat::RGBA32f;

        stbi_data = ::stbi_loadf_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input,
            &dimensions.x, &dimensions.y, &components, components);
    }
    else
    if (!!::stbi_is_16_bit_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input)) {
        format = ETextureSourceFormat::G16;
        if (components == 2) {
            format = ETextureSourceFormat::RA16;
        } else if (components > 2) {
            components = 4;
            format = ETextureSourceFormat::RGBA16;
        }

        stbi_data = ::stbi_load_16_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input, &dimensions.x, &dimensions.y, &components, components);
    }
    else {
        format = ETextureSourceFormat::G8;
        if (components == 2) {
            format = ETextureSourceFormat::RA8;
        } else if (components > 2) {
            components = 4;
            format = ETextureSourceFormat::RGBA8;
        }

        stbi_data = ::stbi_load_from_callbacks(&GSTBStreamReaderIOCallbacks_, &input, &dimensions.x, &dimensions.y, &components, components);
    }

    if (Unlikely(nullptr == stbi_data)) {
        PPE_LOG(Texture, Error, "stb_image failed to import texture: {0}", MakeCStringView(::stbi_failure_reason()));
        return FTextureImporterResult{};
    }

    if (Unlikely(not STBImageInitSourceProperties_(outProperties, imageView, colorMask, checked_cast<u32>(dimensions), flags, format))) {
        PPE_LOG(Texture, Error, "stb_image has an incorrect image view: {0}", imageView);
        return FTextureImporterResult{};
    }

    return FUniqueBuffer::TakeOwn(stbi_data, outProperties->SizeInBytes(), &stbi_image_free);
}
//----------------------------------------------------------------------------
NODISCARD static FTextureImporterResult STBImageLoadFromMemory_(
    FTextureSourceProperties* outProperties,
    RHI::EImageView imageView,
    const FRawMemoryConst& memory) {

    ETextureSourceFormat format;
    int2 dimensions;
    int components = 0;
    RHI::EColorMask colorMask = Default;
    ETextureSourceFlags flags = Default;
    PPE_LOG_CHECKEX(Texture, FTextureImporterResult{}, ::stbi_info_from_memory(memory.data(), checked_cast<int>(memory.size()),
        &dimensions.x, &dimensions.y, &components));

    switch (components) {
    case 1: colorMask = RHI::EColorMask::G; break;
    case 2: colorMask = RHI::EColorMask::RA; break;
    case 3: colorMask = RHI::EColorMask::RGB; break;
    case 4: colorMask = RHI::EColorMask::RGBA; break;
    default:
        PPE_LOG_CHECKEX(Texture, FTextureImporterResult{}, components >= 0 && components <= 4);
        break;
    }

    void* stbi_data;
    if (!!::stbi_is_hdr_from_memory(memory.data(), checked_cast<int>(memory.size()))) {
        components = 4; /* always RGBA32f */
        flags += ETextureSourceFlags::HDR;
        format = ETextureSourceFormat::RGBA32f;

        stbi_data = ::stbi_loadf_from_memory(memory.data(), checked_cast<int>(memory.size()),
            &dimensions.x, &dimensions.y, &components, components);
    }
    else
    if (!!::stbi_is_16_bit_from_memory(memory.data(), checked_cast<int>(memory.size()))) {
        format = ETextureSourceFormat::G16;
        if (components > 1) {
            components = 4;
            format = ETextureSourceFormat::RGBA16;
        }

        stbi_data = ::stbi_load_16_from_memory(memory.data(), checked_cast<int>(memory.size()), &dimensions.x, &dimensions.y, &components, components);
    }
    else {
        format = ETextureSourceFormat::G8;
        if (components > 1) {
            components = 4;
            format = ETextureSourceFormat::RGBA8;
        }

        stbi_data = ::stbi_load_from_memory(memory.data(), checked_cast<int>(memory.size()), &dimensions.x, &dimensions.y, &components, components);
    }

    if (Unlikely(nullptr == stbi_data)) {
        PPE_LOG(Texture, Error, "stb_image failed to import texure: {0}", MakeCStringView(::stbi_failure_reason()));
        return FTextureImporterResult{};
    }

    if (Unlikely(not STBImageInitSourceProperties_(outProperties, imageView, colorMask, checked_cast<u32>(dimensions), flags, format))) {
        PPE_LOG(Texture, Error, "stb_image has an incorrect image view: {0}", imageView);
        return FTextureImporterResult{};
    }

    return FUniqueBuffer::TakeOwn(stbi_data, outProperties->SizeInBytes(), &stbi_image_free);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::SupportsImageView(RHI::EImageView view) const NOEXCEPT {
    switch (view) {
    case RHI::EImageView::_1D:
    case RHI::EImageView::_1DArray:
    case RHI::EImageView::_2D:
    case RHI::EImageView::_2DArray:
    case RHI::EImageView::_3D:
    case RHI::EImageView::_Cube:
    case RHI::EImageView::_CubeArray:
        return true;

    case RHI::EImageView::Unknown:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::SupportsTextureSource(const FTextureSourceProperties& properties) const NOEXCEPT {
    if (SupportsImageView(properties.ImageView) &&
        SupportsTextureFormat(properties.Format)) {

        switch (properties.ImageView) {
        case RHI::EImageView::_1D:
            if (properties.Dimensions.yz != uint2::One)
                return false;
            break;

        case RHI::EImageView::_1DArray:
            if (properties.Dimensions.yz != uint2::One)
                return false;
            break;

        case RHI::EImageView::_2D:
            if (properties.Dimensions.z != 1)
                return false;
            break;

        case RHI::EImageView::_2DArray:
            if (properties.Dimensions.x != properties.Dimensions.y || properties.Dimensions.z != 1)
                return false;
            break;

        case RHI::EImageView::_3D:
            if (properties.Dimensions.x != properties.Dimensions.y)
                return false;
            break;

        case RHI::EImageView::_Cube:
            if (properties.Flags & ETextureSourceFlags::LongLatCubemap) {
                if (properties.Dimensions.z != 1)
                    return false;
            }
            else if (properties.NumSlices != 6 ||
                    properties.Dimensions.x != properties.Dimensions.y || properties.Dimensions.z != 1) {
                return false;
            }
            break;

        case RHI::EImageView::_CubeArray:
            if (properties.Flags & ETextureSourceFlags::LongLatCubemap) {
                if (properties.Dimensions.z != 1)
                    return false;
            }
            else if ((properties.NumSlices % 6) != 0 ||
                    properties.Dimensions.x != properties.Dimensions.y || properties.Dimensions.z != 1) {
                return false;
            }
            break;

        case RHI::EImageView::Unknown:
            AssertNotImplemented();
        }

        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
// Export to stream:
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::ExportTexture2DArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    PPE_LOG_CHECK(Texture, properties.NumMips == 1);
    PPE_LOG_CHECK(Texture, properties.Dimensions.x == properties.Dimensions.y);

    FTextureSourceProperties propertiesFlatten = properties;
    propertiesFlatten.ImageView = RHI::EImageView_2D;
    propertiesFlatten.Dimensions.y *= properties.NumSlices;
    propertiesFlatten.NumSlices = 1;

    return ExportTexture2D(output, propertiesFlatten, bulk);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::ExportTexture3D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    PPE_LOG_CHECK(Texture, properties.NumSlices == 1);
    PPE_LOG_CHECK(Texture, properties.Dimensions.x == properties.Dimensions.y);

    FTextureSourceProperties propertiesFlatten = properties;
    propertiesFlatten.ImageView = RHI::EImageView_2D;
    propertiesFlatten.Dimensions.y *= properties.Dimensions.z;
    propertiesFlatten.Dimensions.z = 1;

    return ExportTexture2D(output, propertiesFlatten, bulk);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::ExportTextureCube(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    PPE_LOG_CHECK(Texture, properties.NumSlices == 6);
    PPE_LOG_CHECK(Texture, properties.Dimensions.x == properties.Dimensions.y);

    FTextureSourceProperties propertiesFlatten = properties;
    propertiesFlatten.ImageView = RHI::EImageView_2D;
    propertiesFlatten.Dimensions.y *= properties.NumSlices;
    propertiesFlatten.NumSlices = 1;

    return ExportTexture2D(output, propertiesFlatten, bulk);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::ExportTextureCubeArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    PPE_LOG_CHECK(Texture, (properties.NumSlices % 6) == 0);
    PPE_LOG_CHECK(Texture, properties.Dimensions.x == properties.Dimensions.y);

    FTextureSourceProperties propertiesFlatten = properties;
    propertiesFlatten.ImageView = RHI::EImageView_2D;
    propertiesFlatten.Dimensions.y *= properties.NumSlices;
    propertiesFlatten.NumSlices = 1;

    return ExportTexture2D(output, propertiesFlatten, bulk);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
bool TSTBImageFormat<_ImageFormat>::ExportTextureCubeLongLat(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    PPE_LOG_CHECK(Texture, properties.NumSlices == 1);

    FTextureSourceProperties propertiesFlatten = properties;
    propertiesFlatten.ImageView = RHI::EImageView_2D;

    return ExportTexture2D(output, propertiesFlatten, bulk);
}
//----------------------------------------------------------------------------
// Import from memory:
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture2D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    // same code for every file format
    return STBImageLoadFromMemory_(outProperties, RHI::EImageView_2D, memory);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture2DArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    // same code for every file format
    return STBImageLoadFromMemory_(outProperties, RHI::EImageView_2DArray, memory);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture3D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    // same code for every file format
    return STBImageLoadFromMemory_(outProperties, RHI::EImageView_3D, memory);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCube(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    // same code for every file format
    return STBImageLoadFromMemory_(outProperties, RHI::EImageView_Cube, memory);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCubeArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    // same code for every file format
    return STBImageLoadFromMemory_(outProperties, RHI::EImageView_CubeArray, memory);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const {
    FTextureImporterResult result = STBImageLoadFromMemory_(outProperties, RHI::EImageView_2D, memory);
    if (result.has_value()) {
        outProperties->ImageView = RHI::EImageView_Cube;
        outProperties->Flags += ETextureSourceFlags::LongLatCubemap;
    }

    return result;
}
//----------------------------------------------------------------------------
// Import from stream callbacks:
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture2D(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    // same code for every file format
    return STBImageLoadFromStream_(outProperties, RHI::EImageView_2D, reader);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture2DArray(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    // same code for every file format
    return STBImageLoadFromStream_(outProperties, RHI::EImageView_2DArray, reader);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTexture3D(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    // same code for every file format
    return STBImageLoadFromStream_(outProperties, RHI::EImageView_3D, reader);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCube(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    // same code for every file format
    return STBImageLoadFromStream_(outProperties, RHI::EImageView_Cube, reader);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCubeArray(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    // same code for every file format
    return STBImageLoadFromStream_(outProperties, RHI::EImageView_CubeArray, reader);
}
//----------------------------------------------------------------------------
template <EImageFormat _ImageFormat>
FTextureImporterResult TSTBImageFormat<_ImageFormat>::ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, IStreamReader& reader) const {
    FTextureImporterResult result = STBImageLoadFromStream_(outProperties, RHI::EImageView_2D, reader);

    if (result.has_value()) {
        outProperties->ImageView = RHI::EImageView_Cube;
        outProperties->Flags += ETextureSourceFlags::LongLatCubemap;
    }

    return result;
}
//----------------------------------------------------------------------------
// PNG
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::PNG>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT {
    switch (fmt) {
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
    case ETextureSourceFormat::RGBA16:
        return true;

    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        break;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::PNG>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    AssertRelease(output);
    PPE_LOG_CHECK(Texture, SupportsTextureSource(properties));
    Assert_NoAssume(properties.SizeInBytes() == bulk.SizeInBytes());

    return (!!::stbi_write_png_to_func(&STBImageWriteFunc_, output,
        checked_cast<int>(properties.Width()),
        checked_cast<int>(properties.Height()),
        static_cast<int>(ETextureSourceFormat_Components(properties.Format)),
        bulk.data(),
        0/* pixels/rows are contiguous in memory */));
}
//----------------------------------------------------------------------------
// BMP
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::BMP>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT {
    switch (fmt) {
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
        return true;

    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RGBA16:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        break;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::BMP>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    AssertRelease(output);
    Assert_NoAssume(SupportsTextureSource(properties));
    Assert_NoAssume(properties.SizeInBytes() == bulk.SizeInBytes());

    return (!!::stbi_write_bmp_to_func(&STBImageWriteFunc_, output,
        checked_cast<int>(properties.Width()),
        checked_cast<int>(properties.Height()),
        static_cast<int>(ETextureSourceFormat_Components(properties.Format)),
        bulk.data()));
}
//----------------------------------------------------------------------------
// BMP
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::TGA>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT {
    switch (fmt) {
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
        return true;

    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RGBA16:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        break;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::TGA>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    AssertRelease(output);
    Assert_NoAssume(SupportsTextureSource(properties));
    Assert_NoAssume(properties.SizeInBytes() == bulk.SizeInBytes());

    return (!!::stbi_write_tga_to_func(&STBImageWriteFunc_, output,
        checked_cast<int>(properties.Width()),
        checked_cast<int>(properties.Height()),
        static_cast<int>(ETextureSourceFormat_Components(properties.Format)),
        bulk.data()));
}
//----------------------------------------------------------------------------
// JPG
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::JPG>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT {
    switch (fmt) {
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
        return true;

    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::RGBA16:
    case ETextureSourceFormat::RGBA16f:
    case ETextureSourceFormat::RGBA32f:
        break;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::JPG>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    AssertRelease(output);
    Assert_NoAssume(SupportsTextureSource(properties));
    Assert_NoAssume(properties.SizeInBytes() == bulk.SizeInBytes());

    constexpr int GSTBImageJPGQuality = 90; // `high` quality by default
    return (!!::stbi_write_jpg_to_func(&STBImageWriteFunc_, output,
        checked_cast<int>(properties.Width()),
        checked_cast<int>(properties.Height()),
        static_cast<int>(ETextureSourceFormat_Components(properties.Format)),
        bulk.data(),
        GSTBImageJPGQuality));
}
//----------------------------------------------------------------------------
// HDR
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::HDR>::SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT {
    switch (fmt) {
    case ETextureSourceFormat::RGBA32f:
        return true;

    case ETextureSourceFormat::G8:
    case ETextureSourceFormat::G16:
    case ETextureSourceFormat::R16f:
    case ETextureSourceFormat::RA8:
    case ETextureSourceFormat::RA16:
    case ETextureSourceFormat::RG8:
    case ETextureSourceFormat::RG16:
    case ETextureSourceFormat::RGBA8:
    case ETextureSourceFormat::BGRA8:
    case ETextureSourceFormat::BGRE8:
    case ETextureSourceFormat::RGBA16:
    case ETextureSourceFormat::RGBA16f:
        break;

    case ETextureSourceFormat::Unknown:
    case ETextureSourceFormat::_Last:
        AssertNotImplemented();
    }

    return false;
}
//----------------------------------------------------------------------------
template <>
bool TSTBImageFormat<EImageFormat::HDR>::ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const {
    AssertRelease(output);
    Assert_NoAssume(SupportsTextureSource(properties));
    Assert_NoAssume(properties.SizeInBytes() == bulk.SizeInBytes());

    return (!!::stbi_write_hdr_to_func(&STBImageWriteFunc_, output,
        checked_cast<int>(properties.Width()),
        checked_cast<int>(properties.Height()),
        static_cast<int>(ETextureSourceFormat_Components(properties.Format)),
        bulk.Cast<const float>().data()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class TSTBImageFormat<EImageFormat::PNG>;
template class TSTBImageFormat<EImageFormat::BMP>;
template class TSTBImageFormat<EImageFormat::TGA>;
template class TSTBImageFormat<EImageFormat::JPG>;
template class TSTBImageFormat<EImageFormat::HDR>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
