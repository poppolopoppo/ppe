#include "stdafx.h"

#include "DX11TextureCube.h"

#include "DirectX11/DX11DeviceEncapsulator.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextureCubeContent::TextureCubeContent(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData)
:   TextureCubeContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
TextureCubeContent::TextureCubeContent(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    AbstractTextureContent::CreateTextureCube(_texture, device, owner, optionalData, bindFlags);
}
//----------------------------------------------------------------------------
TextureCubeContent::TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
TextureCubeContent::~TextureCubeContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void TextureCubeContent::GetContent(IDeviceAPIEncapsulator *device, const Graphics::TextureCube *owner, size_t offset, void *const dst, size_t stride, size_t count) {
    AbstractTextureContent::GetContentTextureCube(device, owner, _texture, offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void TextureCubeContent::SetContent(IDeviceAPIEncapsulator *device, const Graphics::TextureCube *owner, size_t offset, const void *src, size_t stride, size_t count) {
    AbstractTextureContent::SetContentTextureCube(device, owner, _texture, offset, src, stride, count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextureCube::TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTextureCube(device, owner, optionalData)
,   TextureCubeContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
TextureCube::TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   DeviceAPIDependantTextureCube(device, owner, optionalData)
,   TextureCubeContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
TextureCube::TextureCube(IDeviceAPIEncapsulator *device, Graphics::TextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DeviceAPIDependantTextureCube(device, owner, MemoryView<const u8>())
,   TextureCubeContent(texture, shaderView) {}
//----------------------------------------------------------------------------
TextureCube::~TextureCube() {}
//----------------------------------------------------------------------------
void TextureCube::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11::TextureCubeContent::GetContent(device, Owner(), offset, dst, stride, count);
}
//----------------------------------------------------------------------------
void TextureCube::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11::TextureCubeContent::SetContent(device, Owner(), offset, src, stride, count);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TextureCube, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
