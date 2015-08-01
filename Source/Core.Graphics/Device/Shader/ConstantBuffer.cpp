#include "stdafx.h"

#include "ConstantBuffer.h"

#include "ConstantBufferLayout.h"
#include "ConstantField.h"
#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, ConstantBufferStride, 4);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer(const ConstantBufferLayout *layout, bool sharable)
:   DeviceResourceSharable(DeviceResourceType::Constants, sharable)
,   _buffer(ConstantBufferStride, layout->SizeInBytes(), BufferMode::WriteDiscard, BufferUsage::Dynamic)
,   _layout(layout) {
    Assert(_layout);
    Assert(0 == (_layout->SizeInBytes() % ConstantBufferStride));
}
//----------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer() {
    Assert(!_deviceAPIDependantWriter);
}
//----------------------------------------------------------------------------
void ConstantBuffer::SetData(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& rawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    const auto deviceAPIDependantRawData = MALLOCA_VIEW(u8, _layout->SizeInBytes());
    Assert(IS_ALIGNED(16, deviceAPIDependantRawData.Pointer()));

    _deviceAPIDependantWriter->SetData(device, this, rawData, deviceAPIDependantRawData);
    _buffer.SetData(device, 0, deviceAPIDependantRawData.Pointer(), deviceAPIDependantRawData.size());
}
//----------------------------------------------------------------------------
void ConstantBuffer::SetData(IDeviceAPIEncapsulator *device, const MemoryView<const void *>& fieldsData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    const auto deviceAPIDependantRawData = MALLOCA_VIEW(u8, _layout->SizeInBytes());
    Assert(IS_ALIGNED(16, deviceAPIDependantRawData.Pointer()));

    _deviceAPIDependantWriter->SetData(device, this, fieldsData, deviceAPIDependantRawData);
    _buffer.SetData(device, 0, deviceAPIDependantRawData.Pointer(), deviceAPIDependantRawData.size());
}
//----------------------------------------------------------------------------
void ConstantBuffer::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(!_deviceAPIDependantWriter);

    DeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateConstantBuffer(this, &_buffer);
    Assert(resourceBuffer);
    Assert(_deviceAPIDependantWriter);

    _deviceAPIDependantWriter = device->ConstantWriter();
    Assert(_deviceAPIDependantWriter);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void ConstantBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantWriter);

    PDeviceAPIDependantResourceBuffer resourceBuffer = _buffer.Destroy(device, this);
    Assert(resourceBuffer);

    device->DestroyConstantBuffer(this, resourceBuffer);

    Assert(!_deviceAPIDependantWriter);
}
//----------------------------------------------------------------------------
size_t ConstantBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool ConstantBuffer::VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
    const DeviceAPIDependantResourceBuffer *resourceBuffer = 
        checked_cast<const DeviceAPIDependantResourceBuffer *>(entity);
    // no restriction on _layout or _deviceAPIDependantWriter :
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantConstantWriter::DeviceAPIDependantConstantWriter(IDeviceAPIEncapsulator *device)
:   DeviceAPIDependantEntity(device->APIEncapsulator(), nullptr) {}
//----------------------------------------------------------------------------
DeviceAPIDependantConstantWriter::~DeviceAPIDependantConstantWriter() {}
//----------------------------------------------------------------------------
size_t DeviceAPIDependantConstantWriter::VideoMemorySizeInBytes() const {
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
