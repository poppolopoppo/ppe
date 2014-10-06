#pragma once

#include "DX11Includes.h"

#include "BlendState.h"

#include "Core/ComPtr.h"
#include "Core/PoolAllocator.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BlendState : public DeviceAPIDependantBlendState {
public:
    BlendState(IDeviceAPIEncapsulator *device, Graphics::BlendState *owner);
    virtual ~BlendState();

    ::ID3D11BlendState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(BlendState);

private:
    ComPtr<::ID3D11BlendState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_BLEND BlendToDX11Blend(Graphics::Blend value);
//----------------------------------------------------------------------------
Graphics::Blend DX11BlendToBlend(D3D11_BLEND value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_BLEND_OP BlendFunctionToDX11BlendOp(Graphics::BlendFunction value);
//----------------------------------------------------------------------------
Graphics::BlendFunction DX11BlendOpToBlendFunction(D3D11_BLEND_OP value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COLOR_WRITE_ENABLE ColorChannelsToDX11ColorWriteEnable(Graphics::ColorChannels value);
//----------------------------------------------------------------------------
Graphics::ColorChannels DX11ColorWriteEnableToColorChannels(D3D11_COLOR_WRITE_ENABLE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
