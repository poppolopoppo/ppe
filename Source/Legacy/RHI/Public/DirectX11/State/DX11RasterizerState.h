#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/State/RasterizerState.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11RasterizerState : public FDeviceAPIDependantRasterizerState {
public:
    FDX11RasterizerState(IDeviceAPIEncapsulator *device, FRasterizerState *owner);
    virtual ~FDX11RasterizerState();

    ::ID3D11RasterizerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11RasterizerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CULL_MODE CullModeToDX11CullMode(ECullMode value);
//----------------------------------------------------------------------------
ECullMode DX11CullModeToCullMode(D3D11_CULL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILL_MODE FillModeToDX11FillMode(EFillMode value);
//----------------------------------------------------------------------------
EFillMode DX11FillModeToFillMode(D3D11_FILL_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
