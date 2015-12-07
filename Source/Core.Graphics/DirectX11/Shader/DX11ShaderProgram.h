#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompiler;
class VertexDeclaration;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ShaderProgram : public DeviceAPIDependantShaderProgram {
public:
    DX11ShaderProgram(IDeviceAPIEncapsulator* device, const ShaderProgram* resource);
    virtual ~DX11ShaderProgram();

    ::ID3D11VertexShader*   VertexShader()  const { Assert(ShaderProgramType::Vertex   == ProgramType()); return static_cast<::ID3D11VertexShader*>(_abstractShader.Get()); }
    ::ID3D11DomainShader*   DomainShader()  const { Assert(ShaderProgramType::Domain   == ProgramType()); return static_cast<::ID3D11DomainShader*>(_abstractShader.Get()); }
    ::ID3D11HullShader*     HullShader()    const { Assert(ShaderProgramType::Hull     == ProgramType()); return static_cast<::ID3D11HullShader*>(_abstractShader.Get()); }
    ::ID3D11GeometryShader* GeometryShader()const { Assert(ShaderProgramType::Geometry == ProgramType()); return static_cast<::ID3D11GeometryShader*>(_abstractShader.Get()); }
    ::ID3D11PixelShader*    PixelShader()   const { Assert(ShaderProgramType::Pixel    == ProgramType()); return static_cast<::ID3D11PixelShader*>(_abstractShader.Get()); }
    ::ID3D11ComputeShader*  ComputeShader() const { Assert(ShaderProgramType::Compute  == ProgramType()); return static_cast<::ID3D11ComputeShader*>(_abstractShader.Get()); }

    virtual size_t VideoMemorySizeInBytes() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ComPtr<::ID3D11DeviceChild> _abstractShader;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
