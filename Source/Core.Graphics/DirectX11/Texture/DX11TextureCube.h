#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/DirectX11/Texture/DX11AbstractTextureContent.h"

#include "Core.Graphics/Device/Texture/TextureCube.h"

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
class TextureCubeContent : public AbstractTextureContent {
public:
    TextureCubeContent(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData);
    TextureCubeContent(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~TextureCubeContent();

    ::ID3D11Texture2D *Texture() const { return _texture.Get(); }

    void GetContent(IDeviceAPIEncapsulator *device, const Graphics::TextureCube *owner, size_t offset, void *const dst, size_t stride, size_t count);
    void SetContent(IDeviceAPIEncapsulator *device, const Graphics::TextureCube *owner, size_t offset, const void *src, size_t stride, size_t count);

private:
    ComPtr<::ID3D11Texture2D> _texture;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TextureCube :
    public DeviceAPIDependantTextureCube
,   public TextureCubeContent {
public:
    TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData);
    TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~TextureCube();

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual const IDeviceAPIDependantAbstractTextureContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(TextureCube);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
