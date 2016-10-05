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
class FDX11TextureCubeContent : public FDX11AbstractTextureContent {
public:
    FDX11TextureCubeContent(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData);
    FDX11TextureCubeContent(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    FDX11TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~FDX11TextureCubeContent();

    ::ID3D11Texture2D *Texture() const { return _texture.Get(); }

    void GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, EBufferMode mode, EBufferUsage usage);
    void SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, EBufferMode mode, EBufferUsage usage);

    void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource);

    void CopySubPart(   IDeviceAPIEncapsulator *device,
                        const FDeviceAPIDependantTextureCube *dst, FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
                        const FDeviceAPIDependantTextureCube *src, FTextureCube::EFace srcFace, size_t srcLevel, const AABB2u& srcBox );

private:
    TComPtr<::ID3D11Texture2D> _texture;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11TextureCube :
    public FDeviceAPIDependantTextureCube
,   public FDX11TextureCubeContent {
public:
    FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData);
    FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~FDX11TextureCube();

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource) override;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTextureCube *src, FTextureCube::EFace srcFace, size_t srcLevel, const AABB2u& srcBox ) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
