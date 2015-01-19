#include "stdafx.h"

#include "DX11Texture2D.h"

#include "DirectX11/DX11DeviceEncapsulator.h"

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
    AbstractTextureContent::CreateTexture2D(_texture, device, owner, optionalData, bindFlags);
}
//----------------------------------------------------------------------------
Texture2DContent::Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
Texture2DContent::~Texture2DContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void Texture2DContent::GetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, void *const dst, size_t stride, size_t count) {
    AbstractTextureContent::GetContentTexture2D(device, owner, _texture, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void Texture2DContent::SetContent(IDeviceAPIEncapsulator *device, const Graphics::Texture2D *owner, size_t offset, const void *src, size_t stride, size_t count) {
    AbstractTextureContent::SetContentTexture2D(device, owner, _texture, offset, src, stride, count);
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
