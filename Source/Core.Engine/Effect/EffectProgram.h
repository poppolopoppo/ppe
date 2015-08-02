#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
struct ShaderProgramTexture;
}

namespace Engine {
FWD_REFPTR(Effect);
FWD_REFPTR(SharedConstantBuffer);
class SharedConstantBufferFactory;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(EffectProgram);
typedef size_t EffectSharedBufferIndex;
//----------------------------------------------------------------------------
class EffectProgram : public Graphics::ShaderProgram {
public:
    EffectProgram(Graphics::ShaderProfileType profile, Graphics::ShaderProgramType type);
    virtual ~EffectProgram();

    void LinkReflectedData( VECTOR(Effect, PSharedConstantBuffer)& sharedBuffers,
                            SharedConstantBufferFactory *sharedBufferFactory,
                            Graphics::IDeviceAPIShaderCompiler *compiler);

    void UnlinkReflectedData();

    void Set(Graphics::IDeviceAPIContext *context, const Effect *effect) const;

    const ASSOCIATIVE_VECTOR(Shader, Graphics::BindName, EffectSharedBufferIndex)& Constants() const { return _constants; }
    const VECTOR(Shader, Graphics::ShaderProgramTexture)& Textures() const { return _textures; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ASSOCIATIVE_VECTOR(Shader, Graphics::BindName, EffectSharedBufferIndex) _constants;
    VECTOR(Shader, Graphics::ShaderProgramTexture) _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
