#pragma once

#include "DirectX11/DX11Includes.h"

#include "Maths/ScalarBoundingBox_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class EBufferMode : u32;
enum class EBufferUsage : u32;
enum class EDeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11ResourceGetData(   IDeviceAPIEncapsulator *device,
                            ::ID3D11Resource *resource, size_t subResource,
                            size_t offset, const TMemoryView<u8>& dst,
                            EBufferMode bufferMode,
                            EBufferUsage bufferUsage );
//----------------------------------------------------------------------------
bool DX11ResourceSetData(   IDeviceAPIEncapsulator *device,
                            ::ID3D11Resource *resource, size_t subResource,
                            size_t offset, const TMemoryView<const u8>& src,
                            EBufferMode bufferMode,
                            EBufferUsage bufferUsage );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11CopyResource(IDeviceAPIEncapsulator *device, ::ID3D11Resource *dst, ::ID3D11Resource *src);
//----------------------------------------------------------------------------
bool DX11CopyResourceSubRegion( IDeviceAPIEncapsulator *device,
                                ::ID3D11Resource *dst, size_t dstSubResource, const uint3& dstPos,
                                ::ID3D11Resource *src, size_t srcSubResource, const FAabb3u& srcBox );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, const void *src, size_t rowPitch, size_t depthPitch);
//----------------------------------------------------------------------------
bool DX11MapRead(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<u8>& dst);
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait);
//----------------------------------------------------------------------------
bool DX11MapWriteDiscard(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait);
//----------------------------------------------------------------------------
bool DX11MapWriteNoOverwrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, bool doNotWait);
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const TMemoryView<const u8>& src, EBufferMode bufferMode);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
