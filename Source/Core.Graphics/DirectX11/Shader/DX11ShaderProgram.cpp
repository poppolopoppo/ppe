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
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11ShaderProgram, );
//----------------------------------------------------------------------------
FDX11ShaderProgram::FDX11ShaderProgram(IDeviceAPIEncapsulator* device, const FShaderProgram* resource)
:   FDeviceAPIDependantShaderProgram(device, resource) {

    const FDX11DeviceWrapper* wrapper = DX11GetDeviceWrapper(device);
    const FShaderCompiled* compiled = Compiled();

    switch (ProgramType())
    {
    case Core::Graphics::EShaderProgramType::Vertex:
        {
            TComPtr<::ID3D11VertexShader> vertexShader;
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
    case Core::Graphics::EShaderProgramType::Hull:
        {
            TComPtr<::ID3D11HullShader> hullShader;
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
    case Core::Graphics::EShaderProgramType::Domain:
        {
            TComPtr<::ID3D11DomainShader> domainShader;
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
    case Core::Graphics::EShaderProgramType::Pixel:
        {
            TComPtr<::ID3D11PixelShader> pixelShader;
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
    case Core::Graphics::EShaderProgramType::Geometry:
        {
            TComPtr<::ID3D11GeometryShader> geometryShader;
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
    case Core::Graphics::EShaderProgramType::Compute:
        {
            TComPtr<::ID3D11ComputeShader> computeShader;
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
FDX11ShaderProgram::~FDX11ShaderProgram() {
    ReleaseComRef(_abstractShader);
}
//----------------------------------------------------------------------------
size_t FDX11ShaderProgram::VideoMemorySizeInBytes() const {
    return Compiled()->Blob().SizeInBytes();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
