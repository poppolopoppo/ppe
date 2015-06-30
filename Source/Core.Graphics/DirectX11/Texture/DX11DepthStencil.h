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
class DX11DepthStencil :
    public DeviceAPIDependantDepthStencil
,   public DX11Texture2DContent
{
public:
    DX11DepthStencil(IDeviceAPIEncapsulator *device, DepthStencil *owner, const MemoryView<const u8>& optionalData);
    DX11DepthStencil(IDeviceAPIEncapsulator *device, DepthStencil *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView);
    virtual ~DX11DepthStencil();

    ::ID3D11DepthStencilView *DepthStencilView() const { return _depthStencilView.Get(); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                size_t dstLevel, const uint2& dstPos,
                                const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(DX11DepthStencil);

private:
    ComPtr<::ID3D11DepthStencilView> _depthStencilView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
