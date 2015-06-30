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
class DX11DepthStencilState : public DeviceAPIDependantDepthStencilState {
public:
    DX11DepthStencilState(IDeviceAPIEncapsulator *device, DepthStencilState *owner);
    virtual ~DX11DepthStencilState();

    ::ID3D11DepthStencilState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(DX11DepthStencilState);

private:
    ComPtr<::ID3D11DepthStencilState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COMPARISON_FUNC CompareFunctionToDX11ComparisonFunc(CompareFunction value);
//----------------------------------------------------------------------------
CompareFunction DX11ComparisonFuncToCompareFunction(D3D11_COMPARISON_FUNC value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_STENCIL_OP StencilOperationToDX11StencilOp(StencilOperation value);
//----------------------------------------------------------------------------
StencilOperation DX11StencilOpToStencilOperation(D3D11_STENCIL_OP value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
