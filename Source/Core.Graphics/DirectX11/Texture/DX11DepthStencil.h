#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11Texture2D.h"

#include "Core.Graphics/Device/Texture/DepthStencil.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11DepthStencil :
    public FDeviceAPIDependantDepthStencil
,   public FDX11Texture2DContent
{
public:
    FDX11DepthStencil(IDeviceAPIEncapsulator *device, FDepthStencil *owner, const TMemoryView<const u8>& optionalData);
    FDX11DepthStencil(IDeviceAPIEncapsulator *device, FDepthStencil *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView);
    virtual ~FDX11DepthStencil();

    ::ID3D11DepthStencilView *DepthStencilView() const { return _depthStencilView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override final;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override final;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) override final;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const FAabb2u& srcBox ) override final;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override final { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11DepthStencilView> _depthStencilView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
