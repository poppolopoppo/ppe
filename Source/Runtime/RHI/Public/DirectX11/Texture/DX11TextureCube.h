#pragma once

#include "DirectX11/DX11Includes.h"

#include "DirectX11/Texture/DX11AbstractTextureContent.h"

#include "Device/Texture/TextureCube.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
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

    void GetContent(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst, EBufferMode mode, EBufferUsage usage);
    void SetContent(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src, EBufferMode mode, EBufferUsage usage);

    void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource);

    void CopySubPart(   IDeviceAPIEncapsulator *device,
                        const FDeviceAPIDependantTextureCube *dst, FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
                        const FDeviceAPIDependantTextureCube *src, FTextureCube::EFace srcFace, size_t srcLevel, const FAabb2u& srcBox );

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

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) override final;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) override final;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource) override final;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device,
                                FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
                                const FDeviceAPIDependantTextureCube *src, FTextureCube::EFace srcFace, size_t srcLevel, const FAabb2u& srcBox ) override final;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override final { return this; }

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
