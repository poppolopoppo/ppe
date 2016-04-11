#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Texture/Texture.h"

#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class Texture2D;
class TextureCube;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11AbstractTextureContent : public IDeviceAPIDependantAbstractTextureContent {
public:
    DX11AbstractTextureContent(::ID3D11ShaderResourceView *shaderView = nullptr);
    virtual ~DX11AbstractTextureContent();

    ::ID3D11ShaderResourceView *ShaderView() const { return _shaderView.Get(); }

protected:
    void CreateTexture( ComPtr<::ID3D11Texture2D>& pTexture2D,
                        IDeviceAPIEncapsulator *device,
                        const Texture *owner,
                        size_t width,
                        size_t height,
                        size_t levelCount,
                        const MemoryView<const u8>& optionalData,
                        ::D3D11_BIND_FLAG bindFlags,
                        bool isCubeMap );

protected:
    ComPtr<::ID3D11ShaderResourceView> _shaderView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
