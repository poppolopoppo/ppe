#pragma once

#include "DX11Includes.h"

#include "Texture2D.h"

#include "Core/ComPtr.h"
#include "Core/PoolAllocator.h"

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
class Texture2DContent : public IDeviceAPIDependantTexture2DContent {
public:
    Texture2DContent(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData);
    Texture2DContent(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~Texture2DContent();

    ::ID3D11Texture2D *Texture() const { return _texture.Get(); }
    ::ID3D11ShaderResourceView *ShaderView() const { return _shaderView.Get(); }

    void GetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, void *const dst, size_t stride, size_t count);
    void SetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, const void *src, size_t stride, size_t count);

private:
    ComPtr<::ID3D11Texture2D> _texture;
    ComPtr<::ID3D11ShaderResourceView> _shaderView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Texture2D :
    public DeviceAPIDependantTexture2D
,   public Texture2DContent {
public:
    Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData);
    Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags);
    Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView);
    virtual ~Texture2D();

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) override;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) override;

    virtual const IDeviceAPIDependantTexture2DContent *Content() const override { return this; }

    SINGLETON_POOL_ALLOCATED_DECL(Texture2D);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
