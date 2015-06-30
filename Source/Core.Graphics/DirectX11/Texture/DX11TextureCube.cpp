#include "stdafx.h"

#include "DX11TextureCube.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/DX11ResourceHelpers.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11TextureCubeContent::DX11TextureCubeContent(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData)
:   DX11TextureCubeContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
DX11TextureCubeContent::DX11TextureCubeContent(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    DX11AbstractTextureContent::CreateTexture(_texture, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, true);
}
//----------------------------------------------------------------------------
DX11TextureCubeContent::DX11TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DX11AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
DX11TextureCubeContent::~DX11TextureCubeContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void DX11TextureCubeContent::GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, BufferMode mode, BufferUsage usage) {
    DX11ResourceGetData(device, _texture.Get(), 0, offset, dst, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void DX11TextureCubeContent::SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, BufferMode mode, BufferUsage usage) {
    DX11ResourceSetData(device, _texture, 0, offset, src, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void DX11TextureCubeContent::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTextureCube *psource) {
    const DX11TextureCube *psourceDX11 = checked_cast<const DX11TextureCube *>(psource);
    DX11CopyResource(device, _texture.Get(), psourceDX11->Texture());
}
//----------------------------------------------------------------------------
void DX11TextureCubeContent::CopySubPart(
    IDeviceAPIEncapsulator *device,
    const DeviceAPIDependantTextureCube *dst, TextureCube::Face dstFace, size_t dstLevel, const uint2& dstPos,
    const DeviceAPIDependantTextureCube *src, TextureCube::Face srcFace, size_t srcLevel, const AABB2u& srcBox ) {
    
    const size_t dstSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(dstLevel), UINT(dstFace), checked_cast<UINT>(dst->LevelCount()) );
    const uint3 dstPos3u(dstPos, 0);

    const DX11TextureCube *psourceDX11 = checked_cast<const DX11TextureCube *>(src);
    const size_t srcSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(srcLevel), UINT(srcFace), checked_cast<UINT>(src->LevelCount()) );
    const AABB3u srcBox3u(uint3(srcBox.Min(), 0), uint3(srcBox.Max(), 1));

    DX11CopyResourceSubRegion(device, _texture.Get(), dstSubResource, dstPos3u, psourceDX11->Texture(), srcSubResource, srcBox3u);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11TextureCube::DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTextureCube(device, owner, optionalData)
,   DX11TextureCubeContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
DX11TextureCube::DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   DeviceAPIDependantTextureCube(device, owner, optionalData)
,   DX11TextureCubeContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
DX11TextureCube::DX11TextureCube(IDeviceAPIEncapsulator *device, TextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DeviceAPIDependantTextureCube(device, owner, MemoryView<const u8>())
,   DX11TextureCubeContent(texture, shaderView) {}
//----------------------------------------------------------------------------
DX11TextureCube::~DX11TextureCube() {}
//----------------------------------------------------------------------------
void DX11TextureCube::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11TextureCubeContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11TextureCube::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11TextureCubeContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11TextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTextureCube *psource) {
    DX11TextureCubeContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void DX11TextureCube::CopySubPart(
    IDeviceAPIEncapsulator *device,
    TextureCube::Face dstFace, size_t dstLevel, const uint2& dstPos,
    const DeviceAPIDependantTextureCube *psource, TextureCube::Face srcFace, size_t srcLevel, const AABB2u& srcBox ) {
    DX11TextureCubeContent::CopySubPart(device, this, dstFace, dstLevel, dstPos, psource, srcFace, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Graphics, DX11TextureCube, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
