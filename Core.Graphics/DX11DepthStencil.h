#pragma once

#include "DX11Includes.h"

#include "DX11Texture2D.h"

#include "DepthStencil.h"

#include "Core/ComPtr.h"
#include "Core/PoolAllocator.h"

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
class DepthStencil :
    public DeviceAPIDependantDepthStencil
,   public DX11::Texture2DContent
{
public:
    DepthStencil(IDeviceAPIEncapsulator *device, Graphics::DepthStencil *owner, const MemoryView<const u8>& optionalData);
    DepthStencil(IDeviceAPIEncapsulator *device, Graphics::DepthStencil *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView);
    virtual ~DepthStencil();

    ::ID3D11DepthStencilView *DepthStencilView() const { return _depthStencilView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual const IDeviceAPIDependantTexture2DContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(DepthStencil);

private:
    ComPtr<::ID3D11DepthStencilView> _depthStencilView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
