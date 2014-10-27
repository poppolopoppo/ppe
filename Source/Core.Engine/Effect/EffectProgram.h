#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
class BindName;
FWD_REFPTR(ConstantBufferLayout);
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MaterialContext;
FWD_REFPTR(EffectProgram);
//----------------------------------------------------------------------------
class EffectProgram : public Graphics::ShaderProgram {
public:
    EffectProgram(Graphics::ShaderProfileType profile, Graphics::ShaderProgramType type);
    virtual ~EffectProgram();

    virtual void Create(    Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler,
                            const char *entryPoint,
                            Graphics::ShaderCompilerFlags flags,
                            const Graphics::ShaderSource *source,
                            const Graphics::VertexDeclaration *vertexDeclaration) override;

    virtual void Destroy(   Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler) override;

    const ASSOCIATIVE_VECTOR(Shader, Graphics::BindName, Graphics::PCConstantBufferLayout)& Constants() const { return _constants; }
    const VECTOR(Shader, Graphics::BindName)& Textures() const { return _textures; }

    SINGLETON_POOL_ALLOCATED_DECL(MaterialProgram);

private:
    ASSOCIATIVE_VECTOR(Shader, Graphics::BindName, Graphics::PCConstantBufferLayout) _constants;
    VECTOR(Shader, Graphics::BindName) _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
