#pragma once

#include "Device/IDeviceAPIShaderCompiler.h"
#include "DirectX11/Shader/DX11ShaderProgram.h"

#include "DirectX11/DX11Includes.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11ShaderCompiler : public IDeviceAPIShaderCompiler {
public:
    FDX11ShaderCompiler();
    virtual ~FDX11ShaderCompiler();

    virtual FShaderCompiled *CompileShaderSource(
        const FShaderSource *source,
        const FVertexDeclaration *vertexDeclaration,
        EShaderProgramType programType,
        EShaderProfileType profileType,
        EShaderCompilerFlags flags,
        const char *entryPoint ) override final;

    virtual void PreprocessShaderSource(
        RAWSTORAGE(Shader, char)& output,
        const FShaderSource *source,
        const FVertexDeclaration *vertexDeclaration) override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(EShaderCompilerFlags value);
//----------------------------------------------------------------------------
EShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value);
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(EShaderProgramType program, EShaderProfileType profile);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
