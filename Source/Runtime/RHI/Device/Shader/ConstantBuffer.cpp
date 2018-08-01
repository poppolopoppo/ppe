#include "stdafx.h"

#include "ConstantBuffer.h"

#include "ConstantBufferLayout.h"
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
FConstantBuffer::FConstantBuffer(const FConstantBufferLayout *layout, bool sharable)
:   FDeviceResourceSharable(EDeviceResourceType::Constants, sharable)
,   _buffer(ConstantBufferStride, layout->SizeInBytes(), EBufferMode::WriteDiscard, EBufferUsage::Dynamic)
,   _layout(layout) {
    Assert(_layout);
    Assert(0 == (_layout->SizeInBytes() % ConstantBufferStride));
}
//----------------------------------------------------------------------------
FConstantBuffer::~FConstantBuffer() {
    Assert(!_deviceAPIDependantWriter);
}
//----------------------------------------------------------------------------
void FConstantBuffer::SetData(IDeviceAPIEncapsulator *device, const TMemoryView<const void *>& fieldsData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    STACKLOCAL_POD_ARRAY(u8, deviceAPIDependantRawData, _layout->SizeInBytes());
    Assert(Meta::IsAligned(GRAPHICS_BOUNDARY, deviceAPIDependantRawData.Pointer()));

    _deviceAPIDependantWriter->SetData(device, this, fieldsData, deviceAPIDependantRawData);
    _buffer.SetData(device, 0, deviceAPIDependantRawData);
}
//----------------------------------------------------------------------------
void FConstantBuffer::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(!_deviceAPIDependantWriter);

    FDeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateConstantBuffer(this, &_buffer);
    Assert(resourceBuffer);

    _deviceAPIDependantWriter = device->ConstantWriter();
    Assert(_deviceAPIDependantWriter);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void FConstantBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantWriter);

    device->DestroyConstantBuffer(this, _buffer.Destroy(device, this));

    _deviceAPIDependantWriter = nullptr;
}
//----------------------------------------------------------------------------
size_t FConstantBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool FConstantBuffer::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const FDeviceAPIDependantResourceBuffer *resourceBuffer =
        checked_cast<const FDeviceAPIDependantResourceBuffer *>(entity);
    // no restriction on _layout or _deviceAPIDependantWriter :
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantConstantWriter::FDeviceAPIDependantConstantWriter(IDeviceAPIEncapsulator *device)
:   FDeviceAPIDependantEntity(device->APIEncapsulator(), EDeviceResourceType::Constants) {}
//----------------------------------------------------------------------------
FDeviceAPIDependantConstantWriter::~FDeviceAPIDependantConstantWriter() {}
//----------------------------------------------------------------------------
size_t FDeviceAPIDependantConstantWriter::VideoMemorySizeInBytes() const {
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
