#include "stdafx.h"

#include "EffectProgram.h"

#include "AbstractMaterialParameter.h"

#include "Core.Graphics/BindName.h"
#include "Core.Graphics/ConstantBufferLayout.h"
#include "Core.Graphics/DeviceAPIEncapsulator.h"

#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(EffectProgram, );
//----------------------------------------------------------------------------
EffectProgram::EffectProgram(Graphics::ShaderProfileType profile, Graphics::ShaderProgramType type)
:   ShaderProgram(profile, type) {}
//----------------------------------------------------------------------------
EffectProgram::~EffectProgram() {}
//----------------------------------------------------------------------------
void EffectProgram::Create(
    Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler,
    const char *entryPoint,
    Graphics::ShaderCompilerFlags flags,
    const Graphics::ShaderSource *source,
    const Graphics::VertexDeclaration *vertexDeclaration ) {
    Assert(_constants.empty());
    Assert(_textures.empty());

    ShaderProgram::Create(compiler, entryPoint, flags, source, vertexDeclaration);
    ShaderProgram::Reflect(compiler, _constants, _textures);
}
//----------------------------------------------------------------------------
void EffectProgram::Destroy(Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler) {
    ShaderProgram::Destroy(compiler);

    _constants.clear();
    _textures.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
