#include "stdafx.h"

#include "DeviceResourceBuffer.h"

#include "DeviceEncapsulator.h"
#include "DeviceResource.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceResourceBuffer::DeviceResourceBuffer(size_t stride, size_t count, BufferMode mode, BufferUsage usage)
:   _count(checked_cast<u32>(count)), _strideModeUsage(0) {
    Assert(stride);

#ifdef WITH_CORE_ASSERT_RELEASE
    switch (usage)
    {
    case BufferUsage::Default:
        AssertRelease(  BufferMode::None == mode ||
                        BufferMode::Write == mode);
        break;
    case BufferUsage::Dynamic:
        AssertRelease(  BufferMode::Write == mode);
        break;
    case BufferUsage::Immutable:
        AssertRelease(  BufferMode::None == mode);
        break;
    case BufferUsage::Staging:
        AssertRelease(  BufferMode::None != mode);
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
DeviceResourceBuffer::~DeviceResourceBuffer() {
    Assert(!_deviceAPIDependantBuffer);
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::Resize(size_t count) {
    Assert(!_deviceAPIDependantBuffer);

    _count = checked_cast<u32>(count);
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::Create(IDeviceAPIEncapsulator *device, const DeviceResource * /* resource */, DeviceAPIDependantResourceBuffer *buffer) {
    Assert(buffer);
    Assert(!_deviceAPIDependantBuffer);
    Assert(buffer->MatchDevice(device));

    _deviceAPIDependantBuffer = buffer;
}
//----------------------------------------------------------------------------
PDeviceAPIDependantResourceBuffer DeviceResourceBuffer::Destroy(IDeviceAPIEncapsulator *device, const DeviceResource * /* resource */) {
    Assert(_deviceAPIDependantBuffer);
    Assert(_deviceAPIDependantBuffer->MatchDevice(device));

    PDeviceAPIDependantResourceBuffer result = std::move(_deviceAPIDependantBuffer);
    Assert(!_deviceAPIDependantBuffer);

    return result;
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    Assert(dst);
    Assert(stride == Stride());
    Assert(offset + count <= _count);
    Assert(_deviceAPIDependantBuffer);
    //Assert(u32(BufferMode::Read) == (u32(Mode()) & u32(BufferMode::Read)));

    _deviceAPIDependantBuffer->GetData(device, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    Assert(src);
    Assert(stride == Stride());
    Assert(offset + count <= _count);
    Assert(_deviceAPIDependantBuffer);
    //Assert(u32(BufferMode::Write) == (u32(Mode()) & u32(BufferMode::Write)));

    _deviceAPIDependantBuffer->SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceResourceBuffer *psource) {
    Assert(psource);
    Assert(psource->Available());
    Assert(psource->SizeInBytes() == SizeInBytes());

    _deviceAPIDependantBuffer->CopyFrom(device, psource->DeviceAPIDependantBuffer());
}
//----------------------------------------------------------------------------
void DeviceResourceBuffer::CopySubPart(
    IDeviceAPIEncapsulator *device, size_t dstOffset, 
    const DeviceResourceBuffer *psource, size_t srcOffset,
    size_t length ) {
    Assert(0 < length);
    Assert(psource);
    Assert(psource->Available());
    Assert(psource->SizeInBytes() >= srcOffset + length);
    Assert(SizeInBytes() >= dstOffset + length);

    _deviceAPIDependantBuffer->CopySubPart(device, dstOffset, psource->DeviceAPIDependantBuffer(), srcOffset, length);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer::DeviceAPIDependantResourceBuffer(
    IDeviceAPIEncapsulator *device,
    const DeviceResource *resource,
    const DeviceResourceBuffer *buffer,
    const MemoryView<const u8>& /* optionalData */)
:   DeviceAPIDependantEntity(device->APIEncapsulator(), resource)
,   _count(checked_cast<u32>(buffer->Count()))
,   _strideModeUsage(0)
,   _resourceType(resource->ResourceType()) {
    bitusage_type::InplaceSet(_strideModeUsage, u32(buffer->Usage()));
    bitmode_type::InplaceSet(_strideModeUsage, u32(buffer->Mode()));
    bitstride_type::InplaceSet(_strideModeUsage, checked_cast<u32>(buffer->Stride()));
}
//----------------------------------------------------------------------------
DeviceAPIDependantResourceBuffer::~DeviceAPIDependantResourceBuffer() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
