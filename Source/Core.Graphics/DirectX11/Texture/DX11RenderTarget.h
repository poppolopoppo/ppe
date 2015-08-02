#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11Texture2D.h"

#include "Core.Graphics/Device/Texture/RenderTarget.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11RenderTarget :
    public DeviceAPIDependantRenderTarget
,   public DX11Texture2DContent {
public:
    DX11RenderTarget(IDeviceAPIEncapsulator *device, RenderTarget *owner, const MemoryView<const u8>& optionalData);
    DX11RenderTarget(IDeviceAPIEncapsulator *device, RenderTarget *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView);
    virtual ~DX11RenderTarget();

    ::ID3D11RenderTargetView *RenderTargetView() const { return _renderTargetView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, 
                                size_t dstLevel, const uint2& dstPos, 
                                const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ComPtr<::ID3D11RenderTargetView> _renderTargetView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
