#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/IDeviceAPIShaderCompiler.h"
#include "Core.Graphics/DirectX11/Shader/DX11ShaderProgram.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ShaderCompiler : public IDeviceAPIShaderCompiler {
public:
    DX11ShaderCompiler();
    virtual ~DX11ShaderCompiler();

    virtual ShaderCompiled *CompileShaderSource(
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration,
        ShaderProgramType programType,
        ShaderProfileType profileType,
        ShaderCompilerFlags flags,
        const char *entryPoint ) override;

    virtual void PreprocessShaderSource(
        RAWSTORAGE(Shader, char)& output,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(ShaderCompilerFlags value);
//----------------------------------------------------------------------------
ShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value);
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(ShaderProgramType program, ShaderProfileType profile);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
