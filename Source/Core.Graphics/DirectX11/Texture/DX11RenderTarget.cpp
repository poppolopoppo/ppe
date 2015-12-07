#include "stdafx.h"

#include "DX11RenderTarget.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "DX11SurfaceFormat.h"
#include "DirectX11/DX11ResourceBuffer.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11RenderTarget::DX11RenderTarget(IDeviceAPIEncapsulator *device, RenderTarget *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantRenderTarget(device, owner, optionalData)
,   DX11Texture2DContent(device, owner, optionalData, static_cast<::D3D11_BIND_FLAG>(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);
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
DX11RenderTarget::DX11RenderTarget(
    IDeviceAPIEncapsulator *device, RenderTarget *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView)
:   DeviceAPIDependantRenderTarget(device, owner, MemoryView<const u8>())
,   DX11Texture2DContent(texture, shaderView)
,   _renderTargetView(renderTargetView) {
    Assert(_renderTargetView);
}
//----------------------------------------------------------------------------
DX11RenderTarget::~DX11RenderTarget() {
    ReleaseComRef(_renderTargetView);
}
//----------------------------------------------------------------------------
void DX11RenderTarget::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11Texture2DContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11RenderTarget::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11Texture2DContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11RenderTarget::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) {
    DX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void DX11RenderTarget::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) {
    DX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11RenderTarget, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
