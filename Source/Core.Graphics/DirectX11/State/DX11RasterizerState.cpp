#include "stdafx.h"

#include "DX11RasterizerState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11RasterizerState::DX11RasterizerState(IDeviceAPIEncapsulator *device, RasterizerState *owner)
:   DeviceAPIDependantRasterizerState(device, owner) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

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

    DX11SetDeviceResourceNameIFP(_entity, owner);
}
//----------------------------------------------------------------------------
DX11RasterizerState::~DX11RasterizerState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Graphics, DX11RasterizerState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CULL_MODE CullModeToDX11CullMode(CullMode value) {
    switch (value)
    {
    case Core::Graphics::CullMode::CullClockwiseFace:
        return D3D11_CULL_BACK;
    case Core::Graphics::CullMode::CullCounterClockwiseFace:
        return D3D11_CULL_FRONT;
    case Core::Graphics::CullMode::None:
        return D3D11_CULL_NONE;
    }
    AssertNotImplemented();
    return static_cast<D3D11_CULL_MODE>(-1);
}
//----------------------------------------------------------------------------
CullMode DX11CullModeToCullMode(D3D11_CULL_MODE value) {
    switch (value)
    {
    case D3D11_CULL_BACK:
        return Core::Graphics::CullMode::CullClockwiseFace;
    case D3D11_CULL_FRONT:
        return Core::Graphics::CullMode::CullCounterClockwiseFace;
    case D3D11_CULL_NONE:
        return Core::Graphics::CullMode::None;
    }
    AssertNotImplemented();
    return static_cast<Graphics::CullMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILL_MODE FillModeToDX11FillMode(FillMode value) {
    switch (value)
    {
    case Core::Graphics::FillMode::Solid:
        return D3D11_FILL_SOLID;
    case Core::Graphics::FillMode::WireFrame:
        return D3D11_FILL_WIREFRAME;
    }
    AssertNotImplemented();
    return static_cast<D3D11_FILL_MODE>(-1);
}
//----------------------------------------------------------------------------
FillMode DX11FillModeToFillMode(D3D11_FILL_MODE value) {
    switch (value)
    {
    case D3D11_FILL_SOLID:
        return Core::Graphics::FillMode::Solid;
    case D3D11_FILL_WIREFRAME:
        return Core::Graphics::FillMode::WireFrame;
    }
    AssertNotImplemented();
    return static_cast<Graphics::FillMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
