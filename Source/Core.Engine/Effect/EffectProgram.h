#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
struct FShaderProgramTexture;
}

namespace Engine {
FWD_REFPTR(Effect);
FWD_REFPTR(SharedConstantBuffer);
class FSharedConstantBufferFactory;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(EffectProgram);
typedef size_t EffectSharedBufferIndex;
//----------------------------------------------------------------------------
class FEffectProgram : public Graphics::FShaderProgram {
public:
    FEffectProgram(Graphics::EShaderProfileType profile, Graphics::EShaderProgramType type);
    virtual ~FEffectProgram();

    void LinkReflectedData( VECTOR(FEffect, PSharedConstantBuffer)& sharedBuffers,
                            FSharedConstantBufferFactory *sharedBufferFactory,
                            Graphics::IDeviceAPIShaderCompiler *compiler);

    void UnlinkReflectedData();

    void Set(Graphics::IDeviceAPIContext *context, const FEffect *effect) const;

    const ASSOCIATIVE_VECTOR(Shader, Graphics::FBindName, EffectSharedBufferIndex)& Constants() const { return _constants; }
    const VECTOR(Shader, Graphics::FShaderProgramTexture)& Textures() const { return _textures; }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ASSOCIATIVE_VECTOR(Shader, Graphics::FBindName, EffectSharedBufferIndex) _constants;
    VECTOR(Shader, Graphics::FShaderProgramTexture) _textures;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
