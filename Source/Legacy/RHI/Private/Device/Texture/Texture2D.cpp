// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Texture2D.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

#include "Container/Hash.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTexture2D::FTexture2D(
    size_t width, size_t height, size_t levelCount,
    const FSurfaceFormat *format,
    EBufferMode mode, EBufferUsage usage,
    bool sharable )
:   FTexture(EDeviceResourceType::FTexture2D, format, mode, usage, sharable)
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
FTexture2D::FTexture2D(
    size_t width, size_t height, size_t levelCount,
    const FSurfaceFormat *format,
    EBufferMode mode, EBufferUsage usage,
    bool sharable,
    Graphics::FDeviceAPIDependantTexture2D *deviceAPIDependantTexture2D)
:   FTexture2D(width, height, levelCount, format, mode, usage, sharable) {
    Assert(deviceAPIDependantTexture2D);
    _deviceAPIDependantTexture2D = deviceAPIDependantTexture2D;
}
//----------------------------------------------------------------------------
FTexture2D::~FTexture2D() {
    Assert(Frozen());
    Assert(!_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
float4 FTexture2D::DuDvDimensions() const {
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
void FTexture2D::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(!_deviceAPIDependantTexture2D);

    _deviceAPIDependantTexture2D = device->CreateTexture2D(this, optionalRawData);

    Assert(_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
void FTexture2D::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTexture2D);

    device->DestroyTexture2D(this, _deviceAPIDependantTexture2D);

    Assert(!_deviceAPIDependantTexture2D);
}
//----------------------------------------------------------------------------
Graphics::FDeviceAPIDependantTexture *FTexture2D::TextureEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    return _deviceAPIDependantTexture2D.get();
}
//----------------------------------------------------------------------------
size_t FTexture2D::SizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
void FTexture2D::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(dst.SizeInBytes());
    Assert(offset + dst.SizeInBytes() <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(_deviceAPIDependantTexture2D);
    Assert(u32(EBufferMode::Read) == (u32(Mode()) & u32(EBufferMode::Read)));

    _deviceAPIDependantTexture2D->GetData(device, offset, dst);
}
//----------------------------------------------------------------------------
void FTexture2D::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(src.SizeInBytes());
    Assert(offset + src.SizeInBytes() <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount));
    Assert(_deviceAPIDependantTexture2D);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));

    _deviceAPIDependantTexture2D->SetData(device, offset, src);
}
//----------------------------------------------------------------------------
void FTexture2D::CopyFrom(IDeviceAPIEncapsulator *device, const FTexture *psource) {
    Assert(psource);
    CopyFrom(device, checked_cast<const FTexture2D *>(psource));
}
//----------------------------------------------------------------------------
void FTexture2D::CopyFrom(IDeviceAPIEncapsulator *device, const FTexture2D *psource2D) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psource2D);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));
    Assert(psource2D->Frozen());
    Assert(psource2D->Available());
    Assert(u32(EBufferMode::Write) == (u32(psource2D->Mode()) & u32(EBufferMode::Read)));
    Assert(psource2D->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantTexture2D->CopyFrom(device, psource2D->DeviceAPIDependantTexture2D().get());
}
//----------------------------------------------------------------------------
void FTexture2D::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const FTexture2D *psource2D, size_t srcLevel, const FAabb2u& srcBox ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psource2D);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));
    Assert(psource2D->Frozen());
    Assert(psource2D->Available());
    Assert(u32(EBufferMode::Write) == (u32(psource2D->Mode()) & u32(EBufferMode::Read)));
    Assert(dstLevel < _levelCount);
    Assert( dstPos.x() < _width &&
            dstPos.y() < _height );
    Assert(srcLevel < psource2D->_levelCount);
    Assert(srcBox.HasPositiveExtentsStrict());
    Assert( srcBox.Max().x() < psource2D->_width &&
            srcBox.Max().y() < psource2D->_height );
    Assert( psource2D->Format()->SizeOfTexture2DInBytes(srcBox.Extents()) ==
            Format()->SizeOfTexture2DInBytes(srcBox.Extents()) );

    _deviceAPIDependantTexture2D->CopySubPart(device, dstLevel, dstPos, psource2D->DeviceAPIDependantTexture2D().get(), srcLevel, srcBox);
}
//----------------------------------------------------------------------------
size_t FTexture2D::VirtualSharedKeyHashValue() const {
    return hash_tuple(FTexture::HashValue_(), _width, _height, _levelCount);
}
//----------------------------------------------------------------------------
bool FTexture2D::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const Graphics::FDeviceAPIDependantTexture2D *texture2D =
        checked_cast<const Graphics::FDeviceAPIDependantTexture2D *>(entity);
    return  FTexture::Match_(*texture2D) &&
            texture2D->Width() == _width &&
            texture2D->Height() == _height &&
            texture2D->LevelCount() == _levelCount;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantTexture2D::FDeviceAPIDependantTexture2D(IDeviceAPIEncapsulator *device, const FTexture2D *resource, const TMemoryView<const u8>& /* optionalData */)
:   FDeviceAPIDependantTexture(device, resource)
,   _width(checked_cast<u32>(resource->Width()))
,   _height(checked_cast<u32>(resource->Height()))
,   _levelCount(checked_cast<u32>(resource->LevelCount())) {}
//----------------------------------------------------------------------------
FDeviceAPIDependantTexture2D::~FDeviceAPIDependantTexture2D() {}
//----------------------------------------------------------------------------
size_t FDeviceAPIDependantTexture2D::VideoMemorySizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
