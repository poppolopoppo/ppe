#pragma once

#include "Graphics.h"

#include "DeviceResource.h"
#include "DeviceResourceBuffer.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class IndexElementSize {
    SixteenBits     = sizeof(u16),
    ThirtyTwoBits   = sizeof(u32)
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(IndexBuffer);
class IndexBuffer : public TypedDeviceResource<DeviceResourceType::Indices> {
public:
    IndexBuffer(Graphics::IndexElementSize indexElementSize, size_t indexCount, BufferMode mode, BufferUsage usage);
    virtual ~IndexBuffer();

    virtual bool Available() const override { return _buffer.Available(); }

    size_t IndexCount() const { return _buffer.Count(); }
    Graphics::IndexElementSize IndexElementSize() const { return static_cast<Graphics::IndexElementSize>(_buffer.Stride()); }
    const DeviceResourceBuffer& Buffer() const { return _buffer; }

    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const u16 *src, size_t count);
    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const u32 *src, size_t count);

    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const u16>& optionalData);
    void Create(IDeviceAPIEncapsulator *device, const MemoryView<const u32>& optionalData);
    void Destroy(IDeviceAPIEncapsulator *device);

private:
    void Create_(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& optionalRawData);
    void SetData_(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count);

    DeviceResourceBuffer _buffer;
};
//----------------------------------------------------------------------------
inline void IndexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const u16 *src, size_t count) {
    Assert(Graphics::IndexElementSize::SixteenBits == IndexElementSize());
    SetData_(device, offset, src, sizeof(u16), count);
}
//----------------------------------------------------------------------------
inline void IndexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const u32 *src, size_t count) {
    Assert(Graphics::IndexElementSize::ThirtyTwoBits == IndexElementSize());
    SetData_(device, offset, src, sizeof(u32), count);
}
//----------------------------------------------------------------------------
inline void IndexBuffer::Create(IDeviceAPIEncapsulator *device, const MemoryView<const u16>& optionalData) {
    Assert(Graphics::IndexElementSize::SixteenBits == IndexElementSize());
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
inline void IndexBuffer::Create(IDeviceAPIEncapsulator *device, const MemoryView<const u32>& optionalData) {
    Assert(Graphics::IndexElementSize::ThirtyTwoBits == IndexElementSize());
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
