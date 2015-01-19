#include "stdafx.h"

#include "TextureCube.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

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
    size_t width,
    size_t height,
    size_t levelCount,
    const SurfaceFormat *format,
    BufferMode mode,
    BufferUsage usage
    )
:   Texture(format, mode, usage)
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
void TextureCube::GetData(
    IDeviceAPIEncapsulator *device,
    Face face,
    size_t level,
    size_t x, size_t y,
    size_t width, size_t height,
    void *const dst, size_t stride, size_t count ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTextureCube);

    // TODO
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void TextureCube::SetData(
    IDeviceAPIEncapsulator *device,
    Face face,
    size_t level,
    size_t x, size_t y,
    size_t width, size_t height,
    const void *src, size_t stride, size_t count ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTextureCube);

    // TODO
    AssertNotImplemented();
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
const Graphics::DeviceAPIDependantTexture *TextureCube::DeviceAPIDependantTexture() const {
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTextureCube::DeviceAPIDependantTextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture(device, owner) {}
//----------------------------------------------------------------------------
DeviceAPIDependantTextureCube::~DeviceAPIDependantTextureCube() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
