#include "stdafx.h"

#include "DX11BlendState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11BlendState::DX11BlendState(IDeviceAPIEncapsulator *device, BlendState *owner)
:   DeviceAPIDependantBlendState(device, owner) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_BLEND_DESC blendStateDesc;
    ::SecureZeroMemory(&blendStateDesc, sizeof(blendStateDesc));

    blendStateDesc.AlphaToCoverageEnable = FALSE;
    blendStateDesc.IndependentBlendEnable = FALSE; // TODO ?

    blendStateDesc.RenderTarget[0].BlendEnable = owner->BlendEnabled();

    blendStateDesc.RenderTarget[0].SrcBlend = BlendToDX11Blend(owner->ColorSourceBlend());
    blendStateDesc.RenderTarget[0].DestBlend = BlendToDX11Blend(owner->ColorDestinationBlend());
    blendStateDesc.RenderTarget[0].BlendOp = BlendFunctionToDX11BlendOp(owner->ColorBlendFunction());

    blendStateDesc.RenderTarget[0].SrcBlendAlpha = BlendToDX11Blend(owner->AlphaSourceBlend());
    blendStateDesc.RenderTarget[0].DestBlendAlpha = BlendToDX11Blend(owner->AlphaDestinationBlend());
    blendStateDesc.RenderTarget[0].BlendOpAlpha = BlendFunctionToDX11BlendOp(owner->AlphaBlendFunction());
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = (UINT8)ColorChannelsToDX11ColorWriteEnable(owner->ColorWriteChannels());

    DX11_THROW_IF_FAILED(device, owner, (
        wrapper->Device()->CreateBlendState(&blendStateDesc, _entity.GetAddressOf())
        ));

    Assert(_entity);

    DX11SetDeviceResourceNameIFP(_entity, owner);
}
//----------------------------------------------------------------------------
DX11BlendState::~DX11BlendState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Graphics, DX11BlendState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND BlendToDX11Blend(Blend value) {
    switch (value)
    {
    case Core::Graphics::Blend::Zero:
        return D3D11_BLEND_ZERO;
    case Core::Graphics::Blend::One:
        return D3D11_BLEND_ONE;
    case Core::Graphics::Blend::SourceColor:
        return D3D11_BLEND_SRC_COLOR;
    case Core::Graphics::Blend::InverseSourceColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case Core::Graphics::Blend::SourceAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case Core::Graphics::Blend::InverseSourceAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case Core::Graphics::Blend::DestinationAlpha:
        return D3D11_BLEND_DEST_ALPHA;
    case Core::Graphics::Blend::InverseDestinationAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case Core::Graphics::Blend::DestinationColor:
        return D3D11_BLEND_DEST_COLOR;
    case Core::Graphics::Blend::InverseDestinationColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case Core::Graphics::Blend::SourceAlphaSaturation:
        return D3D11_BLEND_SRC_ALPHA_SAT;
    case Core::Graphics::Blend::BlendFactor:
        return D3D11_BLEND_BLEND_FACTOR;
    case Core::Graphics::Blend::InverseBlendFactor:
        return D3D11_BLEND_INV_BLEND_FACTOR;
    }
    AssertNotImplemented();
    return static_cast<D3D11_BLEND>(-1);
}
//----------------------------------------------------------------------------
Blend DX11BlendToBlend(::D3D11_BLEND value) {
    switch (value)
    {
    case D3D11_BLEND_ZERO:
        return Core::Graphics::Blend::Zero;
    case D3D11_BLEND_ONE:
        return Core::Graphics::Blend::One;
    case D3D11_BLEND_SRC_COLOR:
        return Core::Graphics::Blend::SourceColor;
    case D3D11_BLEND_INV_SRC_COLOR:
        return Core::Graphics::Blend::InverseSourceColor;
    case D3D11_BLEND_SRC_ALPHA:
        return Core::Graphics::Blend::SourceAlpha;
    case D3D11_BLEND_INV_SRC_ALPHA:
        return Core::Graphics::Blend::InverseSourceAlpha;
    case D3D11_BLEND_DEST_ALPHA:
        return Core::Graphics::Blend::DestinationAlpha;
    case D3D11_BLEND_INV_DEST_ALPHA:
        return Core::Graphics::Blend::InverseDestinationAlpha;
    case D3D11_BLEND_DEST_COLOR:
        return Core::Graphics::Blend::DestinationColor;
    case D3D11_BLEND_INV_DEST_COLOR:
        return Core::Graphics::Blend::InverseDestinationColor;
    case D3D11_BLEND_SRC_ALPHA_SAT:
        return Core::Graphics::Blend::SourceAlphaSaturation;
    case D3D11_BLEND_BLEND_FACTOR:
        return Core::Graphics::Blend::BlendFactor;
    case D3D11_BLEND_INV_BLEND_FACTOR:
        return Core::Graphics::Blend::InverseBlendFactor;
    default:
        AssertNotImplemented();
    }
    return static_cast<Blend>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND_OP BlendFunctionToDX11BlendOp(BlendFunction value) {
    switch (value)
    {
    case Core::Graphics::BlendFunction::Add:
        return D3D11_BLEND_OP_ADD;
    case Core::Graphics::BlendFunction::Max:
        return D3D11_BLEND_OP_MAX;
    case Core::Graphics::BlendFunction::Min:
        return D3D11_BLEND_OP_MIN;;
    case Core::Graphics::BlendFunction::ReverseSubstract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case Core::Graphics::BlendFunction::Substract:
        return D3D11_BLEND_OP_SUBTRACT;
    default:
        AssertNotImplemented();
    }
    return static_cast<D3D11_BLEND_OP>(-1);
}
//----------------------------------------------------------------------------
BlendFunction DX11BlendOpToBlendFunction(::D3D11_BLEND_OP value) {
    switch (value)
    {
    case D3D11_BLEND_OP_ADD:
        return Core::Graphics::BlendFunction::Add;
    case D3D11_BLEND_OP_MAX:
        return Core::Graphics::BlendFunction::Max;
    case D3D11_BLEND_OP_MIN:
        return Core::Graphics::BlendFunction::Min;
    case D3D11_BLEND_OP_REV_SUBTRACT:
        return Core::Graphics::BlendFunction::ReverseSubstract;
    case D3D11_BLEND_OP_SUBTRACT:
        return Core::Graphics::BlendFunction::Substract;
    default:
        AssertNotImplemented();
    }
    return static_cast<BlendFunction>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_RED)  == size_t(ColorChannels::Red));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_GREEN)== size_t(ColorChannels::Green));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_BLUE) == size_t(ColorChannels::Blue));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_ALPHA)== size_t(ColorChannels::Alpha));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_ALL)  == size_t(ColorChannels::All));
//----------------------------------------------------------------------------
::D3D11_COLOR_WRITE_ENABLE ColorChannelsToDX11ColorWriteEnable(ColorChannels value) {
    return static_cast<::D3D11_COLOR_WRITE_ENABLE>(value);
}
//----------------------------------------------------------------------------
ColorChannels DX11ColorWriteEnableToColorChannels(::D3D11_COLOR_WRITE_ENABLE value) {
    return static_cast<ColorChannels>(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
