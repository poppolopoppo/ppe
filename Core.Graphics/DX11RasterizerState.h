#pragma once

#include "DX11Includes.h"

#include "RasterizerState.h"

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
class RasterizerState : public DeviceAPIDependantRasterizerState {
public:
    RasterizerState(IDeviceAPIEncapsulator *device, Graphics::RasterizerState *owner);
    virtual ~RasterizerState();

    ::ID3D11RasterizerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(RasterizerState);

private:
    ComPtr<::ID3D11RasterizerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CULL_MODE CullModeToDX11CullMode(Graphics::CullMode value);
//----------------------------------------------------------------------------
Graphics::CullMode DX11CullModeToCullMode(D3D11_CULL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILL_MODE FillModeToDX11FillMode(Graphics::FillMode value);
//----------------------------------------------------------------------------
Graphics::FillMode DX11FillModeToFillMode(D3D11_FILL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
