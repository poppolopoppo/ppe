#include "stdafx.h"

#include "Texture2D.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Texture2D::Texture2D(
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
    Assert(0 < (width>>(levelCount-1)) );
    Assert(0 < (height>>(levelCount-1)) );
    Assert(0 == (width % format->BlockSize()));
    Assert(0 == (height % format->BlockSize()));
}
//----------------------------------------------------------------------------
Texture2D::Texture2D(
    size_t width,
    size_t height,
    size_t levelCount,
    const SurfaceFormat *format,
    BufferMode mode,
    BufferUsage usage,
    Graphics::DeviceAPIDependantTexture2D *deviceAPIDependantTexture2D)
:   Texture2D(width, height, levelCount, format, mode, usage) {
    Assert(deviceAPIDependantTexture2D);
    _deviceAPIDependantTexture2D = deviceAPIDependantTexture2D;
}
//----------------------------------------------------------------------------
Texture2D::~Texture2D() {
    Assert(Frozen());
    Assert(!_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
float4 Texture2D::DuDvDimensions() const {
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
void Texture2D::GetData(
    IDeviceAPIEncapsulator *device,
    size_t level,
    size_t x, size_t y,
    size_t width, size_t height,
    void *const dst, size_t stride, size_t count ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    // TODO
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void Texture2D::SetData(
    IDeviceAPIEncapsulator *device,
    size_t level,
    size_t x, size_t y,
    size_t width, size_t height,
    const void *src, size_t stride, size_t count ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    // TODO
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void Texture2D::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateTexture(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void Texture2D::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    device->DestroyTexture(this, _deviceAPIDependantTexture2D);

    Assert(!_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
const Graphics::DeviceAPIDependantTexture *Texture2D::DeviceAPIDependantTexture() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    return _deviceAPIDependantTexture2D.get();
}
//----------------------------------------------------------------------------
size_t Texture2D::SizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
void Texture2D::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(dst);
    Assert(stride);
    Assert(count);
    Assert(offset + stride * count <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(_deviceAPIDependantTexture2D);
    Assert(u32(BufferMode::Read) == (u32(Mode()) & u32(BufferMode::Read)));

    _deviceAPIDependantTexture2D->GetData(device, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void Texture2D::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(src);
    Assert(stride);
    Assert(count);
    Assert(offset + stride * count <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(_deviceAPIDependantTexture2D);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));

    _deviceAPIDependantTexture2D->SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D::DeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture(device, owner) {}
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D::~DeviceAPIDependantTexture2D() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
