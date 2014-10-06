#pragma once

#include "DX11Includes.h"

#include "DepthStencilState.h"

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
class DepthStencilState : public DeviceAPIDependantDepthStencilState {
public:
    DepthStencilState(IDeviceAPIEncapsulator *device, Graphics::DepthStencilState *owner);
    virtual ~DepthStencilState();

    ::ID3D11DepthStencilState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(DepthStencilState);

private:
    ComPtr<::ID3D11DepthStencilState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COMPARISON_FUNC CompareFunctionToDX11ComparisonFunc(Graphics::CompareFunction value);
//----------------------------------------------------------------------------
Graphics::CompareFunction DX11ComparisonFuncToCompareFunction(D3D11_COMPARISON_FUNC value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_STENCIL_OP StencilOperationToDX11StencilOp(Graphics::StencilOperation value);
//----------------------------------------------------------------------------
Graphics::StencilOperation DX11StencilOpToStencilOperation(D3D11_STENCIL_OP value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
