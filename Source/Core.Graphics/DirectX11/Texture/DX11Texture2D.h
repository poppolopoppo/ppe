#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11AbstractTextureContent.h"

#include "Core.Graphics/Device/Texture/Texture2D.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11Texture2DContent : public DX11AbstractTextureContent {
public:
    DX11Texture2DContent(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData);
    DX11Texture2DContent(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    DX11Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~DX11Texture2DContent();

    ::ID3D11Texture2D *Texture() const { return _texture.Get(); }

    void GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, BufferMode mode, BufferUsage usage);
    void SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, BufferMode mode, BufferUsage usage);

    void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource);

    void CopySubPart(   IDeviceAPIEncapsulator *device, 
                        const DeviceAPIDependantTexture2D *dst, size_t dstLevel, const uint2& dstPos, 
                        const DeviceAPIDependantTexture2D *src, size_t srcLevel, const AABB2u& srcBox );

private:
    ComPtr<::ID3D11Texture2D> _texture;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11Texture2D :
    public DeviceAPIDependantTexture2D
,   public DX11Texture2DContent {
public:
    DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData);
    DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~DX11Texture2D();

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, 
                                size_t dstLevel, const uint2& dstPos, 
                                const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(DX11Texture2D);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
