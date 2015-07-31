#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"

#include "Core/Memory/MemoryStack.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(ConstantBufferLayout);
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ConstantBuffer);
class ConstantBuffer : public DeviceResource {
public:
    explicit ConstantBuffer(const ConstantBufferLayout *layout);
    virtual ~ConstantBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual DeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    const PDeviceAPIDependantConstantWriter& DeviceAPIDependantWriter() const {
        Assert(Frozen()); return _deviceAPIDependantWriter;
    }

    const DeviceResourceBuffer& Buffer() const { return _buffer; }
    const ConstantBufferLayout *Layout() const { return _layout.get(); }

    void SetData(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& rawData);
    void SetData(IDeviceAPIEncapsulator *device, const MemoryView<const void *>& fieldsData);

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

private:
    DeviceResourceBuffer _buffer;
    PCConstantBufferLayout _layout;
    PDeviceAPIDependantConstantWriter _deviceAPIDependantWriter;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantConstantWriter : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantConstantWriter(IDeviceAPIEncapsulator *device);
    virtual ~DeviceAPIDependantConstantWriter();

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const ConstantBuffer *resource,
        const MemoryView<const u8>& rawData,
        const MemoryView<u8>& output) const = 0;

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const ConstantBuffer *resource,
        const MemoryView<const void *>& fieldsData,
        const MemoryView<u8>& output) const = 0;

    virtual size_t VideoMemorySizeInBytes() const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
