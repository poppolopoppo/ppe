#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

#include "Core/Container/Stack.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(ConstantBufferLayout);
class IDeviceAPIEncapsulator;
FWD_REFPTR(DeviceAPIDependantConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ConstantBuffer);
class FConstantBuffer : public FDeviceResourceSharable {
public:
    FConstantBuffer(const FConstantBufferLayout *layout, bool sharable);
    virtual ~FConstantBuffer();

    virtual bool Available() const override { return _buffer.Available(); }
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override { return _buffer.DeviceAPIDependantBuffer().get(); }

    const FDeviceAPIDependantConstantWriter *DeviceAPIDependantWriter() const {
        Assert(Available()); return _deviceAPIDependantWriter.get();
    }

    const FDeviceResourceBuffer& Buffer() const { return _buffer; }
    const FConstantBufferLayout* Layout() const { return _layout.get(); }

    void SetData(IDeviceAPIEncapsulator *device, const TMemoryView<const u8>& rawData);
    void SetData(IDeviceAPIEncapsulator *device, const TMemoryView<const void *>& fieldsData);

    void Create(IDeviceAPIEncapsulator *device);
    void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

private:
    FDeviceResourceBuffer _buffer;
    PCConstantBufferLayout _layout;
    PCDeviceAPIDependantConstantWriter _deviceAPIDependantWriter;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantConstantWriter : public FDeviceAPIDependantEntity {
public:
    FDeviceAPIDependantConstantWriter(IDeviceAPIEncapsulator *device);
    virtual ~FDeviceAPIDependantConstantWriter();

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const FConstantBuffer *resource,
        const TMemoryView<const u8>& rawData,
        const TMemoryView<u8>& output) const = 0;

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const FConstantBuffer *resource,
        const TMemoryView<const void *>& fieldsData,
        const TMemoryView<u8>& output) const = 0;

    virtual size_t VideoMemorySizeInBytes() const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
