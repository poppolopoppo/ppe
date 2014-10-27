#include "stdafx.h"

#include "DX11Texture2D.h"

#include "DirectX11/DX11DeviceEncapsulator.h"

#include "DX11SurfaceFormat.h"
#include "DirectX11/DX11ResourceBuffer.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Texture2DContent::Texture2DContent(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData)
:   Texture2DContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
Texture2DContent::Texture2DContent(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);
    {
        ::D3D11_TEXTURE2D_DESC textureDesc;
        ::SecureZeroMemory(&textureDesc, sizeof(textureDesc));

        textureDesc.Width = checked_cast<UINT>(owner->Width());
        textureDesc.Height = checked_cast<UINT>(owner->Height());
        textureDesc.MipLevels = checked_cast<UINT>(owner->LevelCount());

        textureDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        textureDesc.ArraySize = 1; // > 1 for Texture2D Array, or Texture2D Cube // TODO : ?

        textureDesc.SampleDesc.Count = 1; // TODO ?
        textureDesc.SampleDesc.Quality = 0;

        textureDesc.BindFlags = bindFlags;
        textureDesc.Usage = BufferUsageToDX11Usage(owner->Usage());
        textureDesc.CPUAccessFlags = BufferModeToDX11CPUAccessFlags(owner->Mode());
        textureDesc.MiscFlags = 0;

        if (optionalData.empty()) {
            Assert(owner->Usage() != BufferUsage::Immutable);

            DX11_THROW_IF_FAILED(device, owner, (
                wrapper->Device()->CreateTexture2D(&textureDesc, NULL, _texture.GetAddressOf())
                ));
        }
        else {
            AssertRelease(optionalData.SizeInBytes() == owner->SizeInBytes()); // <=> 0 == offset
            Assert(IS_ALIGNED(16, optionalData.Pointer()));

            const auto initDatas = MALLOCA_VIEW(::D3D11_SUBRESOURCE_DATA, textureDesc.ArraySize * textureDesc.MipLevels);

            Assert(1 == textureDesc.ArraySize); // only used for cube texture or texture arrays, would be supported in DX11Texture3D and DX11Texture2DArray

            size_t w = textureDesc.Width;
            size_t h = textureDesc.Height;
            size_t dataOffset = 0;
            for (::D3D11_SUBRESOURCE_DATA& initData : initDatas) {
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

            DX11_THROW_IF_FAILED(device, owner, (
                wrapper->Device()->CreateTexture2D(&textureDesc, initDatas.Pointer(), _texture.GetAddressOf())
                ));
        }
    }
    Assert(_texture);
    if (D3D11_BIND_DEPTH_STENCIL != (bindFlags & D3D11_BIND_DEPTH_STENCIL)) {
        ::D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        ::SecureZeroMemory(&viewDesc, sizeof(viewDesc));

        viewDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

        viewDesc.Texture2D.MipLevels = checked_cast<UINT>(owner->LevelCount());
        viewDesc.Texture2D.MostDetailedMip = 0; // 0 <=> largest mip available will be used

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateShaderResourceView(_texture.Get(), &viewDesc, _shaderView.GetAddressOf())
            ));

        Assert(_shaderView);
    }

    DX11SetDeviceResourceNameIFP(_texture, owner);
    DX11SetDeviceResourceNameIFP(_shaderView, owner);
}
//----------------------------------------------------------------------------
Texture2DContent::Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   _texture(texture), _shaderView(shaderView) {
    Assert(texture);
    //Assert(shaderView);
}
//----------------------------------------------------------------------------
Texture2DContent::~Texture2DContent() {
    if (_shaderView) ReleaseComRef(_shaderView);
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void Texture2DContent::GetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, void *const dst, size_t stride, size_t count) {
    AssertRelease(owner->Usage() == BufferUsage::Staging);

    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    if (!DX11MapRead(wrapper->ImmediateContext(), _texture.Get(), offset, dst, stride, count))
        throw DeviceEncapsulatorException("DX11: failed to map texture2D for reading", device);
}
//----------------------------------------------------------------------------
void Texture2DContent::SetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, const void *src, size_t stride, size_t count) {
    Assert(IS_ALIGNED(16, src));
    Assert(_texture);

    const BufferUsage usage = owner->Usage();
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    switch (usage)
    {
    case Core::Graphics::BufferUsage::Default:
        AssertRelease(stride * count == owner->SizeInBytes()); // <=> 0 == offset
        if (!DX11UpdateResource(wrapper->ImmediateContext(), _texture.Get(), src, stride, count))
            throw DeviceEncapsulatorException("DX11: failed to update texture2D", device);
        break;

    case Core::Graphics::BufferUsage::Dynamic:
    case Core::Graphics::BufferUsage::Staging:
        if (!DX11MapWrite(wrapper->ImmediateContext(), _texture.Get(), offset, src, stride, count, BufferUsage::Dynamic == usage))
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
Texture2D::Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture2D(device, owner, optionalData)
,   Texture2DContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
Texture2D::Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   DeviceAPIDependantTexture2D(device, owner, optionalData)
,   Texture2DContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
Texture2D::Texture2D(IDeviceAPIEncapsulator *device, Graphics::Texture2D *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DeviceAPIDependantTexture2D(device, owner, MemoryView<const u8>())
,   Texture2DContent(texture, shaderView) {}
//----------------------------------------------------------------------------
Texture2D::~Texture2D() {}
//----------------------------------------------------------------------------
void Texture2D::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11::Texture2DContent::GetContent(device, Owner(), offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void Texture2D::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11::Texture2DContent::SetContent(device, Owner(), offset, src, stride, count);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Texture2D, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
