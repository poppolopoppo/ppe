#include "stdafx.h"

#include "DX11AbstractTextureContent.h"

#include "DirectX11/DX11DeviceEncapsulator.h"
#include "DirectX11/DX11ResourceBuffer.h"
#include "DirectX11/Texture/DX11SurfaceFormat.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Texture/SurfaceFormat.h"
#include "Device/Texture/Texture2D.h"
#include "Device/Texture/TextureCube.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractTextureContent::AbstractTextureContent(::ID3D11ShaderResourceView *shaderView/* = nullptr */)
:   _shaderView(shaderView) {}
//----------------------------------------------------------------------------
AbstractTextureContent::~AbstractTextureContent() {
    if (_shaderView) 
        ReleaseComRef(_shaderView);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::CreateTexture2D(
    ComPtr<::ID3D11Texture2D>& pTexture2D,
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture2D *owner,
    const MemoryView<const u8>& optionalData,
    ::D3D11_BIND_FLAG bindFlags ) {
    CreateTextureImpl_(pTexture2D, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, false);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::CreateTextureCube(
    ComPtr<::ID3D11Texture2D>& pTexture2D,
    IDeviceAPIEncapsulator *device,
    const Graphics::TextureCube *owner,
    const MemoryView<const u8>& optionalData,
    ::D3D11_BIND_FLAG bindFlags ) {
    CreateTextureImpl_(pTexture2D, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, true);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::GetContentTexture2D(
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture2D *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, void *const dst, size_t stride, size_t count ) {
    GetContentTextureImpl_(device, owner, texture, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::GetContentTextureCube(
    IDeviceAPIEncapsulator *device,
    const Graphics::TextureCube *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, void *const dst, size_t stride, size_t count ) {
    GetContentTextureImpl_(device, owner, texture, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::SetContentTexture2D(
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture2D *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, const void *src, size_t stride, size_t count ) {
    SetContentTextureImpl_(device, owner, texture, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::SetContentTextureCube(
    IDeviceAPIEncapsulator *device,
    const Graphics::TextureCube *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, const void *src, size_t stride, size_t count ) {
    SetContentTextureImpl_(device, owner, texture, offset, src, stride, count);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::CreateTextureImpl_(
    ComPtr<::ID3D11Texture2D>& pTexture2D,
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture *owner,
    size_t width,
    size_t height,
    size_t levelCount,
    const MemoryView<const u8>& optionalData,
    ::D3D11_BIND_FLAG bindFlags,
    bool isCubeMap ) {
    Assert(owner);
    Assert(nullptr == pTexture2D.Get());

    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);
    {
        ::D3D11_TEXTURE2D_DESC textureDesc;
        ::SecureZeroMemory(&textureDesc, sizeof(textureDesc));

        textureDesc.Width = checked_cast<UINT>(width);
        textureDesc.Height = checked_cast<UINT>(height);
        textureDesc.MipLevels = checked_cast<UINT>(levelCount);

        textureDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        textureDesc.ArraySize = (isCubeMap ? 6 : 1);

        // necessarry to bind the depth buffer to the device :
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
            Assert(owner->Usage() != BufferUsage::Immutable);

            DX11_THROW_IF_FAILED(device, owner, (
                wrapper->Device()->CreateTexture2D(&textureDesc, NULL, pTexture2D.GetAddressOf())
                ));
        }
        else {
            AssertRelease(optionalData.SizeInBytes() == owner->SizeInBytes()); // <=> 0 == offset
            Assert(IS_ALIGNED(16, optionalData.Pointer()));

            const auto initDatas = MALLOCA_VIEW(::D3D11_SUBRESOURCE_DATA, textureDesc.ArraySize * textureDesc.MipLevels);

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

        // necessarry to bind the depth buffer to the device :
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
void AbstractTextureContent::GetContentTextureImpl_(
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, void *const dst, size_t stride, size_t count) {
    Assert(owner);
    Assert(nullptr != texture.Get());
    Assert(IS_ALIGNED(16, dst));

    AssertRelease(owner->Usage() == BufferUsage::Staging);

    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    if (!DX11MapRead(wrapper->ImmediateContext(), texture.Get(), offset, dst, stride, count))
        throw DeviceEncapsulatorException("DX11: failed to map texture2D for reading", device);
}
//----------------------------------------------------------------------------
void AbstractTextureContent::SetContentTextureImpl_(
    IDeviceAPIEncapsulator *device,
    const Graphics::Texture *owner,
    const ComPtr<::ID3D11Texture2D>& texture,
    size_t offset, const void *src, size_t stride, size_t count ) {
    Assert(owner);
    Assert(nullptr != texture.Get());
    Assert(IS_ALIGNED(16, src));

    const BufferUsage usage = owner->Usage();
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    switch (usage)
    {
    case Core::Graphics::BufferUsage::Default:
        AssertRelease(stride * count == owner->SizeInBytes()); // <=> 0 == offset
        if (!DX11UpdateResource(wrapper->ImmediateContext(), texture.Get(), src, stride, count))
            throw DeviceEncapsulatorException("DX11: failed to update texture2D", device);
        break;

    case Core::Graphics::BufferUsage::Dynamic:
    case Core::Graphics::BufferUsage::Staging:
        if (!DX11MapWrite(wrapper->ImmediateContext(), texture.Get(), offset, src, stride, count, BufferUsage::Dynamic == usage))
            throw DeviceEncapsulatorException("DX11: failed to map texture2D for writing", device);
        break;

    case Core::Graphics::BufferUsage::Immutable:
        throw DeviceEncapsulatorException("DX11: immutable texture2D can't be muted", device, owner);

    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
