#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/State/RasterizerState.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11RasterizerState : public DeviceAPIDependantRasterizerState {
public:
    DX11RasterizerState(IDeviceAPIEncapsulator *device, RasterizerState *owner);
    virtual ~DX11RasterizerState();

    ::ID3D11RasterizerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ComPtr<::ID3D11RasterizerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CULL_MODE CullModeToDX11CullMode(CullMode value);
//----------------------------------------------------------------------------
CullMode DX11CullModeToCullMode(D3D11_CULL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILL_MODE FillModeToDX11FillMode(FillMode value);
//----------------------------------------------------------------------------
FillMode DX11FillModeToFillMode(D3D11_FILL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
