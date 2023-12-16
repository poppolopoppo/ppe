// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureCube.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceResourceBuffer.h"
#include "SurfaceFormat.h"

#include "Container/Hash.h"

namespace PPE {
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
FTextureCube::FTextureCube(
    size_t width, size_t height, size_t levelCount,
    const FSurfaceFormat *format,
    EBufferMode mode, EBufferUsage usage,
    bool sharable )
:   FTexture(EDeviceResourceType::FTextureCube, format, mode, usage, sharable)
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
FTextureCube::~FTextureCube() {
    Assert(Frozen());
    Assert(!_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
float4 FTextureCube::DuDvDimensions() const {
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
void FTextureCube::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(!_deviceAPIDependantTextureCube);

    _deviceAPIDependantTextureCube = device->CreateTextureCube(this, optionalRawData);

    Assert(_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
void FTextureCube::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantTextureCube);

    device->DestroyTextureCube(this, _deviceAPIDependantTextureCube);

    Assert(!_deviceAPIDependantTextureCube);
}
//----------------------------------------------------------------------------
Graphics::FDeviceAPIDependantTexture *FTextureCube::TextureEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    return _deviceAPIDependantTextureCube.get();
}
//----------------------------------------------------------------------------
size_t FTextureCube::SizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount;
}
//----------------------------------------------------------------------------
void FTextureCube::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(dst.SizeInBytes());
    Assert(offset + dst.SizeInBytes() <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(_deviceAPIDependantTextureCube);
    Assert(u32(EBufferMode::Read) == (u32(Mode()) & u32(EBufferMode::Read)));

    _deviceAPIDependantTextureCube->GetData(device, offset, dst);
}
//----------------------------------------------------------------------------
void FTextureCube::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(src.SizeInBytes());
    Assert(offset + src.SizeInBytes() <= Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount) * CubeMapFaceCount);
    Assert(_deviceAPIDependantTextureCube);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));

    _deviceAPIDependantTextureCube->SetData(device, offset, src);
}
//----------------------------------------------------------------------------
void FTextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const FTexture *psource) {
    Assert(psource);
    CopyFrom(device, checked_cast<const FTextureCube *>(psource));
}
//----------------------------------------------------------------------------
void FTextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const FTextureCube *psourceCube) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psourceCube);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));
    Assert(psourceCube->Frozen());
    Assert(psourceCube->Available());
    Assert(u32(EBufferMode::Write) == (u32(psourceCube->Mode()) & u32(EBufferMode::Read)));
    Assert(psourceCube->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantTextureCube->CopyFrom(device, psourceCube->DeviceAPIDependantTextureCube().get());
}
//----------------------------------------------------------------------------
void FTextureCube::CopySubPart(
    IDeviceAPIEncapsulator *device,
    EFace dstFace, size_t dstLevel, const uint2& dstPos,
    const FTextureCube *psourceCube, EFace srcFace, size_t srcLevel, const FAabb2u& srcBox ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(Available());
    Assert(psourceCube);
    Assert(u32(EBufferMode::Write) == (u32(Mode()) & u32(EBufferMode::Write)));
    Assert(psourceCube->Frozen());
    Assert(psourceCube->Available());
    Assert(u32(EBufferMode::Write) == (u32(psourceCube->Mode()) & u32(EBufferMode::Read)));
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
size_t FTextureCube::VirtualSharedKeyHashValue() const {
    return hash_tuple(FTexture::HashValue_(), _width, _height, _levelCount);
}
//----------------------------------------------------------------------------
bool FTextureCube::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const Graphics::FDeviceAPIDependantTextureCube *textureCube =
        checked_cast<const Graphics::FDeviceAPIDependantTextureCube *>(entity);
    return  FTexture::Match_(*textureCube) &&
            textureCube->Width() == _width &&
            textureCube->Height() == _height &&
            textureCube->LevelCount() == _levelCount;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantTextureCube::FDeviceAPIDependantTextureCube(IDeviceAPIEncapsulator *device, const FTextureCube *resource, const TMemoryView<const u8>& /* optionalData */)
:   FDeviceAPIDependantTexture(device, resource)
,   _width(checked_cast<u32>(resource->Width()))
,   _height(checked_cast<u32>(resource->Height()))
,   _levelCount(checked_cast<u32>(resource->LevelCount())) {}
//----------------------------------------------------------------------------
FDeviceAPIDependantTextureCube::~FDeviceAPIDependantTextureCube() {}
//----------------------------------------------------------------------------
size_t FDeviceAPIDependantTextureCube::VideoMemorySizeInBytes() const {
    return Format()->SizeOfTexture2DInBytes(_width, _height, _levelCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
