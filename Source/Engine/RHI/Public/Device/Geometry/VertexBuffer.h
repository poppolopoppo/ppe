#pragma once

#include "Graphics.h"

#include "Device/DeviceResourceBuffer.h"
#include "Device/Pool/DeviceResourceSharable.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(VertexDeclaration);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVertexBufferBinding {
    size_t InstanceFrequency;
    size_t VertexOffset;
    const Graphics::FVertexBuffer *VertexBuffer;

    void Set(   size_t instanceFrequency,
                size_t vertexOffset,
                const Graphics::FVertexBuffer *vertexBuffer);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVertexBuffer : public FDeviceResourceSharable {
public:
    FVertexBuffer(   const Graphics::FVertexDeclaration *vertexDeclaration, size_t vertexCount,
                    EBufferMode mode, EBufferUsage usage,
                    bool sharable );
    virtual ~FVertexBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    size_t VertexCount() const { return _buffer.Count(); }
    const PCVertexDeclaration& VertexDeclaration() const { return _vertexDeclaration; }
    const FDeviceResourceBuffer& Buffer() const { return _buffer; }

    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src);

    template <typename T>
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData);
    void Create(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& optionalRawData);
    void Create(IDeviceAPIEncapsulator *device) { Create(device, TMemoryView<const u8>());  }
    void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

private:
    PCVertexDeclaration _vertexDeclaration;
    FDeviceResourceBuffer _buffer;
};
//----------------------------------------------------------------------------
template <typename T>
void FVertexBuffer::Create(IDeviceAPIEncapsulator *device, const TMemoryView<const T>& optionalData) {
    Create(device, optionalData.Cast<const u8>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
