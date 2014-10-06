#pragma once

#include "DX11Includes.h"

#include "DeviceResourceBuffer.h"

#include "Core/ComPtr.h"
#include "Core/PoolAllocator.h"

namespace Core {
    template <typename T>
    class MemoryView;
} //!namespace Core

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class DeviceResourceType;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ResourceBuffer : public DeviceAPIDependantResourceBuffer {
public:
    ResourceBuffer( IDeviceAPIEncapsulator *device,
                    Graphics::DeviceResourceBuffer *owner,
                    const DeviceResource *resource,
                    const MemoryView<const u8>& optionalData);
    virtual ~ResourceBuffer();

    ::ID3D11Buffer *Entity() const { return _entity.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    SINGLETON_POOL_ALLOCATED_DECL(ResourceBuffer);

private:
    ComPtr<::ID3D11Buffer> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE ::ID3D11Buffer *DX11DeviceBufferEntity(const Graphics::DeviceResourceBuffer& resourceBuffer) {
    return checked_cast<const DX11::ResourceBuffer *>(resourceBuffer.DeviceAPIDependantBuffer().get())->Entity();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, const void *src, size_t stride, size_t count);
//----------------------------------------------------------------------------
bool DX11MapRead(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t offset, void *const dst, size_t stride, size_t count);
//----------------------------------------------------------------------------
bool DX11MapWrite(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t offset, const void *src, size_t stride, size_t count, bool discard);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(Graphics::BufferMode value);
//----------------------------------------------------------------------------
Graphics::BufferMode DX11CPUAccessFlagsToBufferMode(D3D11_CPU_ACCESS_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_USAGE BufferUsageToDX11Usage(Graphics::BufferUsage value);
//----------------------------------------------------------------------------
Graphics::BufferUsage DX11UsageToBufferUsage(D3D11_USAGE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(Graphics::DeviceResourceType value);
//----------------------------------------------------------------------------
Graphics::DeviceResourceType DX11BindFlagToDeviceResourceType(D3D11_BIND_FLAG value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
