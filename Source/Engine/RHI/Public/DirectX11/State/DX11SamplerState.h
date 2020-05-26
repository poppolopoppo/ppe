#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/State/SamplerState.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11SamplerState : public FDeviceAPIDependantSamplerState {
public:
    FDX11SamplerState(IDeviceAPIEncapsulator *device, FSamplerState *owner);
    virtual ~FDX11SamplerState();

    ::ID3D11SamplerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11SamplerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILTER TextureFilterToDX11Filter(ETextureFilter value);
//----------------------------------------------------------------------------
ETextureFilter DX11FilterToTextureFilter(D3D11_FILTER value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToDX11TextureAddressMode(ETextureAddressMode value);
//----------------------------------------------------------------------------
ETextureAddressMode DX11TextureAddressModeToTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
