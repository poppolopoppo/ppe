// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IndexBuffer.h"

#include "Device/DeviceEncapsulator.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FIndexBuffer::FIndexBuffer(Graphics::EIndexElementSize indexElementSize, size_t indexCount, EBufferMode mode, EBufferUsage usage, bool sharable)
:   FDeviceResourceSharable(EDeviceResourceType::Indices, sharable)
,   _buffer(size_t(indexElementSize), indexCount, mode, usage) {
    Assert( sizeof(u16) == size_t(indexElementSize) ||
            sizeof(u32) == size_t(indexElementSize) );
    Assert(indexCount);
}
//----------------------------------------------------------------------------
FIndexBuffer::~FIndexBuffer() {}
//----------------------------------------------------------------------------
void FIndexBuffer::SetData_(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    _buffer.SetData(device, offset, src);
}
//----------------------------------------------------------------------------
size_t FIndexBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool FIndexBuffer::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const FDeviceAPIDependantResourceBuffer *resourceBuffer =
        checked_cast<const FDeviceAPIDependantResourceBuffer *>(entity);
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
void FIndexBuffer::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.SizeInBytes() == size_t(IndexElementSize()) * IndexCount());

    FDeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateIndexBuffer(this, &_buffer, optionalRawData);
    Assert(resourceBuffer);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void FIndexBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);

    device->DestroyIndexBuffer(this, _buffer.Destroy(device, this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
