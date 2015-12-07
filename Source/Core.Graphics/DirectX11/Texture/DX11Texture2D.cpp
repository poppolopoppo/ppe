#include "stdafx.h"

#include "DX11Texture2D.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/DX11ResourceHelpers.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11Texture2DContent::DX11Texture2DContent(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData)
:   DX11Texture2DContent(device, owner, optionalData, D3D11_BIND_SHADER_RESOURCE) {}
//----------------------------------------------------------------------------
DX11Texture2DContent::DX11Texture2DContent(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags) {
    DX11AbstractTextureContent::CreateTexture(_texture, device, owner, owner->Width(), owner->Height(), owner->LevelCount(), optionalData, bindFlags, false);
}
//----------------------------------------------------------------------------
DX11Texture2DContent::DX11Texture2DContent(::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DX11AbstractTextureContent(shaderView)
,   _texture(texture) {
    Assert(texture);
}
//----------------------------------------------------------------------------
DX11Texture2DContent::~DX11Texture2DContent() {
    ReleaseComRef(_texture);
}
//----------------------------------------------------------------------------
void DX11Texture2DContent::GetContent(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count, BufferMode mode, BufferUsage usage) {
    DX11ResourceGetData(device, _texture.Get(), 0, offset, dst, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void DX11Texture2DContent::SetContent(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count, BufferMode mode, BufferUsage usage) {
    DX11ResourceSetData(device, _texture.Get(), 0, offset, src, stride, count, mode, usage);
}
//----------------------------------------------------------------------------
void DX11Texture2DContent::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) {
    const DX11Texture2D *psourceDX11 = checked_cast<const DX11Texture2D *>(psource);
    DX11CopyResource(device, _texture.Get(), psourceDX11->Texture());
}
//----------------------------------------------------------------------------
void DX11Texture2DContent::CopySubPart(
    IDeviceAPIEncapsulator *device,
    const DeviceAPIDependantTexture2D *dst, size_t dstLevel, const uint2& dstPos, 
    const DeviceAPIDependantTexture2D *src, size_t srcLevel, const AABB2u& srcBox ) {
    
    const size_t dstSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(dstLevel), 0, checked_cast<UINT>(dst->LevelCount()) );
    const uint3 dstPos3u(dstPos, 0);

    const DX11Texture2D *psourceDX11 = checked_cast<const DX11Texture2D *>(src);
    const size_t srcSubResource = ::D3D11CalcSubresource(checked_cast<UINT>(srcLevel), 0, checked_cast<UINT>(src->LevelCount()) );
    const AABB3u srcBox3u(uint3(srcBox.Min(), 0), uint3(srcBox.Max(), 1));

    DX11CopyResourceSubRegion(device, _texture.Get(), dstSubResource, dstPos3u, psourceDX11->Texture(), srcSubResource, srcBox3u);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11Texture2D::DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantTexture2D(device, owner, optionalData)
,   DX11Texture2DContent(device, owner, optionalData) {}
//----------------------------------------------------------------------------
DX11Texture2D::DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, const MemoryView<const u8>& optionalData, ::D3D11_BIND_FLAG bindFlags)
:   DeviceAPIDependantTexture2D(device, owner, optionalData)
,   DX11Texture2DContent(device, owner, optionalData, bindFlags) {}
//----------------------------------------------------------------------------
DX11Texture2D::DX11Texture2D(IDeviceAPIEncapsulator *device, Texture2D *owner, ::ID3D11Texture2D *texture, ::ID3D11ShaderResourceView *shaderView)
:   DeviceAPIDependantTexture2D(device, owner, MemoryView<const u8>())
,   DX11Texture2DContent(texture, shaderView) {}
//----------------------------------------------------------------------------
DX11Texture2D::~DX11Texture2D() {}
//----------------------------------------------------------------------------
void DX11Texture2D::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    DX11Texture2DContent::GetContent(device, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11Texture2D::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    DX11Texture2DContent::SetContent(device, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11Texture2D::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantTexture2D *psource) {
    DX11Texture2DContent::CopyFrom(device, psource);
}
//----------------------------------------------------------------------------
void DX11Texture2D::CopySubPart(
    IDeviceAPIEncapsulator *device,
    size_t dstLevel, const uint2& dstPos,
    const DeviceAPIDependantTexture2D *psource, size_t srcLevel, const AABB2u& srcBox ) {
    DX11Texture2DContent::CopySubPart(device, this, dstLevel, dstPos, psource, srcLevel, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11Texture2D, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
