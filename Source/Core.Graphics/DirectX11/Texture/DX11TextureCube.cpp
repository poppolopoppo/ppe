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
FDX11TextureCubeContent::FDX11TextureCubeContent(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData)
:   FDX11TextureCubeContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
FDX11TextureCubeContent::FDX11TextureCubeContent(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    FDX11AbstractTextureContent::CreateTexture(_texture, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, true);
}
//----------------------------------------------------------------------------
FDX11TextureCubeContent::FDX11TextureCubeContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   FDX11AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
FDX11TextureCubeContent::~FDX11TextureCubeContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void FDX11TextureCubeContent::GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, EBufferMode mode, EBufferUsage usage) {
    DX11ResourceGetData(device, _texture.Get(), 0, offset, dst, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void FDX11TextureCubeContent::SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, EBufferMode mode, EBufferUsage usage) {
    DX11ResourceSetData(device, _texture, 0, offset, src, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void FDX11TextureCubeContent::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource) {
    const FDX11TextureCube *psourceDX11 = checked_cast<const FDX11TextureCube *>(psource);
    DX11CopyResource(device, _texture.Get(), psourceDX11->Texture());
}
//----------------------------------------------------------------------------
void FDX11TextureCubeContent::CopySubPart(
    IDeviceAPIEncapsulator *device,
    const FDeviceAPIDependantTextureCube *dst, FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTextureCube *src, FTextureCube::EFace srcFace, size_t srcLevel, const AABB2u& srcBox ) {

    const size_t dstSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(dstLevel), UINT(dstFace), checked_cast<UINT>(dst->LevelCount()) );
    const uint3 dstPos3u(dstPos, 0);

    const FDX11TextureCube *psourceDX11 = checked_cast<const FDX11TextureCube *>(src);
    const size_t srcSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(srcLevel), UINT(srcFace), checked_cast<UINT>(src->LevelCount()) );
    const AABB3u srcBox3u(uint3(srcBox.Min(), 0), uint3(srcBox.Max(), 1));

    DX11CopyResourceSubRegion(device, _texture.Get(), dstSubResource, dstPos3u, psourceDX11->Texture(), srcSubResource, srcBox3u);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11TextureCube::FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantTextureCube(device, owner, optionalData)
,   FDX11TextureCubeContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
FDX11TextureCube::FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   FDeviceAPIDependantTextureCube(device, owner, optionalData)
,   FDX11TextureCubeContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
FDX11TextureCube::FDX11TextureCube(IDeviceAPIEncapsulator *device, FTextureCube *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   FDeviceAPIDependantTextureCube(device, owner, TMemoryView<const u8>())
,   FDX11TextureCubeContent(texture, shaderView) {}
//----------------------------------------------------------------------------
FDX11TextureCube::~FDX11TextureCube() {}
//----------------------------------------------------------------------------
void FDX11TextureCube::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    FDX11TextureCubeContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11TextureCube::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    FDX11TextureCubeContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11TextureCube::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTextureCube *psource) {
    FDX11TextureCubeContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void FDX11TextureCube::CopySubPart(
    IDeviceAPIEncapsulator *device,
    FTextureCube::EFace dstFace, size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTextureCube *psource, FTextureCube::EFace srcFace, size_t srcLevel, const AABB2u& srcBox ) {
    FDX11TextureCubeContent::CopySubPart(device, this, dstFace, dstLevel, dstPos, psource, srcFace, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11TextureCube, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
