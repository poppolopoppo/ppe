#include "stdafx.h"

#include "DX11ResourceHelpers.h"

#include "DX11DeviceAPIEncapsulator.h"
#include "DX11ResourceBuffer.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResource.h"
#include "Device/DeviceResourceBuffer.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarRectangle.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool DX11MapWrite_(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, ::D3D11_MAP mapType, bool doNotWait) {
    Assert(resource);
    Assert(Meta::IsAligned(GRAPHICS_BOUNDARY, src.data()));

    const ::UINT mapFlags = (doNotWait ? ::D3D11_MAP_FLAG_DO_NOT_WAIT : 0);

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, checked_cast<::UINT>(subResource), mapType, mapFlags, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(reinterpret_cast<u8 *>(mappedResource.pData) + offset, src.data(), src.size());

    deviceContext->Unmap(resource, checked_cast<::UINT>(subResource));
    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11ResourceGetData(
    IDeviceAPIEncapsulator *device,
    ::ID3D11Resource *resource, size_t subResource,
    size_t offset, const TMemoryView<u8>& dst,
    EBufferMode bufferMode,
    EBufferUsage bufferUsage ) {
    AssertRelease(EBufferUsage::Staging == bufferUsage);
    Assert(EBufferUsage::Immutable != bufferUsage);
    Assert(bufferMode ^ EBufferMode::Read);
    UNUSED(bufferMode);
    UNUSED(bufferUsage);

    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    if (!DX11MapRead(wrapper->ImmediateContext(), resource, subResource, offset, dst))
        PPE_THROW_IT(FDeviceEncapsulatorException("DX11: failed to map resource buffer for reading", device));

    return true;
}
//----------------------------------------------------------------------------
bool DX11ResourceSetData(
    IDeviceAPIEncapsulator *device,
    ::ID3D11Resource *resource, size_t subResource,
    size_t offset, const TMemoryView<const u8>& src,
    Graphics::EBufferMode bufferMode,
    Graphics::EBufferUsage bufferUsage ) {
    Assert(EBufferUsage::Immutable != bufferUsage);

    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    const size_t rowPitch = 0; // TODO : RTFM ...
    const size_t depthPitch = 0; // TODO

    switch (bufferUsage)
    {
    case PPE::Graphics::EBufferUsage::Default:
        if (!DX11UpdateResource(wrapper->ImmediateContext(), resource, subResource, src.data(), rowPitch, depthPitch))
            PPE_THROW_IT(FDeviceEncapsulatorException("DX11: failed to update resource buffer", device));
        break;

    case PPE::Graphics::EBufferUsage::Dynamic:
    case PPE::Graphics::EBufferUsage::Staging:
        Assert(bufferMode ^ EBufferMode::Write);
        if (!DX11MapWrite(wrapper->ImmediateContext(), resource, subResource, offset, src, bufferMode))
            PPE_THROW_IT(FDeviceEncapsulatorException("DX11: failed to map resource buffer for writing", device));
        break;

    case PPE::Graphics::EBufferUsage::Immutable:
        PPE_THROW_IT(FDeviceEncapsulatorException("DX11: immutable buffer can't be muted", device));

    default:
        AssertNotImplemented();
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11CopyResource(IDeviceAPIEncapsulator *device, ::ID3D11Resource *dst, ::ID3D11Resource *src) {
    Assert(dst);
    Assert(src);

    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    wrapper->ImmediateContext()->CopyResource(dst, src);
    return true;
}
//----------------------------------------------------------------------------
bool DX11CopyResourceSubRegion(
    IDeviceAPIEncapsulator *device,
    ::ID3D11Resource *dst, size_t dstSubResource, const uint3& dstPos,
    ::ID3D11Resource *src, size_t srcSubResource, const FAabb3u& srcBox ) {
    Assert(dst);
    Assert(src);

    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_BOX dx11SrcBox;

    dx11SrcBox.left = checked_cast<::UINT>(srcBox.Min().x());
    dx11SrcBox.right = checked_cast<::UINT>(srcBox.Max().x());

    dx11SrcBox.top = checked_cast<::UINT>(srcBox.Min().y());
    dx11SrcBox.bottom = checked_cast<::UINT>(srcBox.Max().z());

    dx11SrcBox.front = checked_cast<::UINT>(srcBox.Min().z());
    dx11SrcBox.back = checked_cast<::UINT>(srcBox.Max().z());

    wrapper->ImmediateContext()->CopySubresourceRegion(
        dst, checked_cast<::UINT>(dstSubResource),
        checked_cast<::UINT>(dstPos.x()), checked_cast<::UINT>(dstPos.y()), checked_cast<::UINT>(dstPos.z()),
        src, checked_cast<::UINT>(srcSubResource),
        &dx11SrcBox );

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, const void *src, size_t rowPitch, size_t depthPitch) {
    Assert(resource);
    Assert(Meta::IsAligned(GRAPHICS_BOUNDARY, src));

    deviceContext->UpdateSubresource(resource, checked_cast<::UINT>(subResource), NULL, src, checked_cast<::UINT>(rowPitch), checked_cast<::UINT>(depthPitch));
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapRead(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<u8>& dst) {
    Assert(resource);
    Assert(Meta::IsAligned(GRAPHICS_BOUNDARY, dst.data()));

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, checked_cast<::UINT>(subResource), D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(dst.data(), reinterpret_cast<const u8 *>(mappedResource.pData) + offset, dst.size());

    deviceContext->Unmap(resource, checked_cast<::UINT>(subResource));
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait) {
    return DX11MapWrite_(deviceContext, resource, subResource, offset, src, ::D3D11_MAP_WRITE, doNotWait);
}
//----------------------------------------------------------------------------
bool DX11MapWriteDiscard(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait) {
    return DX11MapWrite_(deviceContext, resource, subResource, offset, src, ::D3D11_MAP_WRITE_DISCARD, doNotWait);
}
//----------------------------------------------------------------------------
bool DX11MapWriteNoOverwrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait) {
    return DX11MapWrite_(deviceContext, resource, subResource, offset, src, ::D3D11_MAP_WRITE_NO_OVERWRITE, doNotWait);
}
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, EBufferMode bufferMode) {
    Assert(EBufferMode::Write ^ bufferMode);

    ::D3D11_MAP mapType;
    if (bufferMode == EBufferMode::WriteDiscard)
        mapType = ::D3D11_MAP_WRITE_DISCARD;
    else if (bufferMode == EBufferMode::WriteNoOverwrite)
        mapType = ::D3D11_MAP_WRITE_NO_OVERWRITE;
    else
        mapType = ::D3D11_MAP_WRITE;

    const bool doNotWait = (EBufferMode::DoNotWait ^ bufferMode);
    return DX11MapWrite_(deviceContext, resource, subResource, offset, src, mapType, doNotWait);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
