#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ConstantBuffer.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11ConstantWriter : public FDeviceAPIDependantConstantWriter {
public:
    FDX11ConstantWriter(IDeviceAPIEncapsulator *device);
    virtual ~FDX11ConstantWriter();

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const FConstantBuffer *resource,
        const TMemoryView<const u8>& rawData,
        const TMemoryView<u8>& output) const override final;

    virtual void SetData(
        IDeviceAPIEncapsulator *device,
        const FConstantBuffer *resource,
        const TMemoryView<const void *>& fieldsData,
        const TMemoryView<u8>& output) const override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
