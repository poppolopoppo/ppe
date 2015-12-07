#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/IDeviceAPIShaderCompiler.h"

#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ShaderCompilerEncapsulator
:   private Meta::ThreadResource
,   private IDeviceAPIShaderCompiler {
public:
    ShaderCompilerEncapsulator();
    virtual ~ShaderCompilerEncapsulator();

    DeviceAPI API() const { return _api; }
    IDeviceAPIShaderCompiler* Compiler() const;

    void Create(DeviceAPI api);
    void Destroy();

private: // IDeviceAPIShaderCompiler

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

private:
    DeviceAPI _api;
    UniquePtr< IDeviceAPIShaderCompiler > _deviceAPIShaderCompiler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, ShaderCompilerFlags flags);
//----------------------------------------------------------------------------
PShaderCompiled CompileShaderSource(
    const ShaderCompilerEncapsulator* encapsulator,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    ShaderCompilerFlags flags,
    const char *entryPoint,
    const MemoryView<const Pair<String, String>>& defines );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
