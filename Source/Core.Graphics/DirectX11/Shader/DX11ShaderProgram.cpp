#include "stdafx.h"

#include "DX11ShaderProgram.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/Geometry/DX11VertexDeclaration.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"
#include "Device/Shader/ShaderCompiled.h"
#include "Device/Shader/VertexSubstitutions.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11ShaderProgram, );
//----------------------------------------------------------------------------
DX11ShaderProgram::DX11ShaderProgram(IDeviceAPIEncapsulator* device, const ShaderProgram* resource)
:   DeviceAPIDependantShaderProgram(device, resource) {

    const DX11DeviceWrapper* wrapper = DX11GetDeviceWrapper(device);
    const ShaderCompiled* compiled = Compiled();

    switch (ProgramType())
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        {
            ComPtr<::ID3D11VertexShader> vertexShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreateVertexShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    vertexShader.GetAddressOf())
                ));
            _abstractShader = vertexShader.Get();
        }
        break;
    case Core::Graphics::ShaderProgramType::Hull:
        {
            ComPtr<::ID3D11HullShader> hullShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreateHullShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    hullShader.GetAddressOf())
                ));
            _abstractShader = hullShader.Get();
        }
        break;
    case Core::Graphics::ShaderProgramType::Domain:
        {
            ComPtr<::ID3D11DomainShader> domainShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreateDomainShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    domainShader.GetAddressOf())
                ));
            _abstractShader = domainShader.Get();
        }
        break;
    case Core::Graphics::ShaderProgramType::Pixel:
        {
            ComPtr<::ID3D11PixelShader> pixelShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreatePixelShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    pixelShader.GetAddressOf())
                ));
            _abstractShader = pixelShader.Get();
        }
        break;
    case Core::Graphics::ShaderProgramType::Geometry:
        {
            ComPtr<::ID3D11GeometryShader> geometryShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreateGeometryShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    geometryShader.GetAddressOf())
                ));
            _abstractShader = geometryShader.Get();
        }
        break;
    case Core::Graphics::ShaderProgramType::Compute:
        {
            ComPtr<::ID3D11ComputeShader> computeShader;
            DX11_THROW_IF_FAILED(device, resource, (
                wrapper->Device()->CreateComputeShader(
                    compiled->Blob().Pointer(),
                    compiled->Blob().SizeInBytes(),
                    NULL,
                    computeShader.GetAddressOf())
                ));
            _abstractShader = computeShader.Get();
        }
        break;
    default:
        AssertNotImplemented();
        break;
    }

    Assert(_abstractShader);
    DX11SetDeviceResourceNameIFP(_abstractShader, resource);
}
//----------------------------------------------------------------------------
DX11ShaderProgram::~DX11ShaderProgram() {
    ReleaseComRef(_abstractShader);
}
//----------------------------------------------------------------------------
size_t DX11ShaderProgram::VideoMemorySizeInBytes() const {
    return Compiled()->Blob().SizeInBytes();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
