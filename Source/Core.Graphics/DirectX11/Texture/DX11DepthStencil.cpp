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
FDX11DepthStencil::FDX11DepthStencil(IDeviceAPIEncapsulator *device, FDepthStencil *owner, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantDepthStencil(device, owner, optionalData)
,   FDX11Texture2DContent(device, owner, optionalData, static_cast<::D3D11_BIND_FLAG>(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL)) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);
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

    DX11SetDeviceResourceNameIFP(_depthStencilView.Get(), owner);
}
//----------------------------------------------------------------------------
FDX11DepthStencil::FDX11DepthStencil(
    IDeviceAPIEncapsulator *device, FDepthStencil *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView)
:   FDeviceAPIDependantDepthStencil(device, owner, TMemoryView<const u8>())
,   FDX11Texture2DContent(texture, shaderView)
,   _depthStencilView(depthStencilView) {
    Assert(depthStencilView);
}
//----------------------------------------------------------------------------
FDX11DepthStencil::~FDX11DepthStencil() {
    ReleaseComRef(_depthStencilView);
}
//----------------------------------------------------------------------------
void FDX11DepthStencil::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    FDX11Texture2DContent::GetContent(device, offset, dst, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11DepthStencil::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    FDX11Texture2DContent::SetContent(device, offset, src, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11DepthStencil::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) {
    FDX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void FDX11DepthStencil::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const FAabb2u& srcBox ) {
    FDX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11DepthStencil, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
