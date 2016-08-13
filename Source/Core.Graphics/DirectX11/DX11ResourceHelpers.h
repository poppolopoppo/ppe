#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarRectangle_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
enum class BufferMode : u32;
enum class BufferUsage : u32;
enum class DeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11ResourceGetData(   IDeviceAPIEncapsulator *device,
                            ::ID3D11Resource *resource, size_t subResource,
                            size_t offset, void *const dst, size_t stride, size_t count,
                            BufferMode bufferMode,
                            BufferUsage bufferUsage );
//----------------------------------------------------------------------------
bool DX11ResourceSetData(   IDeviceAPIEncapsulator *device,
                            ::ID3D11Resource *resource, size_t subResource,
                            size_t offset, const void *src, size_t stride, size_t count,
                            BufferMode bufferMode,
                            BufferUsage bufferUsage );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11CopyResource(IDeviceAPIEncapsulator *device, ::ID3D11Resource *dst, ::ID3D11Resource *src);
//----------------------------------------------------------------------------
bool DX11CopyResourceSubRegion( IDeviceAPIEncapsulator *device,
                                ::ID3D11Resource *dst, size_t dstSubResource, const uint3& dstPos,
                                ::ID3D11Resource *src, size_t srcSubResource, const AABB3u& srcBox );

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, const void *src, size_t rowPitch, size_t depthPitch);
//----------------------------------------------------------------------------
bool DX11MapRead(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, void *const dst, size_t stride, size_t count);
//----------------------------------------------------------------------------
bool DX11MapWrite(::ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t subResource, size_t offset, const void *src, size_t stride, size_t count, bool discard, bool doNotWait);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
