// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11SamplerState.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"

#include "Allocator/PoolAllocator-impl.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11SamplerState::FDX11SamplerState(IDeviceAPIEncapsulator *device, FSamplerState *owner)
:   FDeviceAPIDependantSamplerState(device, owner) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

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

    DX11SetDeviceResourceNameIFP(_entity.Get(), owner);
}
//----------------------------------------------------------------------------
FDX11SamplerState::~FDX11SamplerState() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11SamplerState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILTER TextureFilterToDX11Filter(ETextureFilter value) {
    switch (value)
    {
    case PPE::Graphics::ETextureFilter::Linear:
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    case PPE::Graphics::ETextureFilter::Point:
        return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case PPE::Graphics::ETextureFilter::Anisotropic:
        return D3D11_FILTER_ANISOTROPIC;
    case PPE::Graphics::ETextureFilter::LinearMipPoint:
        return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    case PPE::Graphics::ETextureFilter::PointMipLinear:
        return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case PPE::Graphics::ETextureFilter::MinLinearMagPointMipLinear:
        return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case PPE::Graphics::ETextureFilter::MinLinearMagPointMipPoint:
        return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case PPE::Graphics::ETextureFilter::MinPointMagLinearMipLinear:
        return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    case PPE::Graphics::ETextureFilter::MinPointMagLinearMipPoint:
        return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    }
    AssertNotImplemented();
    return static_cast<D3D11_FILTER>(-1);
}
//----------------------------------------------------------------------------
ETextureFilter DX11FilterToTextureFilter(D3D11_FILTER value) {
    switch (value)
    {
    case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
        return PPE::Graphics::ETextureFilter::Linear;
    case D3D11_FILTER_MIN_MAG_MIP_POINT:
        return PPE::Graphics::ETextureFilter::Point;
    case D3D11_FILTER_ANISOTROPIC:
        return PPE::Graphics::ETextureFilter::Anisotropic;
    case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
        return PPE::Graphics::ETextureFilter::LinearMipPoint;
    case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
        return PPE::Graphics::ETextureFilter::PointMipLinear;
    case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
        return PPE::Graphics::ETextureFilter::MinLinearMagPointMipLinear;
    case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
        return PPE::Graphics::ETextureFilter::MinLinearMagPointMipPoint;
    case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
        return PPE::Graphics::ETextureFilter::MinPointMagLinearMipLinear;
    case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
        return PPE::Graphics::ETextureFilter::MinPointMagLinearMipPoint;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::ETextureFilter>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToDX11TextureAddressMode(ETextureAddressMode value) {
    switch (value)
    {
    case PPE::Graphics::ETextureAddressMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case PPE::Graphics::ETextureAddressMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    case PPE::Graphics::ETextureAddressMode::Wrap:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    default:
        AssertNotImplemented();
    }
    return static_cast<D3D11_TEXTURE_ADDRESS_MODE>(-1);
}
//----------------------------------------------------------------------------
ETextureAddressMode DX11TextureAddressModeToTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE value) {
    switch (value)
    {
    case D3D11_TEXTURE_ADDRESS_CLAMP:
        return PPE::Graphics::ETextureAddressMode::Clamp;
    case D3D11_TEXTURE_ADDRESS_MIRROR:
        return PPE::Graphics::ETextureAddressMode::Mirror;
    case D3D11_TEXTURE_ADDRESS_WRAP:
        return PPE::Graphics::ETextureAddressMode::Wrap;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::ETextureAddressMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
