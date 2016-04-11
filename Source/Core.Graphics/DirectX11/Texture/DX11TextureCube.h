#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11AbstractTextureContent.h"

#include "Core.Graphics/Device/Texture/TextureCube.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11TextureCubeContent : public DX11AbstractTextureContent {
public:
    DX11TextureCubeContent(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData);
    DX11TextureCubeContent(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    DX11TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~DX11TextureCubeContent();

    ::ID3D11Texture2D *Texture() const { return _texture.Get(); }

    void GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, BufferMode mode, BufferUsage usage);
    void SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, BufferMode mode, BufferUsage usage);

    void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTextureCube *psource);

    void CopySubPart(   IDeviceAPIEncapsulator *device,
                        const DeviceAPIDependantTextureCube *dst, TextureCube::Face dstFace, size_t dstLevel, const uint2& dstPos,
                        const DeviceAPIDependantTextureCube *src, TextureCube::Face srcFace, size_t srcLevel, const AABB2u& srcBox );

private:
    ComPtr<::ID3D11Texture2D> _texture;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11TextureCube :
    public DeviceAPIDependantTextureCube
,   public DX11TextureCubeContent {
public:
    DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData);
    DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~DX11TextureCube();

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTextureCube *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                TextureCube::Face dstFace, size_t dstLevel, const uint2& dstPos,
                                const DeviceAPIDependantTextureCube *src, TextureCube::Face srcFace, size_t srcLevel, const AABB2u& srcBox ) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
