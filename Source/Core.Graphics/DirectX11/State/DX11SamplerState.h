#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/State/SamplerState.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11SamplerState : public DeviceAPIDependantSamplerState {
public:
    DX11SamplerState(IDeviceAPIEncapsulator *device, SamplerState *owner);
    virtual ~DX11SamplerState();

    ::ID3D11SamplerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(DX11SamplerState);

private:
    ComPtr<::ID3D11SamplerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILTER TextureFilterToDX11Filter(TextureFilter value);
//----------------------------------------------------------------------------
TextureFilter DX11FilterToTextureFilter(D3D11_FILTER value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToDX11TextureAddressMode(TextureAddressMode value);
//----------------------------------------------------------------------------
TextureAddressMode DX11TextureAddressModeToTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
