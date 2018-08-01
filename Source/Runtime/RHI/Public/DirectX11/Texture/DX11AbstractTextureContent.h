#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/Texture/Texture.h"

#include "Memory/ComPtr.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
class FTexture2D;
class FTextureCube;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11AbstractTextureContent : public IDeviceAPIDependantAbstractTextureContent {
public:
    FDX11AbstractTextureContent(::ID3D11ShaderResourceView *shaderView = nullptr);
    virtual ~FDX11AbstractTextureContent();

    ::ID3D11ShaderResourceView *ShaderView() const { return _shaderView.Get(); }

protected:
    void CreateTexture( TComPtr<::ID3D11Texture2D>& pTexture2D,
                        IDeviceAPIEncapsulator *device,
                        const FTexture *owner,
                        size_t width,
                        size_t height,
                        size_t levelCount,
                        const TMemoryView<const u8>& optionalData,
                        ::D3D11_BIND_FLAG bindFlags,
                        bool isCubeMap );

protected:
    TComPtr<::ID3D11ShaderResourceView> _shaderView;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
