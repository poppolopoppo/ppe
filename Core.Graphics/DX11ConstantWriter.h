#pragma once

#include "DX11Includes.h"

#include "ConstantBuffer.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ConstantWriter : public DeviceAPIDependantConstantWriter {
public:
    ConstantWriter(IDeviceAPIEncapsulator *device);
    virtual ~ConstantWriter();

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const ConstantBuffer *resource,
        const MemoryView<const u8>& rawData,
        const MemoryView<u8>& output) const override;

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const ConstantBuffer *resource,
        const MemoryView<const void *>& fieldsData,
        const MemoryView<u8>& output) const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
