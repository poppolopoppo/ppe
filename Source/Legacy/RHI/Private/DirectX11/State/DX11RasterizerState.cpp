﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11RasterizerState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11RasterizerState::FDX11RasterizerState(IDeviceAPIEncapsulator *device, FRasterizerState *owner)
:   FDeviceAPIDependantRasterizerState(device, owner) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_RASTERIZER_DESC rasterizerStateDesc;
    ::SecureZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));

    rasterizerStateDesc.FillMode = FillModeToDX11FillMode(owner->FillMode());

    rasterizerStateDesc.FrontCounterClockwise = TRUE; // defaulted
    rasterizerStateDesc.CullMode = CullModeToDX11CullMode(owner->CullMode());

    Assert(0 <= owner->DepthBias());
    Assert(16 >= owner->DepthBias());

    rasterizerStateDesc.DepthBias = checked_cast<INT>(owner->DepthBias());
    rasterizerStateDesc.SlopeScaledDepthBias = owner->SlopeScaledDepthBias();
    rasterizerStateDesc.DepthBiasClamp = 0;
    rasterizerStateDesc.DepthClipEnable = TRUE;

    rasterizerStateDesc.MultisampleEnable = owner->MultiSampleAntiAlias();
    rasterizerStateDesc.AntialiasedLineEnable = owner->MultiSampleAntiAlias();

    DX11_THROW_IF_FAILED(device, owner, (
        wrapper->Device()->CreateRasterizerState(&rasterizerStateDesc, _entity.GetAddressOf())
        ));

    Assert(_entity);

    DX11SetDeviceResourceNameIFP(_entity.Get(), owner);
}
//----------------------------------------------------------------------------
FDX11RasterizerState::~FDX11RasterizerState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11RasterizerState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CULL_MODE CullModeToDX11CullMode(ECullMode value) {
    switch (value)
    {
    case PPE::Graphics::ECullMode::CullClockwiseFace:
        return D3D11_CULL_BACK;
    case PPE::Graphics::ECullMode::CullCounterClockwiseFace:
        return D3D11_CULL_FRONT;
    case PPE::Graphics::ECullMode::None:
        return D3D11_CULL_NONE;
    }
    AssertNotImplemented();
    return static_cast<D3D11_CULL_MODE>(-1);
}
//----------------------------------------------------------------------------
ECullMode DX11CullModeToCullMode(D3D11_CULL_MODE value) {
    switch (value)
    {
    case D3D11_CULL_BACK:
        return PPE::Graphics::ECullMode::CullClockwiseFace;
    case D3D11_CULL_FRONT:
        return PPE::Graphics::ECullMode::CullCounterClockwiseFace;
    case D3D11_CULL_NONE:
        return PPE::Graphics::ECullMode::None;
    }
    AssertNotImplemented();
    return static_cast<Graphics::ECullMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILL_MODE FillModeToDX11FillMode(EFillMode value) {
    switch (value)
    {
    case PPE::Graphics::EFillMode::Solid:
        return D3D11_FILL_SOLID;
    case PPE::Graphics::EFillMode::WireFrame:
        return D3D11_FILL_WIREFRAME;
    }
    AssertNotImplemented();
    return static_cast<D3D11_FILL_MODE>(-1);
}
//----------------------------------------------------------------------------
EFillMode DX11FillModeToFillMode(D3D11_FILL_MODE value) {
    switch (value)
    {
    case D3D11_FILL_SOLID:
        return PPE::Graphics::EFillMode::Solid;
    case D3D11_FILL_WIREFRAME:
        return PPE::Graphics::EFillMode::WireFrame;
    }
    AssertNotImplemented();
    return static_cast<Graphics::EFillMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
