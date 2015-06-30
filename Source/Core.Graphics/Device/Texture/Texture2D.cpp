#include "stdafx.h"

#include "Texture2D.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"

#include "Device/DeviceAPI.h"
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
:   Texture(DeviceResourceType::Texture2D, format, mode, usage)
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
void Texture2D::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateTexture2D(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void Texture2D::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    device->DestroyTexture2D(this, _deviceAPIDependantTexture2D);

    Assert(!_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
Graphics::DeviceAPIDependantTexture *Texture2D::TextureEntity() const {
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
    Assert(Available());
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
    Assert(Available());
    Assert(src);
    Assert(stride);
    Assert(count);
    Assert(offset + stride * count <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(_deviceAPIDependantTexture2D);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));

    _deviceAPIDependantTexture2D->SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void Texture2D::CopyFrom(IDeviceAPIEncapsulator *device, const Texture *psource) {
    Assert(psource);
    CopyFrom(device, checked_cast<const Texture2D *>(psource));
}
//----------------------------------------------------------------------------
void Texture2D::CopyFrom(IDeviceAPIEncapsulator *device, const Texture2D *psource2D) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psource2D);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));
    Assert(psource2D->Frozen());
    Assert(psource2D->Available());
    Assert(u32(BufferMode::Write) == (u32(psource2D->Mode()) & u32(BufferMode::Read)));
    Assert(psource2D->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantTexture2D->CopyFrom(device, psource2D->DeviceAPIDependantTexture2D());
}
//----------------------------------------------------------------------------
void Texture2D::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const Texture2D *psource2D, size_t srcLevel, const AABB2u& srcBox ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psource2D);
    Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));
    Assert(psource2D->Frozen());
    Assert(psource2D->Available());
    Assert(u32(BufferMode::Write) == (u32(psource2D->Mode()) & u32(BufferMode::Read)));
    Assert(dstLevel < _levelCount);
    Assert( dstPos.x() < _width && 
            dstPos.y() < _height );
    Assert(srcLevel < psource2D->_levelCount);
    Assert(srcBox.HasPositiveExtentsStrict());
    Assert( srcBox.Max().x() < psource2D->_width && 
            srcBox.Max().y() < psource2D->_height );
    Assert( psource2D->Format()->SizeOfTexture2DInBytes(srcBox.Extents()) == 
            Format()->SizeOfTexture2DInBytes(srcBox.Extents()) );

    _deviceAPIDependantTexture2D->CopySubPart(device, dstLevel, dstPos, psource2D->DeviceAPIDependantTexture2D(), srcLevel, srcBox);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D::DeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, const Texture2D *resource, const MemoryView<const u8>& /* optionalData */)
:   DeviceAPIDependantTexture(device, resource)
,   _width(checked_cast<u32>(resource->Width()))
,   _height(checked_cast<u32>(resource->Height())) 
,   _levelCount(checked_cast<u32>(resource->LevelCount())) {}
//----------------------------------------------------------------------------
DeviceAPIDependantTexture2D::~DeviceAPIDependantTexture2D() {}
//----------------------------------------------------------------------------
size_t DeviceAPIDependantTexture2D::VideoMemorySizeInBytes() const { 
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
