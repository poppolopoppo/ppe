#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/State/SamplerState.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SamplerState : public DeviceAPIDependantSamplerState {
public:
    SamplerState(IDeviceAPIEncapsulator *device, Graphics::SamplerState *owner);
    virtual ~SamplerState();

    ::ID3D11SamplerState *Entity() const { return _entity.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(SamplerState);

private:
    ComPtr<::ID3D11SamplerState> _entity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_FILTER TextureFilterToDX11Filter(Graphics::TextureFilter value);
//----------------------------------------------------------------------------
Graphics::TextureFilter DX11FilterToTextureFilter(D3D11_FILTER value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE TextureAddressModeToDX11TextureAddressMode(Graphics::TextureAddressMode value);
//----------------------------------------------------------------------------
Graphics::TextureAddressMode DX11TextureAddressModeToTextureAddressMode(D3D11_TEXTURE_ADDRESS_MODE value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
