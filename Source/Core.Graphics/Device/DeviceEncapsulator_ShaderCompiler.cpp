#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "Geometry/IndexBuffer.h"
#include "Geometry/PrimitiveType.h"
#include "Geometry/VertexBuffer.h"
#include "Geometry/VertexDeclaration.h"

#include "Shader/ConstantBuffer.h"
#include "Shader/ShaderEffect.h"
#include "Shader/ShaderProgram.h"

#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/RasterizerState.h"
#include "State/SamplerState.h"

#include "Texture/DepthStencil.h"
#include "Texture/RenderTarget.h"
#include "Texture/Texture.h"
#include "Texture/Texture2D.h"
#include "Texture/TextureCube.h"

#include "Core/Diagnostic/Logger.h"

// IDeviceAPIShaderCompiler

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Shader Program
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram *DeviceEncapsulator::CreateShaderProgram(
    ShaderProgram *program,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entryPoint);
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIEncapsulator->ShaderCompiler()->CreateShaderProgram(program, entryPoint, flags, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(entity);
    Assert(&entity == &program->DeviceAPIDependantProgram());

    _deviceAPIEncapsulator->ShaderCompiler()->DestroyShaderProgram(program, entity);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::PreprocessShaderProgram(
    RAWSTORAGE(Shader, char)& output,
    const ShaderProgram *program,
    const ShaderSource *source,
    const VertexDeclaration *vertexDeclaration) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(source);
    Assert(vertexDeclaration);

    return _deviceAPIEncapsulator->ShaderCompiler()->PreprocessShaderProgram(output, program, source, vertexDeclaration);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ReflectShaderProgram(
    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
    VECTOR(Shader, ShaderProgramTexture)& textures,
    const ShaderProgram *program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(program);
    Assert(program->Frozen());
    Assert(program->Available());
    Assert(constants.empty());
    Assert(textures.empty());

    return _deviceAPIEncapsulator->ShaderCompiler()->ReflectShaderProgram(constants, textures, program);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
