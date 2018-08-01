#include "stdafx.h"

#include "DeviceResourceBuffer.h"

#include "DeviceEncapsulator.h"
#include "DeviceResource.h"

#include "Container/Hash.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceResourceBuffer::FDeviceResourceBuffer(size_t stride, size_t count, EBufferMode mode, EBufferUsage usage)
:   _count(checked_cast<u32>(count)), _strideModeUsage(0) {
    Assert(stride);

#ifdef WITH_PPE_ASSERT_RELEASE
    switch (usage)
    {
    case EBufferUsage::Default:
        AssertRelease(  EBufferMode::None == mode ||
                        EBufferMode::Write == mode);
        break;
    case EBufferUsage::Dynamic:
        AssertRelease(  EBufferMode::Write == mode ||
                        EBufferMode::WriteDiscard == mode);
        break;
    case EBufferUsage::Immutable:
        AssertRelease(  EBufferMode::None == mode);
        break;
    case EBufferUsage::Staging:
        AssertRelease(  EBufferMode::None != mode);
        break;
    default:
        AssertNotImplemented();
        break;
    }
#endif

    bitusage_type::InplaceSet(_strideModeUsage, u32(usage));
    bitmode_type::InplaceSet(_strideModeUsage, u32(mode));
    bitstride_type::InplaceSet(_strideModeUsage, checked_cast<u32>(stride));
}
//----------------------------------------------------------------------------
FDeviceResourceBuffer::~FDeviceResourceBuffer() {
    Assert(!_deviceAPIDependantBuffer);
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::Resize(size_t count) {
    Assert(!_deviceAPIDependantBuffer);

    _count = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::Create(IDeviceAPIEncapsulator *device, const FDeviceResource * /* resource */, FDeviceAPIDependantResourceBuffer *buffer) {
    UNUSED(device);
    Assert(buffer);
    Assert(!_deviceAPIDependantBuffer);
    Assert(buffer->MatchDevice(device));

    _deviceAPIDependantBuffer = buffer;
}
//----------------------------------------------------------------------------
PDeviceAPIDependantResourceBuffer& FDeviceResourceBuffer::Destroy(IDeviceAPIEncapsulator *device, const FDeviceResource * /* resource */) {
    UNUSED(device);
    Assert(_deviceAPIDependantBuffer);
    Assert(_deviceAPIDependantBuffer->MatchDevice(device));

    return _deviceAPIDependantBuffer;
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    Assert(Meta::IsAligned(Stride(), dst.SizeInBytes()));
    Assert(dst.SizeInBytes() + offset <= _count * Stride());
    Assert(_deviceAPIDependantBuffer);
    //Assert(u32(EBufferMode::Read) == (u32(EMode()) & u32(EBufferMode::Read)));

    _deviceAPIDependantBuffer->GetData(device, offset, dst);
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    Assert(Meta::IsAligned(Stride(), src.SizeInBytes()));
    Assert(src.SizeInBytes() + offset <= _count * Stride());
    Assert(_deviceAPIDependantBuffer);
    //Assert(u32(EBufferMode::Write) == (u32(EMode()) & u32(EBufferMode::Write)));

    _deviceAPIDependantBuffer->SetData(device, offset, src);
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceResourceBuffer *psource) {
    Assert(psource);
    Assert(psource->Available());
    Assert(psource->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantBuffer->CopyFrom(device, psource->DeviceAPIDependantBuffer().get());
}
//----------------------------------------------------------------------------
void FDeviceResourceBuffer::CopySubPart(
    IDeviceAPIEncapsulator *device, size_t dstOffset,
    const FDeviceResourceBuffer *psource, size_t srcOffset,
    size_t length ) {
    Assert(0 < length);
    Assert(psource);
    Assert(psource->Available());
    Assert(psource->SizeInBytes() >= srcOffset + length);
    Assert(SizeInBytes() >= dstOffset + length);

    _deviceAPIDependantBuffer->CopySubPart(device, dstOffset, psource->DeviceAPIDependantBuffer().get(), srcOffset, length);
}
//----------------------------------------------------------------------------
size_t FDeviceResourceBuffer::HashValue() const {
    return hash_tuple(_count, _strideModeUsage);
}
//----------------------------------------------------------------------------
bool FDeviceResourceBuffer::Match(const FDeviceAPIDependantResourceBuffer& entity) const {
    return  entity.Count() == Count() &&
            entity.Stride() == Stride() &&
            entity.SizeInBytes() == SizeInBytes() &&
            entity.Mode() == Mode() &&
            entity.Usage() == Usage();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer::FDeviceAPIDependantResourceBuffer(
    IDeviceAPIEncapsulator *device,
    const FDeviceResource *resource,
    const FDeviceResourceBuffer *buffer,
    const TMemoryView<const u8>& /* optionalData */)
:   FDeviceAPIDependantEntity(device->APIEncapsulator(), resource)
,   _count(checked_cast<u32>(buffer->Count()))
,   _strideModeUsage(0)
,   _resourceType(resource->ResourceType()) {
    bitusage_type::InplaceSet(_strideModeUsage, u32(buffer->Usage()));
    bitmode_type::InplaceSet(_strideModeUsage, u32(buffer->Mode()));
    bitstride_type::InplaceSet(_strideModeUsage, checked_cast<u32>(buffer->Stride()));
}
//----------------------------------------------------------------------------
FDeviceAPIDependantResourceBuffer::~FDeviceAPIDependantResourceBuffer() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
