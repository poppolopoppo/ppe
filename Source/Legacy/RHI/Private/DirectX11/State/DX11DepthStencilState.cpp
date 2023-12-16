// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11DepthStencilState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11DepthStencilState::FDX11DepthStencilState(IDeviceAPIEncapsulator *device, FDepthStencilState *owner)
:   FDeviceAPIDependantDepthStencilState(device, owner) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

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

    DX11SetDeviceResourceNameIFP(_entity.Get(), owner);
}
//----------------------------------------------------------------------------
FDX11DepthStencilState::~FDX11DepthStencilState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11DepthStencilState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_COMPARISON_FUNC CompareFunctionToDX11ComparisonFunc(ECompareFunction value) {
    switch (value)
    {
    case PPE::Graphics::ECompareFunction::Always:
        return D3D11_COMPARISON_ALWAYS;
    case PPE::Graphics::ECompareFunction::Equal:
        return D3D11_COMPARISON_EQUAL;
    case PPE::Graphics::ECompareFunction::TGreater:
        return D3D11_COMPARISON_GREATER;
    case PPE::Graphics::ECompareFunction::TGreaterEqual:
        return D3D11_COMPARISON_GREATER_EQUAL;
    case PPE::Graphics::ECompareFunction::TLess:
        return D3D11_COMPARISON_LESS;
    case PPE::Graphics::ECompareFunction::TLessEqual:
        return D3D11_COMPARISON_LESS_EQUAL;
    case PPE::Graphics::ECompareFunction::Never:
        return D3D11_COMPARISON_NEVER;
    case PPE::Graphics::ECompareFunction::NotEqual:
        return D3D11_COMPARISON_NOT_EQUAL;
    }
    AssertNotImplemented();
    return static_cast<D3D11_COMPARISON_FUNC>(-1);
}
//----------------------------------------------------------------------------
ECompareFunction DX11ComparisonFuncToCompareFunction(D3D11_COMPARISON_FUNC value) {
    switch (value)
    {
    case D3D11_COMPARISON_ALWAYS:
        return PPE::Graphics::ECompareFunction::Always;
    case D3D11_COMPARISON_EQUAL:
        return PPE::Graphics::ECompareFunction::Equal;
    case D3D11_COMPARISON_GREATER:
        return PPE::Graphics::ECompareFunction::TGreater;
    case D3D11_COMPARISON_GREATER_EQUAL:
        return PPE::Graphics::ECompareFunction::TGreaterEqual;
    case D3D11_COMPARISON_LESS:
        return PPE::Graphics::ECompareFunction::TLess;
    case D3D11_COMPARISON_LESS_EQUAL:
        return PPE::Graphics::ECompareFunction::TLessEqual;
    case D3D11_COMPARISON_NEVER:
        return PPE::Graphics::ECompareFunction::Never;
    case D3D11_COMPARISON_NOT_EQUAL:
        return PPE::Graphics::ECompareFunction::NotEqual;
    }
    AssertNotImplemented();
    return static_cast<Graphics::ECompareFunction>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_STENCIL_OP StencilOperationToDX11StencilOp(EStencilOperation value) {
    switch (value)
    {
    case PPE::Graphics::EStencilOperation::Decrement:
        return D3D11_STENCIL_OP_DECR;
    case PPE::Graphics::EStencilOperation::DecrementSaturation:
        return D3D11_STENCIL_OP_DECR_SAT;
    case PPE::Graphics::EStencilOperation::Increment:
        return D3D11_STENCIL_OP_INCR;
    case PPE::Graphics::EStencilOperation::IncrementSaturation:
        return D3D11_STENCIL_OP_INCR_SAT;
    case PPE::Graphics::EStencilOperation::Invert:
        return D3D11_STENCIL_OP_INVERT;
    case PPE::Graphics::EStencilOperation::Keep:
        return D3D11_STENCIL_OP_KEEP;
    case PPE::Graphics::EStencilOperation::Replace:
        return D3D11_STENCIL_OP_REPLACE;
    case PPE::Graphics::EStencilOperation::Zero:
        return D3D11_STENCIL_OP_ZERO;
    }
    AssertNotImplemented();
    return static_cast<D3D11_STENCIL_OP>(-1);
}
//----------------------------------------------------------------------------
EStencilOperation DX11StencilOpToStencilOperation(D3D11_STENCIL_OP value) {
    switch (value)
    {
    case D3D11_STENCIL_OP_DECR:
        return PPE::Graphics::EStencilOperation::Decrement;
    case D3D11_STENCIL_OP_DECR_SAT:
        return PPE::Graphics::EStencilOperation::DecrementSaturation;
    case D3D11_STENCIL_OP_INCR:
        return PPE::Graphics::EStencilOperation::Increment;
    case D3D11_STENCIL_OP_INCR_SAT:
        return PPE::Graphics::EStencilOperation::IncrementSaturation;
    case D3D11_STENCIL_OP_INVERT:
        return PPE::Graphics::EStencilOperation::Invert;
    case D3D11_STENCIL_OP_KEEP:
        return PPE::Graphics::EStencilOperation::Keep;
    case D3D11_STENCIL_OP_REPLACE:
        return PPE::Graphics::EStencilOperation::Replace;
    case D3D11_STENCIL_OP_ZERO:
        return PPE::Graphics::EStencilOperation::Zero;
    }
    AssertNotImplemented();
    return static_cast<Graphics::EStencilOperation>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
