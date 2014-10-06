#include "stdafx.h"

#include "DX11SamplerState.h"

#include "DX11DeviceEncapsulator.h"

#include "DeviceAPIEncapsulator.h"
#include "DeviceEncapsulatorException.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SamplerState::SamplerState(IDeviceAPIEncapsulator *device, Graphics::SamplerState *owner)
:   DeviceAPIDependantSamplerState(device, owner) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    ::D3D11_SAMPLER_DESC samplerStateDesc;
    ::SecureZeroMemory(&samplerStateDesc, sizeof(samplerStateDesc));

    samplerStateDesc.Filter = TextureFilterToDX11Filter(owner->Filter());
    samplerStateDesc.AddressU = TextureAddressModeToDX11TextureAddressMode(owner->AddressU());
    samplerStateDesc.AddressV = TextureAddressModeToDX11TextureAddressMode(owner->AddressV());
    samplerStateDesc.AddressW = TextureAddressModeToDX11TextureAddressMode(owner->AddressW());

    samplerStateDesc.MipLODBias = owner->MipMapLODBias();
    samplerStateDesc.MaxAnisotropy = checked_cast<UINT>(owner->MaxAnisotropy());
    samplerStateDesc.MinLOD = 0;
    samplerStateDesc.MaxLOD = float(owner->MaxMipLevel());

    samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // not used

    DX11_THROW_IF_FAILED(device, owner, (
        wrapper->Device()->CreateSamplerState(&samplerStateDesc, _entity.GetAddressOf())
        ));

    Assert(_entity);

    DX11SetDeviceResourceNameIFP(_entity, owner);
}
//----------------------------------------------------------------------------
SamplerState::~SamplerState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(SamplerState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILTER TextureFilterToDX11Filter(Graphics::TextureFilter value) {
    switch (value)
    {
    case Core::Graphics::TextureFilter::Linear:
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    case Core::Graphics::TextureFilter::Point:
        return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case Core::Graphics::TextureFilter::Anisotropic:
        return D3D11_FILTER_ANISOTROPIC;
    case Core::Graphics::TextureFilter::LinearMipPoint:
        return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    case Core::Graphics::TextureFilter::PointMipLinear:
        return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case Core::Graphics::TextureFilter::MinLinearMagPointMipLinear:
        return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case Core::Graphics::TextureFilter::MinLinearMagPointMipPoint:
        return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case Core::Graphics::TextureFilter::MinPointMagLinearMipLinear:
        return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    case Core::Graphics::TextureFilter::MinPointMagLinearMipPoint:
        return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    }
    AssertNotImplemented();
    return static_cast<D3D11_FILTER>(-1);
}
//----------------------------------------------------------------------------
Graphics::TextureFilter DX11FilterToTextureFilter(D3D11_FILTER value) {
    switch (value)
    {
    case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
        return Core::Graphics::TextureFilter::Linear;
    case D3D11_FILTER_MIN_MAG_MIP_POINT:
        return Core::Graphics::TextureFilter::Point;
    case D3D11_FILTER_ANISOTROPIC:
        return Core::Graphics::TextureFilter::Anisotropic;
    case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
        return Core::Graphics::TextureFilter::LinearMipPoint;
    case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
        return Core::Graphics::TextureFilter::PointMipLinear;
    case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return Core::Graphics::TextureFilter::MinLinearMagPointMipLinear;
    case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
        return Core::Graphics::TextureFilter::MinLinearMagPointMipPoint;
    case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
        return Core::Graphics::TextureFilter::MinPointMagLinearMipLinear;
    case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
        return Core::Graphics::TextureFilter::MinPointMagLinearMipPoint;
    }
    AssertNotImplemented();
    return static_cast<Graphics::TextureFilter>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToDX11TextureAddressMode(Graphics::TextureAddressMode value) {
    switch (value)
    {
    case Core::Graphics::TextureAddressMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case Core::Graphics::TextureAddressMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    case Core::Graphics::TextureAddressMode::Wrap:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    }
    AssertNotImplemented();
    return static_cast<D3D11_TEXTURE_ADDRESS_MODE>(-1);
}
//----------------------------------------------------------------------------
Graphics::TextureAddressMode DX11TextureAddressModeToTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE value) {
    switch (value)
    {
    case D3D11_TEXTURE_ADDRESS_CLAMP:
        return Core::Graphics::TextureAddressMode::Clamp;
    case D3D11_TEXTURE_ADDRESS_MIRROR:
        return Core::Graphics::TextureAddressMode::Mirror;
    case D3D11_TEXTURE_ADDRESS_WRAP:
        return Core::Graphics::TextureAddressMode::Wrap;
    }
    AssertNotImplemented();
    return static_cast<Graphics::TextureAddressMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
