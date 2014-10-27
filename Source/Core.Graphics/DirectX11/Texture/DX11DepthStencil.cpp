#include "stdafx.h"

#include "DX11DepthStencil.h"

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
DepthStencil::DepthStencil(IDeviceAPIEncapsulator *device, Graphics::DepthStencil *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantDepthStencil(device, owner, optionalData)
,   DX11::Texture2DContent(device, owner, optionalData, D3D11_BIND_DEPTH_STENCIL) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);
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
DepthStencil::DepthStencil(
    IDeviceAPIEncapsulator *device, Graphics::DepthStencil *owner,
    ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView, ::ID3D11DepthStencilView *depthStencilView)
:   DeviceAPIDependantDepthStencil(device, owner, MemoryView<const u8>())
,   DX11::Texture2DContent(texture, shaderView)
,   _depthStencilView(depthStencilView) {
    Assert(depthStencilView);
}
//----------------------------------------------------------------------------
DepthStencil::~DepthStencil() {
    ReleaseComRef(_depthStencilView);
}
//----------------------------------------------------------------------------
void DepthStencil::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11::Texture2DContent::GetContent(device, Owner(), offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void DepthStencil::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11::Texture2DContent::SetContent(device, Owner(), offset, src, stride, count);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(DepthStencil, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
