﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VertexBuffer.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexDeclaration.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVertexBufferBinding::Set(  size_t instanceFrequency,
                                size_t vertexOffset,
                                const Graphics::FVertexBuffer *vertexBuffer) {
    Assert(vertexBuffer);
    Assert(vertexBuffer->VertexCount() > vertexOffset);

    InstanceFrequency = instanceFrequency;
    VertexOffset = vertexOffset;
    VertexBuffer = vertexBuffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVertexBuffer::FVertexBuffer(const Graphics::FVertexDeclaration *vertexDeclaration, size_t vertexCount, EBufferMode mode, EBufferUsage usage, bool sharable)
:   FDeviceResourceSharable(EDeviceResourceType::Vertices, sharable)
,   _vertexDeclaration(vertexDeclaration)
,   _buffer(vertexDeclaration->SizeInBytes(), vertexCount, mode, usage) {
    Assert(vertexDeclaration);
    Assert(vertexCount);
}
//----------------------------------------------------------------------------
FVertexBuffer::~FVertexBuffer() {}
//----------------------------------------------------------------------------
void FVertexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    _buffer.SetData(device, offset, src);
}
//----------------------------------------------------------------------------
void FVertexBuffer::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(optionalRawData.empty() || optionalRawData.SizeInBytes() == _vertexDeclaration->SizeInBytes() * VertexCount());

    FDeviceAPIDependantResourceBuffer *const resourceBuffer = device->CreateVertexBuffer(this, &_buffer, optionalRawData);
    Assert(resourceBuffer);

    _buffer.Create(device, this, resourceBuffer);
}
//----------------------------------------------------------------------------
void FVertexBuffer::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);

    device->DestroyVertexBuffer(this, _buffer.Destroy(device, this));
}
//----------------------------------------------------------------------------
size_t FVertexBuffer::VirtualSharedKeyHashValue() const {
    return _buffer.HashValue();
}
//----------------------------------------------------------------------------
bool FVertexBuffer::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const FDeviceAPIDependantResourceBuffer *resourceBuffer =
        checked_cast<const FDeviceAPIDependantResourceBuffer *>(entity);
    // no restriction on _vertexDeclaration :
    return _buffer.Match(*resourceBuffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
