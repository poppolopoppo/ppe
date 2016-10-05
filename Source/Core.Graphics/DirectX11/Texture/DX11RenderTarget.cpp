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
FDX11RenderTarget::FDX11RenderTarget(IDeviceAPIEncapsulator *device, FRenderTarget *owner, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantRenderTarget(device, owner, optionalData)
,   FDX11Texture2DContent(device, owner, optionalData, static_cast<::D3D11_BIND_FLAG>(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);
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
FDX11RenderTarget::FDX11RenderTarget(
    IDeviceAPIEncapsulator *device, FRenderTarget *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11RenderTargetView *renderTargetView)
:   FDeviceAPIDependantRenderTarget(device, owner, TMemoryView<const u8>())
,   FDX11Texture2DContent(texture, shaderView)
,   _renderTargetView(renderTargetView) {
    Assert(_renderTargetView);
}
//----------------------------------------------------------------------------
FDX11RenderTarget::~FDX11RenderTarget() {
    ReleaseComRef(_renderTargetView);
}
//----------------------------------------------------------------------------
void FDX11RenderTarget::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    FDX11Texture2DContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11RenderTarget::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    FDX11Texture2DContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11RenderTarget::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) {
    FDX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void FDX11RenderTarget::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) {
    FDX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11RenderTarget, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
