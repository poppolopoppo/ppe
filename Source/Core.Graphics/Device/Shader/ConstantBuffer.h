#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

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
class ConstantBuffer : public DeviceResourceSharable {
public:
    ConstantBuffer(const ConstantBufferLayout *layout, bool sharable);
    virtual ~ConstantBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual DeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    const DeviceAPIDependantConstantWriter *DeviceAPIDependantWriter() const {
        Assert(Available()); return _deviceAPIDependantWriter.get();
    }

    const DeviceResourceBuffer& Buffer() const { return _buffer; }
    const ConstantBufferLayout *Layout() const { return _layout.get(); }

    void SetData(IDeviceAPIEncapsulator *device, const MemoryView<const u8>& rawData);
    void SetData(IDeviceAPIEncapsulator *device, const MemoryView<const void *>& fieldsData);

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const override;

private:
    DeviceResourceBuffer _buffer;
    PCConstantBufferLayout _layout;
    PCDeviceAPIDependantConstantWriter _deviceAPIDependantWriter;
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
