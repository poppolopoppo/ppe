#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/Shader/ShaderEffect.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/ComPtr.h"

namespace PPE {
namespace Graphics {
class IDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11ShaderEffect : public FDeviceAPIDependantShaderEffect {
public:
    explicit FDX11ShaderEffect(IDeviceAPIEncapsulator *device, FShaderEffect *owner);
    virtual ~FDX11ShaderEffect();

    ::ID3D11InputLayout *InputLayout() const { return _inputLayout.Get(); }

    ::ID3D11VertexShader *VertexShader() const { return _vertexShader.Get(); }
    ::ID3D11DomainShader *DomainShader() const { return _domainShader.Get(); }
    ::ID3D11HullShader *HullShader() const { return _hullShader.Get(); }
    ::ID3D11GeometryShader *GeometryShader() const { return _geometryShader.Get(); }
    ::ID3D11PixelShader *PixelShader() const { return _pixelShader.Get(); }
    ::ID3D11ComputeShader *ComputeShader() const { return _computeShader.Get(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11InputLayout> _inputLayout;

    TComPtr<::ID3D11VertexShader> _vertexShader;
    TComPtr<::ID3D11DomainShader> _domainShader;
    TComPtr<::ID3D11HullShader> _hullShader;
    TComPtr<::ID3D11GeometryShader> _geometryShader;
    TComPtr<::ID3D11PixelShader> _pixelShader;
    TComPtr<::ID3D11ComputeShader> _computeShader;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
