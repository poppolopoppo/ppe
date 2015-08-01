#include "stdafx.h"

#include "VertexBuffer.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexDeclaration.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexBufferBinding::Set(  size_t instanceFrequency,
                                size_t vertexOffset,
                                const Graphics::VertexBuffer *vertexBuffer) {
    Assert(vertexBuffer);
    Assert(vertexBuffer->VertexCount() > vertexOffset);

    InstanceFrequency = instanceFrequency;
    VertexOffset = vertexOffset;
    VertexBuffer = vertexBuffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VertexBuffer::VertexBuffer(const Graphics::VertexDeclaration *vertexDeclaration, size_t vertexCount, BufferMode mode, BufferUsage usage, bool sharable)
:   DeviceResourceSharable(DeviceResourceType::Vertices, sharable)
,   _vertexDeclaration(vertexDeclaration)
,   _buffer(vertexDeclaration->SizeInBytes(), vertexCount, mode, usage) {
    Assert(vertexDeclaration);
    Assert(vertexCount);
}
//----------------------------------------------------------------------------
VertexBuffer::~VertexBuffer() {}
//----------------------------------------------------------------------------
void VertexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    _buffer.SetData(device, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void VertexBuffer::Create(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == _vertexDeclaration->SizeInBytes() * VertexCount());

    DeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateVertexBuffer(this, &_buffer, optionalRawData);
    Assert(resourceBuffer);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void VertexBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);

    PDeviceAPIDependantResourceBuffer resourceBuffer = _buffer.Destroy(device, this);
    Assert(resourceBuffer);

    device->DestroyVertexBuffer(this, resourceBuffer);
}
//----------------------------------------------------------------------------
size_t VertexBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool VertexBuffer::VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
    const DeviceAPIDependantResourceBuffer *resourceBuffer = 
        checked_cast<const DeviceAPIDependantResourceBuffer *>(entity);
    // no restriction on _vertexDeclaration :
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
