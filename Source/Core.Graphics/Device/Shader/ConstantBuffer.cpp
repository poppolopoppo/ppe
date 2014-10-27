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
ConstantBuffer::ConstantBuffer(const ConstantBufferLayout *layout)
:   _buffer(1, layout->SizeInBytes(), BufferMode::None, BufferUsage::Default)
,   _layout(layout) {
    Assert(_layout);
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

    _deviceAPIDependantWriter->SetData(device, this, rawData, deviceAPIDependantRawData);
    _buffer.SetData(device, 0, deviceAPIDependantRawData.Pointer(), deviceAPIDependantRawData.size());
}
//----------------------------------------------------------------------------
void ConstantBuffer::SetData(IDeviceAPIEncapsulator *device, const MemoryView<const void *>& fieldsData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    const auto deviceAPIDependantRawData = MALLOCA_VIEW(u8, _layout->SizeInBytes());

    _deviceAPIDependantWriter->SetData(device, this, fieldsData, deviceAPIDependantRawData);
    _buffer.SetData(device, 0, deviceAPIDependantRawData.Pointer(), deviceAPIDependantRawData.size());
}
//----------------------------------------------------------------------------
void ConstantBuffer::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(!_deviceAPIDependantWriter);

    DeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateConstantBuffer(this, &_buffer, _deviceAPIDependantWriter);
    Assert(resourceBuffer);
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

    device->DestroyConstantBuffer(this, resourceBuffer, _deviceAPIDependantWriter);

    Assert(!_deviceAPIDependantWriter);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantConstantWriter::DeviceAPIDependantConstantWriter(IDeviceAPIEncapsulator *device)
:   DeviceAPIDependantEntity(device) {}
//----------------------------------------------------------------------------
DeviceAPIDependantConstantWriter::~DeviceAPIDependantConstantWriter() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
