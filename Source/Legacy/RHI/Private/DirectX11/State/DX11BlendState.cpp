// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11BlendState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
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

    DX11SetDeviceResourceNameIFP(_entity.Get(), owner);
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
    case PPE::Graphics::EBlend::Zero:
        return D3D11_BLEND_ZERO;
    case PPE::Graphics::EBlend::One:
        return D3D11_BLEND_ONE;
    case PPE::Graphics::EBlend::SourceColor:
        return D3D11_BLEND_SRC_COLOR;
    case PPE::Graphics::EBlend::InverseSourceColor:
        return D3D11_BLEND_INV_SRC_COLOR;
    case PPE::Graphics::EBlend::SourceAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case PPE::Graphics::EBlend::InverseSourceAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case PPE::Graphics::EBlend::DestinationAlpha:
        return D3D11_BLEND_DEST_ALPHA;
    case PPE::Graphics::EBlend::InverseDestinationAlpha:
        return D3D11_BLEND_INV_DEST_ALPHA;
    case PPE::Graphics::EBlend::DestinationColor:
        return D3D11_BLEND_DEST_COLOR;
    case PPE::Graphics::EBlend::InverseDestinationColor:
        return D3D11_BLEND_INV_DEST_COLOR;
    case PPE::Graphics::EBlend::SourceAlphaSaturation:
        return D3D11_BLEND_SRC_ALPHA_SAT;
    case PPE::Graphics::EBlend::BlendFactor:
        return D3D11_BLEND_BLEND_FACTOR;
    case PPE::Graphics::EBlend::InverseBlendFactor:
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
        return PPE::Graphics::EBlend::Zero;
    case D3D11_BLEND_ONE:
        return PPE::Graphics::EBlend::One;
    case D3D11_BLEND_SRC_COLOR:
        return PPE::Graphics::EBlend::SourceColor;
    case D3D11_BLEND_INV_SRC_COLOR:
        return PPE::Graphics::EBlend::InverseSourceColor;
    case D3D11_BLEND_SRC_ALPHA:
        return PPE::Graphics::EBlend::SourceAlpha;
    case D3D11_BLEND_INV_SRC_ALPHA:
        return PPE::Graphics::EBlend::InverseSourceAlpha;
    case D3D11_BLEND_DEST_ALPHA:
        return PPE::Graphics::EBlend::DestinationAlpha;
    case D3D11_BLEND_INV_DEST_ALPHA:
        return PPE::Graphics::EBlend::InverseDestinationAlpha;
    case D3D11_BLEND_DEST_COLOR:
        return PPE::Graphics::EBlend::DestinationColor;
    case D3D11_BLEND_INV_DEST_COLOR:
        return PPE::Graphics::EBlend::InverseDestinationColor;
    case D3D11_BLEND_SRC_ALPHA_SAT:
        return PPE::Graphics::EBlend::SourceAlphaSaturation;
    case D3D11_BLEND_BLEND_FACTOR:
        return PPE::Graphics::EBlend::BlendFactor;
    case D3D11_BLEND_INV_BLEND_FACTOR:
        return PPE::Graphics::EBlend::InverseBlendFactor;
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
    case PPE::Graphics::EBlendFunction::Add:
        return D3D11_BLEND_OP_ADD;
    case PPE::Graphics::EBlendFunction::Max:
        return D3D11_BLEND_OP_MAX;
    case PPE::Graphics::EBlendFunction::Min:
        return D3D11_BLEND_OP_MIN;;
    case PPE::Graphics::EBlendFunction::ReverseSubtract:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case PPE::Graphics::EBlendFunction::Subtract:
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
        return PPE::Graphics::EBlendFunction::Add;
    case D3D11_BLEND_OP_MAX:
        return PPE::Graphics::EBlendFunction::Max;
    case D3D11_BLEND_OP_MIN:
        return PPE::Graphics::EBlendFunction::Min;
    case D3D11_BLEND_OP_REV_SUBTRACT:
        return PPE::Graphics::EBlendFunction::ReverseSubtract;
    case D3D11_BLEND_OP_SUBTRACT:
        return PPE::Graphics::EBlendFunction::Subtract;
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
} //!namespace PPE
