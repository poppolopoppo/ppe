#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/DeviceResourceBuffer.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
    template <typename T>
    class MemoryView;
} //!namespace Core

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class DeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ResourceBuffer : public DeviceAPIDependantResourceBuffer {
public:
    DX11ResourceBuffer( IDeviceAPIEncapsulator *device,
                        const DeviceResource *resource,
                        const DeviceResourceBuffer *buffer,
                        const MemoryView<const u8>& optionalData);
    virtual ~DX11ResourceBuffer();

    ::ID3D11Buffer *Entity() const { return _entity.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantResourceBuffer *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset, 
                                const DeviceAPIDependantResourceBuffer *psource, size_t srcOffset, 
                                size_t length ) override;

    SINGLETON_POOL_ALLOCATED_DECL(DX11ResourceBuffer);

private:
    ComPtr<::ID3D11Buffer> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline ::ID3D11Buffer *DX11DeviceBufferEntity(const DeviceResourceBuffer& resourceBuffer) {
    return checked_cast<const DX11ResourceBuffer *>(resourceBuffer.DeviceAPIDependantBuffer().get())->Entity();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(BufferMode value);
//----------------------------------------------------------------------------
BufferMode DX11CPUAccessFlagsToBufferMode(::D3D11_CPU_ACCESS_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_USAGE BufferUsageToDX11Usage(BufferUsage value);
//----------------------------------------------------------------------------
BufferUsage DX11UsageToBufferUsage(::D3D11_USAGE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(DeviceResourceType value);
//----------------------------------------------------------------------------
DeviceResourceType DX11BindFlagToDeviceResourceType(::D3D11_BIND_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
