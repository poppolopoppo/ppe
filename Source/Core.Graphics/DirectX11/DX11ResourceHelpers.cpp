#include "stdafx.h"

#include "DX11ResourceHelpers.h"

#include "DX11DeviceAPIEncapsulator.h"
#include "DX11ResourceBuffer.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResource.h"
#include "Device/DeviceResourceBuffer.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarRectangle.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11ResourceGetData(
    IDeviceAPIEncapsulator *device, 
    ::ID3D11Resource *resource, size_t subResource, 
    size_t offset, void *const dst, size_t stride, size_t count,
    BufferMode bufferMode,
    BufferUsage bufferUsage ) {
    AssertRelease(BufferUsage::Staging == bufferUsage);

    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    Assert(BufferUsage::Immutable != bufferUsage);
    Assert(Meta::HasFlag(bufferMode, BufferMode::Read) );

    if (!DX11MapRead(wrapper->ImmediateContext(), resource, subResource, offset, dst, stride, count))
        throw DeviceEncapsulatorException("DX11: failed to map resource buffer for reading", device);

    return true;
}
//----------------------------------------------------------------------------
bool DX11ResourceSetData(
    IDeviceAPIEncapsulator *device,
    ::ID3D11Resource *resource, size_t subResource,
    size_t offset, const void *src, size_t stride, size_t count,
    Graphics::BufferMode bufferMode,
    Graphics::BufferUsage bufferUsage ) {

    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    Assert(BufferUsage::Immutable != bufferUsage);

    const size_t rowPitch = 0; // TODO : RTFM ...
    const size_t depthPitch = 0; // TODO

    switch (bufferUsage)
    {
    case Core::Graphics::BufferUsage::Default:
        if (!DX11UpdateResource(wrapper->ImmediateContext(), resource, subResource, src, rowPitch, depthPitch))
            throw DeviceEncapsulatorException("DX11: failed to update resource buffer", device);
        break;

    case Core::Graphics::BufferUsage::Dynamic:
    case Core::Graphics::BufferUsage::Staging:
        Assert(Meta::HasFlag(bufferMode, BufferMode::Write) );
        if (!DX11MapWrite(  wrapper->ImmediateContext(), resource, subResource, offset, src, stride, count,
                            Meta::HasFlag(bufferMode, BufferMode::Discard),
                            Meta::HasFlag(bufferMode, BufferMode::DoNotWait) ))
            throw DeviceEncapsulatorException("DX11: failed to map resource buffer for writing", device);
        break;

    case Core::Graphics::BufferUsage::Immutable:
        throw DeviceEncapsulatorException("DX11: immutable buffer can't be muted", device);

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

    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    wrapper->ImmediateContext()->CopyResource(dst, src);
    return true;
}
//----------------------------------------------------------------------------
bool DX11CopyResourceSubRegion(
    IDeviceAPIEncapsulator *device,
    ::ID3D11Resource *dst, size_t dstSubResource, const uint3& dstPos, 
    ::ID3D11Resource *src, size_t srcSubResource, const AABB3u& srcBox ) {
    Assert(dst);
    Assert(src);

    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_BOX dx11SrcBox;
    
    dx11SrcBox.left = checked_cast<UINT>(srcBox.Min().x());
    dx11SrcBox.right = checked_cast<UINT>(srcBox.Max().x());

    dx11SrcBox.top = checked_cast<UINT>(srcBox.Min().y());
    dx11SrcBox.bottom = checked_cast<UINT>(srcBox.Max().z());

    dx11SrcBox.front = checked_cast<UINT>(srcBox.Min().z());
    dx11SrcBox.back = checked_cast<UINT>(srcBox.Max().z());

    wrapper->ImmediateContext()->CopySubresourceRegion(
        dst, checked_cast<UINT>(dstSubResource), 
        checked_cast<UINT>(dstPos.x()), checked_cast<UINT>(dstPos.y()), checked_cast<UINT>(dstPos.z()), 
        src, checked_cast<UINT>(srcSubResource), 
        &dx11SrcBox );

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, const void *src, size_t rowPitch, size_t depthPitch) {
    Assert(resource);
    Assert(IS_ALIGNED(16, src));

    deviceContext->UpdateSubresource(resource, checked_cast<UINT>(subResource), NULL, src, checked_cast<UINT>(rowPitch), checked_cast<UINT>(depthPitch));
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapRead(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, void *const dst, size_t stride, size_t count) {
    Assert(resource);
    Assert(IS_ALIGNED(16, dst));

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, checked_cast<UINT>(subResource), D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(dst, reinterpret_cast<const u8 *>(mappedResource.pData) + offset, stride * count);

    deviceContext->Unmap(resource, checked_cast<UINT>(subResource));
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const void *src, size_t stride, size_t count, bool discard, bool doNotWait) {
    Assert(resource);
    Assert(IS_ALIGNED(16, src));

    const ::D3D11_MAP mapType = (discard)
        ? ::D3D11_MAP_WRITE_DISCARD
        : ::D3D11_MAP_WRITE;

    const UINT mapFlags = (doNotWait)
        ? ::D3D11_MAP_FLAG_DO_NOT_WAIT
        : 0;

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, checked_cast<UINT>(subResource), mapType, mapFlags, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(reinterpret_cast<u8 *>(mappedResource.pData) + offset, src, stride * count);

    deviceContext->Unmap(resource, checked_cast<UINT>(subResource));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
