#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ShaderEffect.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ShaderEffect : public DeviceAPIDependantShaderEffect {
public:
    explicit DX11ShaderEffect(IDeviceAPIEncapsulator *device, ShaderEffect *owner);
    virtual ~DX11ShaderEffect();

    ::ID3D11InputLayout *InputLayout() const { return _inputLayout.Get(); }

    ::ID3D11VertexShader *VertexShader() const { return _vertexShader.Get(); }
    ::ID3D11DomainShader *DomainShader() const { return _domainShader.Get(); }
    ::ID3D11HullShader *HullShader() const { return _hullShader.Get(); }
    ::ID3D11GeometryShader *GeometryShader() const { return _geometryShader.Get(); }
    ::ID3D11PixelShader *PixelShader() const { return _pixelShader.Get(); }
    ::ID3D11ComputeShader *ComputeShader() const { return _computeShader.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL(DX11ShaderEffect);

private:
    ComPtr<::ID3D11InputLayout> _inputLayout;

    ComPtr<::ID3D11VertexShader> _vertexShader;
    ComPtr<::ID3D11DomainShader> _domainShader;
    ComPtr<::ID3D11HullShader> _hullShader;
    ComPtr<::ID3D11GeometryShader> _geometryShader;
    ComPtr<::ID3D11PixelShader> _pixelShader;
    ComPtr<::ID3D11ComputeShader> _computeShader;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
