#include "stdafx.h"

#include "TextureLoader.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
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
static const Graphics::SurfaceFormat *FinalizeSurfaceFormat_(const Graphics::SurfaceFormat *format, bool useSRGB) {
    if (useSRGB) {
        switch (format->Type())
        {
        case Graphics::SurfaceFormatType::DXT1:
        case Graphics::SurfaceFormatType::DXT1_SRGB:
            format = Graphics::SurfaceFormat::DXT1_SRGB;
            break;
        case Graphics::SurfaceFormatType::DXT3:
        case Graphics::SurfaceFormatType::DXT3_SRGB:
            format = Graphics::SurfaceFormat::DXT3_SRGB;
            break;
        case Graphics::SurfaceFormatType::DXT5:
        case Graphics::SurfaceFormatType::DXT5_SRGB:
            format = Graphics::SurfaceFormat::DXT5_SRGB;
            break;
        case Graphics::SurfaceFormatType::R8G8B8A8:
        case Graphics::SurfaceFormatType::R8G8B8A8_SRGB:
            format = Graphics::SurfaceFormat::R8G8B8A8_SRGB;
            break;
        case Graphics::SurfaceFormatType::B8G8R8A8:
        case Graphics::SurfaceFormatType::B8G8R8A8_SRGB:
            format = Graphics::SurfaceFormat::B8G8R8A8_SRGB;
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
BasicTextureLoader::BasicTextureLoader() {}
//----------------------------------------------------------------------------
BasicTextureLoader::BasicTextureLoader(const TextureHeader& header)
:   _header(header) {}
//----------------------------------------------------------------------------
bool BasicTextureLoader::ReadHeader(TextureHeader *header, const Filename& filename, IVirtualFileSystemIStream *stream) {
    Assert(header);
    Assert(!filename.empty());
    Assert(stream);

    AssertRelease(filename.Extname().Equals(L".dds")); // only format supported for now

    return DDS::ReadTextureHeader(*header, stream);
}
//----------------------------------------------------------------------------
bool BasicTextureLoader::ReadPixels(const MemoryView<u8>& pixels,  const TextureHeader *header, const Filename& filename, IVirtualFileSystemIStream *stream) {
    Assert(pixels.SizeInBytes() == header->SizeInBytes);
    Assert(!filename.empty());
    Assert(stream);

    AssertRelease(filename.Extname().Equals(L".dds")); // only format supported for now

    return DDS::ReadTextureData(*header, pixels, stream);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Graphics::Texture2D *Texture2DLoader::CreateTexture2D(
    Graphics::IDeviceAPIEncapsulator *device,
    const MemoryView<const u8>& pixels,
    String&& name,
    bool useSRGB ) const {
    Assert(_header.Format);
    Assert(pixels.SizeInBytes() == _header.SizeInBytes );

    AssertRelease(1 == _header.Depth);
    AssertRelease(1 == _header.ArraySize);

    const Graphics::SurfaceFormat *format = FinalizeSurfaceFormat_(_header.Format, useSRGB);

    Graphics::Texture2D *texture = new Graphics::Texture2D(
        _header.Width,
        _header.Height,
        _header.LevelCount,
        format,
        Graphics::BufferMode::None,
        Graphics::BufferUsage::Default );

    texture->SetResourceName(std::move(name));
    texture->Freeze();
    texture->Create(device, pixels);

    return texture;
}
//----------------------------------------------------------------------------
Graphics::TextureCube *Texture2DLoader::CreateTextureCube(
    Graphics::IDeviceAPIEncapsulator *device,
    const MemoryView<const u8>& pixels,
    String&& name,
    bool useSRGB ) const {
    Assert(_header.Format);
    Assert(pixels.SizeInBytes() == _header.SizeInBytes );

    AssertRelease(1 == _header.Depth);
    AssertRelease(6 == _header.ArraySize);

    const Graphics::SurfaceFormat *format = FinalizeSurfaceFormat_(_header.Format, useSRGB);

    Graphics::TextureCube *texture = new Graphics::TextureCube(
        _header.Width,
        _header.Height,
        _header.LevelCount,
        format,
        Graphics::BufferMode::None,
        Graphics::BufferUsage::Default );

    texture->SetResourceName(std::move(name));
    texture->Freeze();
    texture->Create(device, pixels);

    return texture;
}
//----------------------------------------------------------------------------
Graphics::Texture *Texture2DLoader::CreateTexture(
    Graphics::IDeviceAPIEncapsulator *device,
    const MemoryView<const u8>& pixels,
    String&& name,
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
Graphics::Texture *Texture2DLoader::Load(Graphics::IDeviceAPIEncapsulator *device, const Filename& filename, bool useSRGB) {
    Assert(device);
    Assert(!filename.empty());

    RAWSTORAGE_ALIGNED(Texture, u8, 16) pixels;

    Texture2DLoader loader;
    if (!loader.Read(pixels, filename))
        return nullptr;

    return loader.CreateTexture(device, pixels.MakeConstView(), filename.ToString(), useSRGB);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
