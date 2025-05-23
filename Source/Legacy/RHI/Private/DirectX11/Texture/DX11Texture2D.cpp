﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11Texture2D.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/DX11ResourceHelpers.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11Texture2DContent::FDX11Texture2DContent(IDeviceAPIEncapsulator *device, FTexture2D *owner, const TMemoryView<const u8>& optionalData)
:   FDX11Texture2DContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
FDX11Texture2DContent::FDX11Texture2DContent(IDeviceAPIEncapsulator *device, FTexture2D *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    FDX11AbstractTextureContent::CreateTexture(_texture, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, false);
}
//----------------------------------------------------------------------------
FDX11Texture2DContent::FDX11Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   FDX11AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
FDX11Texture2DContent::~FDX11Texture2DContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void FDX11Texture2DContent::GetContent(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst, EBufferMode mode, EBufferUsage usage) {
    DX11ResourceGetData(device, _texture.Get(), 0, offset, dst, mode, usage);
}
//----------------------------------------------------------------------------
void FDX11Texture2DContent::SetContent(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src, EBufferMode mode, EBufferUsage usage) {
    DX11ResourceSetData(device, _texture.Get(), 0, offset, src, mode, usage);
}
//----------------------------------------------------------------------------
void FDX11Texture2DContent::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) {
    const FDX11Texture2D *psourceDX11 = checked_cast<const FDX11Texture2D *>(psource);
    DX11CopyResource(device, _texture.Get(), psourceDX11->Texture());
}
//----------------------------------------------------------------------------
void FDX11Texture2DContent::CopySubPart(
    IDeviceAPIEncapsulator *device,
    const FDeviceAPIDependantTexture2D *dst, size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTexture2D *src, size_t srcLevel, const FAabb2u& srcBox ) {

    const size_t dstSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(dstLevel), 0, checked_cast<UINT>(dst->LevelCount()) );
    const uint3 dstPos3u(dstPos, 0);

    const FDX11Texture2D *psourceDX11 = checked_cast<const FDX11Texture2D *>(src);
    const size_t srcSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(srcLevel), 0, checked_cast<UINT>(src->LevelCount()) );
    const FAabb3u srcBox3u(uint3(srcBox.Min(), 0), uint3(srcBox.Max(), 1));

    DX11CopyResourceSubRegion(device, _texture.Get(), dstSubResource, dstPos3u, psourceDX11->Texture(), srcSubResource, srcBox3u);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11Texture2D::FDX11Texture2D(IDeviceAPIEncapsulator *device, FTexture2D *owner, const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantTexture2D(device, owner, optionalData)
,   FDX11Texture2DContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
FDX11Texture2D::FDX11Texture2D(IDeviceAPIEncapsulator *device, FTexture2D *owner, const TMemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   FDeviceAPIDependantTexture2D(device, owner, optionalData)
,   FDX11Texture2DContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
FDX11Texture2D::FDX11Texture2D(IDeviceAPIEncapsulator *device, FTexture2D *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   FDeviceAPIDependantTexture2D(device, owner, TMemoryView<const u8>())
,   FDX11Texture2DContent(texture, shaderView) {}
//----------------------------------------------------------------------------
FDX11Texture2D::~FDX11Texture2D() {}
//----------------------------------------------------------------------------
void FDX11Texture2D::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    FDX11Texture2DContent::GetContent(device, offset, dst, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11Texture2D::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {
    FDX11Texture2DContent::SetContent(device, offset, src, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11Texture2D::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantTexture2D *psource) {
    FDX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void FDX11Texture2D::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const FDeviceAPIDependantTexture2D *psource, size_t srcLevel, const FAabb2u& srcBox ) {
    FDX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11Texture2D, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
