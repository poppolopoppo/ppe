#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/State/BlendState.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11BlendState : public DeviceAPIDependantBlendState {
public:
    DX11BlendState(IDeviceAPIEncapsulator *device, BlendState *owner);
    virtual ~DX11BlendState();

    ::ID3D11BlendState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ComPtr<::ID3D11BlendState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND BlendToDX11Blend(Blend value);
//----------------------------------------------------------------------------
Blend DX11BlendToBlend(::D3D11_BLEND value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND_OP BlendFunctionToDX11BlendOp(BlendFunction value);
//----------------------------------------------------------------------------
BlendFunction DX11BlendOpToBlendFunction(::D3D11_BLEND_OP value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_COLOR_WRITE_ENABLE ColorChannelsToDX11ColorWriteEnable(ColorChannels value);
//----------------------------------------------------------------------------
ColorChannels DX11ColorWriteEnableToColorChannels(::D3D11_COLOR_WRITE_ENABLE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
