#include "stdafx.h"

#include "DX11DepthStencil.h"

#include "DX11SurfaceFormat.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
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
DX11DepthStencil::DX11DepthStencil(IDeviceAPIEncapsulator *device, DepthStencil *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantDepthStencil(device, owner, optionalData)
,   DX11Texture2DContent(device, owner, optionalData, static_cast<::D3D11_BIND_FLAG>(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL)) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);
    {
        ::D3D11_DEPTH_STENCIL_VIEW_DESC dSDesc;
        ::SecureZeroMemory(&dSDesc, sizeof(dSDesc));

        dSDesc.Format = SurfaceFormatTypeToDXGIFormat(owner->Format()->Type());
        dSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        dSDesc.Texture2D.MipSlice = 0; // 0 <=> largest mip available will be used

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateDepthStencilView(Texture(), &dSDesc, _depthStencilView.GetAddressOf())
            ));
    }
    Assert(_depthStencilView);

    DX11SetDeviceResourceNameIFP(_depthStencilView, owner);
}
//----------------------------------------------------------------------------
DX11DepthStencil::DX11DepthStencil(
    IDeviceAPIEncapsulator *device, DepthStencil *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView)
:   DeviceAPIDependantDepthStencil(device, owner, MemoryView<const u8>())
,   DX11Texture2DContent(texture, shaderView)
,   _depthStencilView(depthStencilView) {
    Assert(depthStencilView);
}
//----------------------------------------------------------------------------
DX11DepthStencil::~DX11DepthStencil() {
    ReleaseComRef(_depthStencilView);
}
//----------------------------------------------------------------------------
void DX11DepthStencil::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11Texture2DContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11DepthStencil::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11Texture2DContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11DepthStencil::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) {
    DX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void DX11DepthStencil::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) {
    DX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11DepthStencil, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
