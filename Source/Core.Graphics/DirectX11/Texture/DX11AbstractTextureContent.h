#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Texture/Texture.h"

#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class Texture2D;
class TextureCube;

namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractTextureContent : public IDeviceAPIDependantAbstractTextureContent {
public:
    AbstractTextureContent(::ID3D11ShaderResourceView *shaderView = nullptr);
    virtual ~AbstractTextureContent();

    ::ID3D11ShaderResourceView *ShaderView() const { return _shaderView.Get(); }

protected:
    void CreateTexture2D(   ComPtr<::ID3D11Texture2D>& pTexture2D,
                            IDeviceAPIEncapsulator *device, 
                            const Graphics::Texture2D *owner,
                            const MemoryView<const u8>& optionalData, 
                            ::D3D11_BIND_FLAG bindFlags );
    void CreateTextureCube( ComPtr<::ID3D11Texture2D>& pTexture2D,
                            IDeviceAPIEncapsulator *device, 
                            const Graphics::TextureCube *owner,
                            const MemoryView<const u8>& optionalData, 
                            ::D3D11_BIND_FLAG bindFlags );

    void GetContentTexture2D(   IDeviceAPIEncapsulator *device,
                                const Graphics::Texture2D *owner,
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, void *const dst, size_t stride, size_t count);
    void GetContentTextureCube( IDeviceAPIEncapsulator *device, 
                                const Graphics::TextureCube *owner, 
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, void *const dst, size_t stride, size_t count);

    void SetContentTexture2D(   IDeviceAPIEncapsulator *device,
                                const Graphics::Texture2D *owner,
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, const void *src, size_t stride, size_t count);
    void SetContentTextureCube( IDeviceAPIEncapsulator *device,
                                const Graphics::TextureCube *owner,
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, const void *src, size_t stride, size_t count);

private:
    void CreateTextureImpl_(ComPtr<::ID3D11Texture2D>& pTexture2D,
                            IDeviceAPIEncapsulator *device, 
                            const Graphics::Texture *owner,
                            size_t width,
                            size_t height,
                            size_t levelCount,
                            const MemoryView<const u8>& optionalData, 
                            ::D3D11_BIND_FLAG bindFlags,
                            bool isCubeMap );

    void GetContentTextureImpl_(IDeviceAPIEncapsulator *device,
                                const Graphics::Texture *owner,
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, void *const dst, size_t stride, size_t count);

    void SetContentTextureImpl_(IDeviceAPIEncapsulator *device,
                                const Graphics::Texture *owner,
                                const ComPtr<::ID3D11Texture2D>& texture,
                                size_t offset, const void *src, size_t stride, size_t count);

protected:
    ComPtr<::ID3D11ShaderResourceView> _shaderView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
