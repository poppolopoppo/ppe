#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResource.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(VertexDeclaration);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct VertexBufferBinding {
    size_t InstanceFrequency;
    size_t VertexOffset;
    const Graphics::VertexBuffer *VertexBuffer;

    void Set(   size_t instanceFrequency,
                size_t vertexOffset,
                const Graphics::VertexBuffer *vertexBuffer);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VertexBuffer : public DeviceResource {
public:
    VertexBuffer(const Graphics::VertexDeclaration *vertexDeclaration, size_t vertexCount, BufferMode mode, BufferUsage usage);
    virtual ~VertexBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual DeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    size_t VertexCount() const { return _buffer.Count(); }
    const PCVertexDeclaration& VertexDeclaration() const { return _vertexDeclaration; }
    const DeviceResourceBuffer& Buffer() const { return _buffer; }

    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count);

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);
    void Destroy(IDeviceAPIEncapsulator *device);

private:
    PCVertexDeclaration _vertexDeclaration;
    DeviceResourceBuffer _buffer;
};
//----------------------------------------------------------------------------
template <typename T>
void VertexBuffer::Create(IDeviceAPIEncapsulator *device, const MemoryView<const T>& optionalData) {
    Create(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
