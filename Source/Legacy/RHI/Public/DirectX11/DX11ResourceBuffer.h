#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/DeviceResourceBuffer.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
    template <typename T>
    class TMemoryView;
} //!namespace PPE

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class EDeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11ResourceBuffer : public FDeviceAPIDependantResourceBuffer {
public:
    FDX11ResourceBuffer( IDeviceAPIEncapsulator *device,
                        const FDeviceResource *resource,
                        const FDeviceResourceBuffer *buffer,
                        const TMemoryView<const u8>& optionalData);
    virtual ~FDX11ResourceBuffer();

    ::ID3D11Buffer *Entity() const { return _entity.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) override final;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) override final;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantResourceBuffer *psource) override final;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset,
                                const FDeviceAPIDependantResourceBuffer *psource, size_t srcOffset,
                                size_t length ) override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11Buffer> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline ::ID3D11Buffer *DX11DeviceBufferEntity(const FDeviceResourceBuffer& resourceBuffer) {
    return checked_cast<const FDX11ResourceBuffer *>(resourceBuffer.DeviceAPIDependantBuffer().get())->Entity();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(EBufferMode value);
//----------------------------------------------------------------------------
EBufferMode DX11CPUAccessFlagsToBufferMode(::D3D11_CPU_ACCESS_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_USAGE BufferUsageToDX11Usage(EBufferUsage value);
//----------------------------------------------------------------------------
EBufferUsage DX11UsageToBufferUsage(::D3D11_USAGE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(EDeviceResourceType value);
//----------------------------------------------------------------------------
EDeviceResourceType DX11BindFlagToDeviceResourceType(::D3D11_BIND_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
