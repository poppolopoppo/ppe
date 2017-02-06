#include "stdafx.h"

#include "IndexBuffer.h"

#include "Device/DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IndexBuffer::IndexBuffer(Graphics::EIndexElementSize indexElementSize, size_t indexCount, EBufferMode mode, EBufferUsage usage, bool sharable)
:   FDeviceResourceSharable(EDeviceResourceType::Indices, sharable)
,   _buffer(size_t(indexElementSize), indexCount, mode, usage) {
    Assert( sizeof(u16) == size_t(indexElementSize) ||
            sizeof(u32) == size_t(indexElementSize) );
    Assert(indexCount);
}
//----------------------------------------------------------------------------
IndexBuffer::~IndexBuffer() {}
//----------------------------------------------------------------------------
void IndexBuffer::SetData_(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    _buffer.SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
size_t IndexBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool IndexBuffer::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const FDeviceAPIDependantResourceBuffer *resourceBuffer =
        checked_cast<const FDeviceAPIDependantResourceBuffer *>(entity);
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
void IndexBuffer::Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.SizeInBytes() == size_t(IndexElementSize()) * IndexCount());

    FDeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateIndexBuffer(this, &_buffer, optionalRawData);
    Assert(resourceBuffer);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void IndexBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);

    PDeviceAPIDependantResourceBuffer resourceBuffer = _buffer.Destroy(device, this);
    Assert(resourceBuffer);

    device->DestroyIndexBuffer(this, resourceBuffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
