#include "stdafx.h"

#include "TextureCube.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

#include "Core/Container/Hash.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, CubeMapFaceCount, 6);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextureCube::TextureCube(
    size_t width, size_t height, size_t levelCount,
    const SurfaceFormat *format,
    BufferMode mode, BufferUsage usage,
    bool sharable )
:   Texture(DeviceResourceType::TextureCube, format, mode, usage, sharable)
,   _width(checked_cast<u32>(width))
,   _height(checked_cast<u32>(height))
,   _levelCount(checked_cast<u32>(levelCount)) {
    Assert(width);
    Assert(height);
    Assert(levelCount);
    Assert(0 < (std::max(width, height)>>(levelCount-1)) );
    Assert(0 == (width % format->BlockSize()));
    Assert(0 == (height % format->BlockSize()));
}
//----------------------------------------------------------------------------
TextureCube::~TextureCube() {
    Assert(Frozen());
    Assert(!_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
float4 TextureCube::DuDvDimensions() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_width && _height);

    return float4(
        1.0f / _width,
        1.0f / _height,
        static_cast<float>(_width),
        static_cast<float>(_height)
        );
}
//----------------------------------------------------------------------------
void TextureCube::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(!_deviceAPIDependantTextureCube);

    _deviceAPIDependantTextureCube = device->CreateTextureCube(this, optionalRawData);

    Assert(_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
void TextureCube::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTextureCube);

    device->DestroyTextureCube(this, _deviceAPIDependantTextureCube);

    Assert(!_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
Graphics::DeviceAPIDependantTexture *TextureCube::TextureEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    return _deviceAPIDependantTextureCube.get();
}
//----------------------------------------------------------------------------
size_t TextureCube::SizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount;
}
//----------------------------------------------------------------------------
void TextureCube::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(dst);
    Assert(stride);
    Assert(count);
    Assert(offset + stride * count <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(_deviceAPIDependantTextureCube);
    Assert(u32(BufferMode::Read) == (u32(Mode()) & u32(BufferMode::Read)));

    _deviceAPIDependantTextureCube->GetData(device, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void TextureCube::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(src);
    Assert(stride);
    Assert(count);
    Assert(offset + stride * count <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(_deviceAPIDependantTextureCube);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));

    _deviceAPIDependantTextureCube->SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void TextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const Texture *psource) {
    Assert(psource);
    CopyFrom(device, checked_cast<const TextureCube *>(psource));
}
//----------------------------------------------------------------------------
void TextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const TextureCube *psourceCube) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psourceCube);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));
    Assert(psourceCube->Frozen());
    Assert(psourceCube->Available());
    Assert(u32(BufferMode::Write) == (u32(psourceCube->Mode()) & u32(BufferMode::Read)));
    Assert(psourceCube->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantTextureCube->CopyFrom(device, psourceCube->DeviceAPIDependantTextureCube().get());
}
//----------------------------------------------------------------------------
void TextureCube::CopySubPart(
    IDeviceAPIEncapsulator *device,
    Face dstFace, size_t dstLevel, const uint2& dstPos,
    const TextureCube *psourceCube, Face srcFace, size_t srcLevel, const AABB2u& srcBox ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psourceCube);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));
    Assert(psourceCube->Frozen());
    Assert(psourceCube->Available());
    Assert(u32(BufferMode::Write) == (u32(psourceCube->Mode()) & u32(BufferMode::Read)));
    Assert(dstLevel < _levelCount);
    Assert( dstPos.x() < _width &&
            dstPos.y() < _height );
    Assert(srcLevel < psourceCube->_levelCount);
    Assert(srcBox.HasPositiveExtentsStrict());
    Assert( srcBox.Max().x() < psourceCube->_width &&
            srcBox.Max().y() < psourceCube->_height );
    Assert( psourceCube->Format()->SizeOfTexture2DInBytes(srcBox.Extents()) ==
            Format()->SizeOfTexture2DInBytes(srcBox.Extents()) );

    _deviceAPIDependantTextureCube->CopySubPart(
        device,
        dstFace, dstLevel, dstPos,
        psourceCube->DeviceAPIDependantTextureCube().get(), srcFace, srcLevel, srcBox );
}
//----------------------------------------------------------------------------
size_t TextureCube::VirtualSharedKeyHashValue() const {
    return hash_tuple(Texture::HashValue_(), _width, _height, _levelCount);
}
//----------------------------------------------------------------------------
bool TextureCube::VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
    const Graphics::DeviceAPIDependantTextureCube *textureCube =
        checked_cast<const Graphics::DeviceAPIDependantTextureCube *>(entity);
    return  Texture::Match_(*textureCube) &&
            textureCube->Width() == _width &&
            textureCube->Height() == _height &&
            textureCube->LevelCount() == _levelCount;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTextureCube::DeviceAPIDependantTextureCube(IDeviceAPIEncapsulator *device, const TextureCube *resource, const MemoryView<const u8>& /* optionalData */)
:   DeviceAPIDependantTexture(device, resource)
,   _width(checked_cast<u32>(resource->Width()))
,   _height(checked_cast<u32>(resource->Height()))
,   _levelCount(checked_cast<u32>(resource->LevelCount())) {}
//----------------------------------------------------------------------------
DeviceAPIDependantTextureCube::~DeviceAPIDependantTextureCube() {}
//----------------------------------------------------------------------------
size_t DeviceAPIDependantTextureCube::VideoMemorySizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
