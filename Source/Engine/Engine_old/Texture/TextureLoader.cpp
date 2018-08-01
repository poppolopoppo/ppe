#include "stdafx.h"

#include "TextureLoader.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"

#include "Core.Graphics/Device/Texture/SurfaceFormat.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"
#include "Core.Graphics/Device/Texture/TextureCube.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const Graphics::FSurfaceFormat *FinalizeSurfaceFormat_(const Graphics::FSurfaceFormat *format, bool useSRGB) {
    if (useSRGB) {
        switch (format->Type())
        {
        case Graphics::ESurfaceFormatType::DXT1:
        case Graphics::ESurfaceFormatType::DXT1_SRGB:
            format = Graphics::FSurfaceFormat::DXT1_SRGB;
            break;
        case Graphics::ESurfaceFormatType::DXT3:
        case Graphics::ESurfaceFormatType::DXT3_SRGB:
            format = Graphics::FSurfaceFormat::DXT3_SRGB;
            break;
        case Graphics::ESurfaceFormatType::DXT5:
        case Graphics::ESurfaceFormatType::DXT5_SRGB:
            format = Graphics::FSurfaceFormat::DXT5_SRGB;
            break;
        case Graphics::ESurfaceFormatType::R8G8B8A8:
        case Graphics::ESurfaceFormatType::R8G8B8A8_SRGB:
            format = Graphics::FSurfaceFormat::R8G8B8A8_SRGB;
            break;
        case Graphics::ESurfaceFormatType::B8G8R8A8:
        case Graphics::ESurfaceFormatType::B8G8R8A8_SRGB:
            format = Graphics::FSurfaceFormat::B8G8R8A8_SRGB;
            break;
        default:
            AssertNotImplemented();
        }
    }
    return format;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasicTextureLoader::FBasicTextureLoader() {}
//----------------------------------------------------------------------------
FBasicTextureLoader::FBasicTextureLoader(const FTextureHeader& header)
:   _header(header) {}
//----------------------------------------------------------------------------
bool FBasicTextureLoader::ReadHeader(FTextureHeader *header, const FFilename& filename, IVirtualFileSystemIStream *stream) {
    Assert(header);
    Assert(!filename.empty());
    Assert(stream);

    AssertRelease(filename.Extname().Equals(L".dds")); // only format supported for now

    return DDS::ReadTextureHeader(*header, stream);
}
//----------------------------------------------------------------------------
bool FBasicTextureLoader::ReadPixels(const TMemoryView<u8>& pixels,  const FTextureHeader *header, const FFilename& filename, IVirtualFileSystemIStream *stream) {
    Assert(pixels.SizeInBytes() == header->SizeInBytes);
    Assert(!filename.empty());
    Assert(stream);

    AssertRelease(filename.Extname().Equals(L".dds")); // only format supported for now

    return DDS::ReadTextureData(*header, pixels, stream);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Graphics::FTexture2D *FTexture2DLoader::CreateTexture2D(
    Graphics::IDeviceAPIEncapsulator *device,
    const TMemoryView<const u8>& pixels,
    FString&& name,
    bool useSRGB ) const {
    Assert(_header.Format);
    Assert(pixels.SizeInBytes() == _header.SizeInBytes );

    AssertRelease(1 == _header.Depth);
    AssertRelease(1 == _header.ArraySize);

    const Graphics::FSurfaceFormat *format = FinalizeSurfaceFormat_(_header.Format, useSRGB);

    Graphics::FTexture2D *texture = new Graphics::FTexture2D(
        _header.Width,
        _header.Height,
        _header.LevelCount,
        format,
        Graphics::EBufferMode::None,
        Graphics::EBufferUsage::Default );

    texture->SetResourceName(std::move(name));
    texture->Freeze();
    texture->Create(device, pixels);

    return texture;
}
//----------------------------------------------------------------------------
Graphics::FTextureCube *FTexture2DLoader::CreateTextureCube(
    Graphics::IDeviceAPIEncapsulator *device,
    const TMemoryView<const u8>& pixels,
    FString&& name,
    bool useSRGB ) const {
    Assert(_header.Format);
    Assert(pixels.SizeInBytes() == _header.SizeInBytes );

    AssertRelease(1 == _header.Depth);
    AssertRelease(6 == _header.ArraySize);

    const Graphics::FSurfaceFormat *format = FinalizeSurfaceFormat_(_header.Format, useSRGB);

    Graphics::FTextureCube *texture = new Graphics::FTextureCube(
        _header.Width,
        _header.Height,
        _header.LevelCount,
        format,
        Graphics::EBufferMode::None,
        Graphics::EBufferUsage::Default );

    texture->SetResourceName(std::move(name));
    texture->Freeze();
    texture->Create(device, pixels);

    return texture;
}
//----------------------------------------------------------------------------
Graphics::FTexture *FTexture2DLoader::CreateTexture(
    Graphics::IDeviceAPIEncapsulator *device,
    const TMemoryView<const u8>& pixels,
    FString&& name,
    bool useSRGB ) const {

    if (_header.IsCubeMap) {
        Assert(6 == _header.ArraySize);
        return CreateTextureCube(device, pixels, std::move(name), useSRGB);
    }
    else {
        Assert(1 == _header.ArraySize);
        return CreateTexture2D(device, pixels, std::move(name), useSRGB);
    }
}
//----------------------------------------------------------------------------
Graphics::FTexture *FTexture2DLoader::Load(Graphics::IDeviceAPIEncapsulator *device, const FFilename& filename, bool useSRGB) {
    Assert(device);
    Assert(!filename.empty());

    RAWSTORAGE_ALIGNED(FTexture, u8, 16) pixels;

    FTexture2DLoader loader;
    if (!loader.Read(pixels, filename))
        return nullptr;

    return loader.CreateTexture(device, pixels.MakeConstView(), filename.ToString(), useSRGB);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
