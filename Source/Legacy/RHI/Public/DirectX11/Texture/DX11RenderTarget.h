#pragma once

#include "DirectX11/DX11Includes.h"

#include "DirectX11/Texture/DX11Texture2D.h"

#include "Device/Texture/RenderTarget.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11RenderTarget :
    public FDeviceAPIDependantRenderTarget
,   public FDX11Texture2DContent {
public:
    FDX11RenderTarget(IDeviceAPIEncapsulator *device, FRenderTarget *owner, const TMemoryView<const u8>& optionalData);
    FDX11RenderTarget(IDeviceAPIEncapsulator *device, FRenderTarget *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView);
    virtual ~FDX11RenderTarget();

    ::ID3D11RenderTargetView *RenderTargetView() const { return _renderTargetView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) override final;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) override final;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) override final;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const FAabb2u& srcBox ) override final;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override final { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11RenderTargetView> _renderTargetView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
