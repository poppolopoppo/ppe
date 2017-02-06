#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(IndexBuffer);
class IndexBuffer : public FDeviceResourceSharable {
public:
    IndexBuffer(Graphics::EIndexElementSize indexElementSize, size_t indexCount,
                EBufferMode mode, EBufferUsage usage,
                bool sharable );
    virtual ~IndexBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    size_t IndexCount() const { return _buffer.Count(); }
    Graphics::EIndexElementSize IndexElementSize() const { return static_cast<Graphics::EIndexElementSize>(_buffer.Stride()); }
    const FDeviceResourceBuffer& Buffer() const { return _buffer; }

    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const u16 *src, size_t count);
    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const u32 *src, size_t count);

    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u16>& optionalData);
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u32>& optionalData);
    void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

private:
    void Create_(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);
    void SetData_(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count);

    FDeviceResourceBuffer _buffer;
};
//----------------------------------------------------------------------------
inline void IndexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const u16 *src, size_t count) {
    Assert(Graphics::EIndexElementSize::SixteenBits == IndexElementSize());
    SetData_(device, offset, src, sizeof(u16), count);
}
//----------------------------------------------------------------------------
inline void IndexBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const u32 *src, size_t count) {
    Assert(Graphics::EIndexElementSize::ThirtyTwoBits == IndexElementSize());
    SetData_(device, offset, src, sizeof(u32), count);
}
//----------------------------------------------------------------------------
inline void IndexBuffer::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u16>& optionalData) {
    Assert(Graphics::EIndexElementSize::SixteenBits == IndexElementSize());
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
inline void IndexBuffer::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u32>& optionalData) {
    Assert(Graphics::EIndexElementSize::ThirtyTwoBits == IndexElementSize());
    Create_(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
