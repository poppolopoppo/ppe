#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/State/DepthStencilState.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11DepthStencilState : public FDeviceAPIDependantDepthStencilState {
public:
    FDX11DepthStencilState(IDeviceAPIEncapsulator *device, FDepthStencilState *owner);
    virtual ~FDX11DepthStencilState();

    ::ID3D11DepthStencilState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11DepthStencilState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COMPARISON_FUNC CompareFunctionToDX11ComparisonFunc(ECompareFunction value);
//----------------------------------------------------------------------------
ECompareFunction DX11ComparisonFuncToCompareFunction(D3D11_COMPARISON_FUNC value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_STENCIL_OP StencilOperationToDX11StencilOp(EStencilOperation value);
//----------------------------------------------------------------------------
EStencilOperation DX11StencilOpToStencilOperation(D3D11_STENCIL_OP value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
