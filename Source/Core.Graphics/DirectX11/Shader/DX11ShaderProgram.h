#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/ComPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompilerEncapsulator;
class VertexDeclaration;

FWD_REFPTR(AnnotedConstantBuffer);
struct AnnotedTextureSlot;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ShaderProgram : public DeviceAPIDependantShaderProgram {
public:
    ShaderProgram(  IDeviceAPIShaderCompilerEncapsulator *compiler,
                    Graphics::ShaderProgram *owner,
                    const char *entryPoint,
                    ShaderCompilerFlags flags,
                    const Graphics::ShaderSource *source,
                    const Graphics::VertexDeclaration *vertexDeclaration);
    virtual ~ShaderProgram();

    ::ID3DBlob *Entity() const { return _entity.Get(); }

    virtual size_t ProgramHashCode() const override { return _programHashCode; }

    SINGLETON_POOL_ALLOCATED_DECL(ShaderProgram);

    static void Preprocess( IDeviceAPIShaderCompilerEncapsulator *compiler,
                            RAWSTORAGE(Shader, char)& output,
                            const Graphics::ShaderProgram *owner,
                            const Graphics::ShaderSource *source,
                            const Graphics::VertexDeclaration *vertexDeclaration);

    static void Reflect(IDeviceAPIShaderCompilerEncapsulator *compiler,
                        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
                        VECTOR(Shader, BindName)& textures,
                        const Graphics::ShaderProgram *program);

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
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
