#include "stdafx.h"

#include "DX11RenderTarget.h"

#include "DX11DeviceEncapsulator.h"

#include "DX11SurfaceFormat.h"
#include "DX11ResourceBuffer.h"

#include "DeviceAPIEncapsulator.h"
#include "DeviceEncapsulatorException.h"
#include "SurfaceFormat.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderTarget::RenderTarget(IDeviceAPIEncapsulator *device, Graphics::RenderTarget *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantRenderTarget(device, owner, optionalData)
,   DX11::Texture2DContent(device, owner, optionalData, static_cast<::D3D11_BIND_FLAG>(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);
    {
        ::D3D11_RENDER_TARGET_VIEW_DESC rTDesc;
        ::SecureZeroMemory(&rTDesc, sizeof(rTDesc));

        rTDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        rTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        rTDesc.Texture2D.MipSlice = 0; // 0 <=> largest mip available will be used

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateRenderTargetView(Texture(), &rTDesc, _renderTargetView.GetAddressOf())
            ));
    }
    Assert(_renderTargetView);

    DX11SetDeviceResourceNameIFP(_renderTargetView, owner);
}
//----------------------------------------------------------------------------
RenderTarget::RenderTarget(
    IDeviceAPIEncapsulator *device, Graphics::RenderTarget *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView)
:   DeviceAPIDependantRenderTarget(device, owner, MemoryView<const u8>())
,   DX11::Texture2DContent(texture, shaderView)
,   _renderTargetView(renderTargetView) {
    Assert(_renderTargetView);
}
//----------------------------------------------------------------------------
RenderTarget::~RenderTarget() {
    ReleaseComRef(_renderTargetView);
}
//----------------------------------------------------------------------------
void RenderTarget::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11::Texture2DContent::GetContent(device, Owner(), offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void RenderTarget::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11::Texture2DContent::SetContent(device, Owner(), offset, src, stride, count);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(RenderTarget, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
