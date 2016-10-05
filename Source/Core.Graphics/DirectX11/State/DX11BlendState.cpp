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
FDX11BlendState::FDX11BlendState(IDeviceAPIEncapsulator *device, FBlendState *owner)
:   FDeviceAPIDependantBlendState(device, owner) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

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
FDX11BlendState::~FDX11BlendState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11BlendState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND BlendToDX11Blend(EBlend value) {
    switch (value)
    {
    case Core::Graphics::EBlend::Zero:
        return D3D11_BLEND_ZERO;
    case Core::Graphics::EBlend::One:
        return D3D11_BLEND_ONE;
    case Core::Graphics::EBlend::SourceColor:
        return D3D11_BLEND_SRC_COLOR;
    case Core::Graphics::EBlend::InverseSourceColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case Core::Graphics::EBlend::SourceAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case Core::Graphics::EBlend::InverseSourceAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case Core::Graphics::EBlend::DestinationAlpha:
        return D3D11_BLEND_DEST_ALPHA;
    case Core::Graphics::EBlend::InverseDestinationAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case Core::Graphics::EBlend::DestinationColor:
        return D3D11_BLEND_DEST_COLOR;
    case Core::Graphics::EBlend::InverseDestinationColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case Core::Graphics::EBlend::SourceAlphaSaturation:
        return D3D11_BLEND_SRC_ALPHA_SAT;
    case Core::Graphics::EBlend::BlendFactor:
        return D3D11_BLEND_BLEND_FACTOR;
    case Core::Graphics::EBlend::InverseBlendFactor:
        return D3D11_BLEND_INV_BLEND_FACTOR;
    }
    AssertNotImplemented();
    return static_cast<D3D11_BLEND>(-1);
}
//----------------------------------------------------------------------------
EBlend DX11BlendToBlend(::D3D11_BLEND value) {
    switch (value)
    {
    case D3D11_BLEND_ZERO:
        return Core::Graphics::EBlend::Zero;
    case D3D11_BLEND_ONE:
        return Core::Graphics::EBlend::One;
    case D3D11_BLEND_SRC_COLOR:
        return Core::Graphics::EBlend::SourceColor;
    case D3D11_BLEND_INV_SRC_COLOR:
        return Core::Graphics::EBlend::InverseSourceColor;
    case D3D11_BLEND_SRC_ALPHA:
        return Core::Graphics::EBlend::SourceAlpha;
    case D3D11_BLEND_INV_SRC_ALPHA:
        return Core::Graphics::EBlend::InverseSourceAlpha;
    case D3D11_BLEND_DEST_ALPHA:
        return Core::Graphics::EBlend::DestinationAlpha;
    case D3D11_BLEND_INV_DEST_ALPHA:
        return Core::Graphics::EBlend::InverseDestinationAlpha;
    case D3D11_BLEND_DEST_COLOR:
        return Core::Graphics::EBlend::DestinationColor;
    case D3D11_BLEND_INV_DEST_COLOR:
        return Core::Graphics::EBlend::InverseDestinationColor;
    case D3D11_BLEND_SRC_ALPHA_SAT:
        return Core::Graphics::EBlend::SourceAlphaSaturation;
    case D3D11_BLEND_BLEND_FACTOR:
        return Core::Graphics::EBlend::BlendFactor;
    case D3D11_BLEND_INV_BLEND_FACTOR:
        return Core::Graphics::EBlend::InverseBlendFactor;
    default:
        AssertNotImplemented();
    }
    return static_cast<EBlend>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BLEND_OP BlendFunctionToDX11BlendOp(EBlendFunction value) {
    switch (value)
    {
    case Core::Graphics::EBlendFunction::Add:
        return D3D11_BLEND_OP_ADD;
    case Core::Graphics::EBlendFunction::Max:
        return D3D11_BLEND_OP_MAX;
    case Core::Graphics::EBlendFunction::Min:
        return D3D11_BLEND_OP_MIN;;
    case Core::Graphics::EBlendFunction::ReverseSubtract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case Core::Graphics::EBlendFunction::Subtract:
        return D3D11_BLEND_OP_SUBTRACT;
    default:
        AssertNotImplemented();
    }
    return static_cast<D3D11_BLEND_OP>(-1);
}
//----------------------------------------------------------------------------
EBlendFunction DX11BlendOpToBlendFunction(::D3D11_BLEND_OP value) {
    switch (value)
    {
    case D3D11_BLEND_OP_ADD:
        return Core::Graphics::EBlendFunction::Add;
    case D3D11_BLEND_OP_MAX:
        return Core::Graphics::EBlendFunction::Max;
    case D3D11_BLEND_OP_MIN:
        return Core::Graphics::EBlendFunction::Min;
    case D3D11_BLEND_OP_REV_SUBTRACT:
        return Core::Graphics::EBlendFunction::ReverseSubtract;
    case D3D11_BLEND_OP_SUBTRACT:
        return Core::Graphics::EBlendFunction::Subtract;
    default:
        AssertNotImplemented();
    }
    return static_cast<EBlendFunction>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_RED)  == size_t(EColorChannels::Red));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_GREEN)== size_t(EColorChannels::Green));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_BLUE) == size_t(EColorChannels::Blue));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_ALPHA)== size_t(EColorChannels::Alpha));
STATIC_ASSERT(size_t(D3D11_COLOR_WRITE_ENABLE_ALL)  == size_t(EColorChannels::All));
//----------------------------------------------------------------------------
::D3D11_COLOR_WRITE_ENABLE ColorChannelsToDX11ColorWriteEnable(EColorChannels value) {
    return static_cast<::D3D11_COLOR_WRITE_ENABLE>(value);
}
//----------------------------------------------------------------------------
EColorChannels DX11ColorWriteEnableToColorChannels(::D3D11_COLOR_WRITE_ENABLE value) {
    return static_cast<EColorChannels>(value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
