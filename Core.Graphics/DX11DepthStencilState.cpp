#include "stdafx.h"

#include "DX11DepthStencilState.h"

#include "DX11DeviceEncapsulator.h"

#include "DeviceAPIEncapsulator.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DepthStencilState::DepthStencilState(IDeviceAPIEncapsulator *device, Graphics::DepthStencilState *owner)
:   DeviceAPIDependantDepthStencilState(device, owner) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    ::D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ::SecureZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));

    depthStencilStateDesc.DepthEnable = owner->DepthBufferEnabled();
    depthStencilStateDesc.DepthWriteMask = owner->DepthBufferWriteEnabled() ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilStateDesc.DepthFunc = CompareFunctionToDX11ComparisonFunc(owner->DepthBufferFunction());

    depthStencilStateDesc.StencilEnable = owner->StencilEnabled();
    depthStencilStateDesc.StencilReadMask = checked_cast<UINT8>(owner->StencilReadMask());
    depthStencilStateDesc.StencilWriteMask = checked_cast<UINT8>(owner->StencilWriteMask());

    depthStencilStateDesc.FrontFace.StencilFailOp = StencilOperationToDX11StencilOp(owner->FrontFaceStencilFail());
    depthStencilStateDesc.FrontFace.StencilDepthFailOp = StencilOperationToDX11StencilOp(owner->FrontFaceStencilDepthBufferFail());
    depthStencilStateDesc.FrontFace.StencilPassOp = StencilOperationToDX11StencilOp(owner->FrontFaceStencilPass());
    depthStencilStateDesc.FrontFace.StencilFunc = CompareFunctionToDX11ComparisonFunc(owner->FrontFaceStencilFunction());

    depthStencilStateDesc.BackFace.StencilFailOp = StencilOperationToDX11StencilOp(owner->BackFaceStencilFail());
    depthStencilStateDesc.BackFace.StencilDepthFailOp = StencilOperationToDX11StencilOp(owner->BackFaceStencilDepthBufferFail());
    depthStencilStateDesc.BackFace.StencilPassOp = StencilOperationToDX11StencilOp(owner->BackFaceStencilPass());
    depthStencilStateDesc.BackFace.StencilFunc = CompareFunctionToDX11ComparisonFunc(owner->BackFaceStencilFunction());

    DX11_THROW_IF_FAILED(device, owner, (
        wrapper->Device()->CreateDepthStencilState(&depthStencilStateDesc, _entity.GetAddressOf())
        ));

    Assert(_entity);

    DX11SetDeviceResourceNameIFP(_entity, owner);
}
//----------------------------------------------------------------------------
DepthStencilState::~DepthStencilState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(DepthStencilState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COMPARISON_FUNC CompareFunctionToDX11ComparisonFunc(Graphics::CompareFunction value) {
    switch (value)
    {
    case Core::Graphics::CompareFunction::Always:
        return D3D11_COMPARISON_ALWAYS;
    case Core::Graphics::CompareFunction::Equal:
        return D3D11_COMPARISON_EQUAL;
    case Core::Graphics::CompareFunction::Greater:
        return D3D11_COMPARISON_GREATER;
    case Core::Graphics::CompareFunction::GreaterEqual:
        return D3D11_COMPARISON_GREATER_EQUAL;
    case Core::Graphics::CompareFunction::Less:
        return D3D11_COMPARISON_LESS;
    case Core::Graphics::CompareFunction::LessEqual:
        return D3D11_COMPARISON_LESS_EQUAL;
    case Core::Graphics::CompareFunction::Never:
        return D3D11_COMPARISON_NEVER;
    case Core::Graphics::CompareFunction::NotEqual:
        return D3D11_COMPARISON_NOT_EQUAL;
    }
    AssertNotImplemented();
    return static_cast<D3D11_COMPARISON_FUNC>(-1);
}
//----------------------------------------------------------------------------
Graphics::CompareFunction DX11ComparisonFuncToCompareFunction(D3D11_COMPARISON_FUNC value) {
    switch (value)
    {
    case D3D11_COMPARISON_ALWAYS:
        return Core::Graphics::CompareFunction::Always;
    case D3D11_COMPARISON_EQUAL:
        return Core::Graphics::CompareFunction::Equal;
    case D3D11_COMPARISON_GREATER:
        return Core::Graphics::CompareFunction::Greater;
    case D3D11_COMPARISON_GREATER_EQUAL:
        return Core::Graphics::CompareFunction::GreaterEqual;
    case D3D11_COMPARISON_LESS:
        return Core::Graphics::CompareFunction::Less;
    case D3D11_COMPARISON_LESS_EQUAL:
        return Core::Graphics::CompareFunction::LessEqual;
    case D3D11_COMPARISON_NEVER:
        return Core::Graphics::CompareFunction::Never;
    case D3D11_COMPARISON_NOT_EQUAL:
        return Core::Graphics::CompareFunction::NotEqual;
    }
    AssertNotImplemented();
    return static_cast<Graphics::CompareFunction>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_STENCIL_OP StencilOperationToDX11StencilOp(Graphics::StencilOperation value) {
    switch (value)
    {
    case Core::Graphics::StencilOperation::Decrement:
        return D3D11_STENCIL_OP_DECR;
    case Core::Graphics::StencilOperation::DecrementSaturation:
        return D3D11_STENCIL_OP_DECR_SAT;
    case Core::Graphics::StencilOperation::Increment:
        return D3D11_STENCIL_OP_INCR;
    case Core::Graphics::StencilOperation::IncrementSaturation:
        return D3D11_STENCIL_OP_INCR_SAT;
    case Core::Graphics::StencilOperation::Invert:
        return D3D11_STENCIL_OP_INVERT;
    case Core::Graphics::StencilOperation::Keep:
        return D3D11_STENCIL_OP_KEEP;
    case Core::Graphics::StencilOperation::Replace:
        return D3D11_STENCIL_OP_REPLACE;
    case Core::Graphics::StencilOperation::Zero:
        return D3D11_STENCIL_OP_ZERO;
    }
    AssertNotImplemented();
    return static_cast<D3D11_STENCIL_OP>(-1);
}
//----------------------------------------------------------------------------
Graphics::StencilOperation DX11StencilOpToStencilOperation(D3D11_STENCIL_OP value) {
    switch (value)
    {
    case D3D11_STENCIL_OP_DECR:
        return Core::Graphics::StencilOperation::Decrement;
    case D3D11_STENCIL_OP_DECR_SAT:
        return Core::Graphics::StencilOperation::DecrementSaturation;
    case D3D11_STENCIL_OP_INCR:
        return Core::Graphics::StencilOperation::Increment;
    case D3D11_STENCIL_OP_INCR_SAT:
        return Core::Graphics::StencilOperation::IncrementSaturation;
    case D3D11_STENCIL_OP_INVERT:
        return Core::Graphics::StencilOperation::Invert;
    case D3D11_STENCIL_OP_KEEP:
        return Core::Graphics::StencilOperation::Keep;
    case D3D11_STENCIL_OP_REPLACE:
        return Core::Graphics::StencilOperation::Replace;
    case D3D11_STENCIL_OP_ZERO:
        return Core::Graphics::StencilOperation::Zero;
    }
    AssertNotImplemented();
    return static_cast<Graphics::StencilOperation>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
