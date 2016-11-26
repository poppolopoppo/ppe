#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompiler;
class FVertexDeclaration;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11ShaderProgram : public FDeviceAPIDependantShaderProgram {
public:
    FDX11ShaderProgram(IDeviceAPIEncapsulator* device, const FShaderProgram* resource);
    virtual ~FDX11ShaderProgram();

    ::ID3D11VertexShader*   VertexShader()  const { Assert(EShaderProgramType::Vertex   == ProgramType()); return static_cast<::ID3D11VertexShader*>(_abstractShader.Get()); }
    ::ID3D11DomainShader*   DomainShader()  const { Assert(EShaderProgramType::Domain   == ProgramType()); return static_cast<::ID3D11DomainShader*>(_abstractShader.Get()); }
    ::ID3D11HullShader*     HullShader()    const { Assert(EShaderProgramType::Hull     == ProgramType()); return static_cast<::ID3D11HullShader*>(_abstractShader.Get()); }
    ::ID3D11GeometryShader* GeometryShader()const { Assert(EShaderProgramType::Geometry == ProgramType()); return static_cast<::ID3D11GeometryShader*>(_abstractShader.Get()); }
    ::ID3D11PixelShader*    PixelShader()   const { Assert(EShaderProgramType::Pixel    == ProgramType()); return static_cast<::ID3D11PixelShader*>(_abstractShader.Get()); }
    ::ID3D11ComputeShader*  ComputeShader() const { Assert(EShaderProgramType::Compute  == ProgramType()); return static_cast<::ID3D11ComputeShader*>(_abstractShader.Get()); }

    virtual size_t VideoMemorySizeInBytes() const override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TComPtr<::ID3D11DeviceChild> _abstractShader;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
