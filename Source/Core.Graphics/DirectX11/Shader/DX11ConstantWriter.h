#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ConstantBuffer.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ConstantWriter : public DeviceAPIDependantConstantWriter {
public:
    DX11ConstantWriter(IDeviceAPIEncapsulator *device);
    virtual ~DX11ConstantWriter();

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
} //!namespace Graphics
} //!namespace Core
