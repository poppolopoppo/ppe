#include "stdafx.h"

#include "DX11AbstractTextureContent.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/DX11ResourceBuffer.h"
#include "DirectX11/DX11ResourceHelpers.h"
#include "DirectX11/Texture/DX11SurfaceFormat.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResourceBuffer.h"
#include "Device/Texture/SurfaceFormat.h"
#include "Device/Texture/Texture2D.h"
#include "Device/Texture/TextureCube.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11AbstractTextureContent::FDX11AbstractTextureContent(::ID3D11ShaderResourceView *shaderView/* = nullptr */)
:   _shaderView(shaderView) {}
//----------------------------------------------------------------------------
FDX11AbstractTextureContent::~FDX11AbstractTextureContent() {
    if (_shaderView)
        ReleaseComRef(_shaderView);
}
//----------------------------------------------------------------------------
void FDX11AbstractTextureContent::CreateTexture(
    TComPtr<::ID3D11Texture2D>& pTexture2D,
    IDeviceAPIEncapsulator *device,
    const FTexture *owner,
    size_t width,
    size_t height,
    size_t levelCount,
    const TMemoryView<const u8>& optionalData,
    ::D3D11_BIND_FLAG bindFlags,
    bool isCubeMap ) {
    Assert(owner);
    Assert(nullptr == pTexture2D.Get());

    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);
    {
        ::D3D11_TEXTURE2D_DESC textureDesc;
        ::SecureZeroMemory(&textureDesc, sizeof(textureDesc));

        textureDesc.Width = checked_cast<UINT>(width);
        textureDesc.Height = checked_cast<UINT>(height);
        textureDesc.MipLevels = checked_cast<UINT>(levelCount);

        textureDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        textureDesc.ArraySize = (isCubeMap ? 6 : 1);

        // necessary to bind the depth buffer to the device :
        switch (textureDesc.Format)
        {
        case DXGI_FORMAT_D16_UNORM:
            textureDesc.Format = DXGI_FORMAT_R16_UNORM;
            break;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
            break;
        case DXGI_FORMAT_D32_FLOAT:
            textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
            break;

        default:
            break;
        }

        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;

        textureDesc.BindFlags = bindFlags;
        textureDesc.Usage = BufferUsageToDX11Usage(owner->Usage());
        textureDesc.CPUAccessFlags = BufferModeToDX11CPUAccessFlags(owner->Mode());
        textureDesc.MiscFlags = (isCubeMap ? ::D3D11_RESOURCE_MISC_TEXTURECUBE : 0);

        if (optionalData.empty()) {
            Assert(owner->Usage() != EBufferUsage::Immutable);

            DX11_THROW_IF_FAILED(device, owner, (
                wrapper->Device()->CreateTexture2D(&textureDesc, NULL, pTexture2D.GetAddressOf())
                ));
        }
        else {
            AssertRelease(optionalData.SizeInBytes() == owner->SizeInBytes()); // <=> 0 == offset
            Assert(IS_ALIGNED(16, optionalData.Pointer()));

            STACKLOCAL_POD_ARRAY(::D3D11_SUBRESOURCE_DATA, initDatas, textureDesc.ArraySize * textureDesc.MipLevels);

            size_t dataOffset = 0;
            for (size_t face = 0; face < textureDesc.ArraySize; ++face) {
                size_t w = textureDesc.Width;
                size_t h = textureDesc.Height;
                for (size_t level = 0; level < textureDesc.MipLevels; ++level) {
                    ::D3D11_SUBRESOURCE_DATA& initData = initDatas[face * textureDesc.MipLevels + level];

                    size_t rowBytes, numRows;
                    owner->Format()->SizeOfTexture2D(&rowBytes, &numRows, w, h);

                    size_t numBytes = numRows * rowBytes;

                    ::SecureZeroMemory(&initData, sizeof(initData));
                    initData.pSysMem = optionalData.SubRange(dataOffset, numBytes).Pointer();
                    initData.SysMemPitch = checked_cast<UINT>(rowBytes);
                    initData.SysMemSlicePitch = checked_cast<UINT>(numBytes);

                    dataOffset += numBytes;

                    w = std::max<size_t>(1, w >> 1);
                    h = std::max<size_t>(1, h >> 1);
                }
            }

            DX11_THROW_IF_FAILED(device, owner, (
                wrapper->Device()->CreateTexture2D(&textureDesc, initDatas.Pointer(), pTexture2D.GetAddressOf())
                ));
        }
    }

    Assert(nullptr != pTexture2D.Get());
    {
        ::D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        ::SecureZeroMemory(&viewDesc, sizeof(viewDesc));

        viewDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());

        // necessary to bind the depth buffer to the device :
        switch (viewDesc.Format)
        {
        case DXGI_FORMAT_D16_UNORM:
            viewDesc.Format = DXGI_FORMAT_R16_UNORM;
            break;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;
        case DXGI_FORMAT_D32_FLOAT:
            viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
            break;

        default:
            break;
        }

        viewDesc.ViewDimension = (isCubeMap ? ::D3D11_SRV_DIMENSION_TEXTURECUBE : ::D3D11_SRV_DIMENSION_TEXTURE2D);

        const UINT mipLevels = checked_cast<UINT>(levelCount);
        const UINT mostDetailedMip = 0; // 0 <=> largest mip available will be used

        if (isCubeMap) {
            viewDesc.TextureCube.MipLevels = mipLevels;
            viewDesc.TextureCube.MostDetailedMip = mostDetailedMip;
        }
        else {
            viewDesc.Texture2D.MipLevels = mipLevels;
            viewDesc.Texture2D.MostDetailedMip = mostDetailedMip;
        }

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateShaderResourceView(pTexture2D.Get(), &viewDesc, _shaderView.GetAddressOf())
            ));

        Assert(_shaderView);
    }

    DX11SetDeviceResourceNameIFP(pTexture2D, owner);
    DX11SetDeviceResourceNameIFP(_shaderView, owner);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
