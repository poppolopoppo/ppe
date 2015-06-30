#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIShaderCompiler {
public:
    virtual ~IDeviceAPIShaderCompiler() {}

    virtual const AbstractDeviceAPIEncapsulator *APIEncapsulator() const = 0;

    virtual DeviceAPIDependantShaderProgram *CreateShaderProgram(
        ShaderProgram *program,
        const char *entryPoint,
        ShaderCompilerFlags flags,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) = 0;
    virtual void DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) = 0;

    virtual void PreprocessShaderProgram(
        RAWSTORAGE(Shader, char)& output,
        const ShaderProgram *program,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) = 0;

    virtual void ReflectShaderProgram(
        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
        VECTOR(Shader, ShaderProgramTexture)& textures,
        const ShaderProgram *program) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
