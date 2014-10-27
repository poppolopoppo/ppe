#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11Texture2D.h"

#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

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
class RenderTarget :
    public DeviceAPIDependantRenderTarget
,   public DX11::Texture2DContent {
public:
    RenderTarget(IDeviceAPIEncapsulator *device, Graphics::RenderTarget *owner, const MemoryView<const u8>& optionalData);
    RenderTarget(IDeviceAPIEncapsulator *device, Graphics::RenderTarget *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView);
    virtual ~RenderTarget();

    ::ID3D11RenderTargetView *RenderTargetView() const { return _renderTargetView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual const IDeviceAPIDependantTexture2DContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderTarget);

private:
    ComPtr<::ID3D11RenderTargetView> _renderTargetView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
