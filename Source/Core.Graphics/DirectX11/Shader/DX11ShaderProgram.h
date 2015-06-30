#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompiler;
class VertexDeclaration;
FWD_REFPTR(AnnotedConstantBuffer);
struct AnnotedTextureSlot;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11ShaderProgram : public DeviceAPIDependantShaderProgram {
public:
    DX11ShaderProgram(  IDeviceAPIShaderCompiler *compiler,
                        ShaderProgram *owner,
                        const char *entryPoint,
                        ShaderCompilerFlags flags,
                        const ShaderSource *source,
                        const VertexDeclaration *vertexDeclaration);
    virtual ~DX11ShaderProgram();

    ::ID3DBlob *Entity() const { return _entity.Get(); }

    virtual size_t ProgramHashCode() const override { return _programHashCode; }

    virtual size_t VideoMemorySizeInBytes() const override;

    SINGLETON_POOL_ALLOCATED_DECL(DX11ShaderProgram);

    static void Preprocess( IDeviceAPIShaderCompiler *compiler,
                            RAWSTORAGE(Shader, char)& output,
                            const ShaderProgram *owner,
                            const ShaderSource *source,
                            const VertexDeclaration *vertexDeclaration);

    static void Reflect(IDeviceAPIShaderCompiler *compiler,
                        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
                        VECTOR(Shader, ShaderProgramTexture)& textures,
                        const ShaderProgram *program);

private:
    ComPtr<::ID3DBlob> _entity;
    size_t _programHashCode;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UINT ShaderCompilerFlagsToD3D11CompileFlags(ShaderCompilerFlags value);
//----------------------------------------------------------------------------
ShaderCompilerFlags D3D11CompileFlagsToShaderCompilerFlags(UINT  value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LPCSTR ShaderProfileTypeToD3D11Target(ShaderProgramType program, ShaderProfileType profile);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
