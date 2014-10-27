#include "stdafx.h"

#include "IndexBuffer.h"

#include "Device/DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IndexBuffer::IndexBuffer(Graphics::IndexElementSize indexElementSize, size_t indexCount, BufferMode mode, BufferUsage usage)
:   _buffer(size_t(indexElementSize), indexCount, mode, usage) {
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
void IndexBuffer::Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.SizeInBytes() == size_t(IndexElementSize()) * IndexCount());

    DeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateIndexBuffer(this, &_buffer, optionalRawData);
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
