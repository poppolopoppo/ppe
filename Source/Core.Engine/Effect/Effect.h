#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/Shader/ShaderEffect.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Graphics {
class FBindName;
FWD_REFPTR(BlendState);
FWD_REFPTR(DepthStencilState);
FWD_REFPTR(RasterizerState);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEffectProgram;
struct FMaterialContext;
FWD_REFPTR(Effect);
FWD_REFPTR(EffectDescriptor);
FWD_REFPTR(SharedConstantBuffer);
class FSharedConstantBufferFactory;
//----------------------------------------------------------------------------
class FEffect : public Graphics::FShaderEffect {
public:
    FEffect( const FEffectDescriptor *descriptor,
            const Graphics::FVertexDeclaration *vertexDeclaration,
            const TMemoryView<const Graphics::FBindName>& tags );
    virtual ~FEffect();

    const PCEffectDescriptor& Descriptor() const { return _descriptor; }

    const VECTOR(FEffect, Graphics::FBindName)& Tags() const { return _tags; }
    const VECTOR(FEffect, PSharedConstantBuffer)& SharedBuffers() const { return _sharedBuffers; }

    const Graphics::FBlendState *FBlendState() const { return _blendState.get(); }
    const Graphics::FDepthStencilState *FDepthStencilState() const { return _depthStencilState.get(); }
    const Graphics::FRasterizerState *FRasterizerState() const;

    FEffectProgram *StageProgram(Graphics::EShaderProgramType stage);
    const FEffectProgram *StageProgram(Graphics::EShaderProgramType stage) const;

    virtual void Create(Graphics::IDeviceAPIEncapsulator *device) override;
    virtual void Destroy(Graphics::IDeviceAPIEncapsulator *device) override;

    void LinkReflectedData( FSharedConstantBufferFactory *sharedBufferFactory,
                            Graphics::IDeviceAPIShaderCompiler *compiler);
    void UnlinkReflectedData(FSharedConstantBufferFactory *sharedBufferFactory);

    void Set(Graphics::IDeviceAPIContext *context) const;

    static const Graphics::FRasterizerState *AutomaticRasterizerState;
    static const Graphics::FRasterizerState *DefaultRasterizerState;

    static void SwitchAutomaticFillMode();

    static void Start();
    static void Shutdown();

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    PCEffectDescriptor _descriptor;

    VECTOR(FEffect, Graphics::FBindName) _tags;
    VECTOR(FEffect, PSharedConstantBuffer) _sharedBuffers;

    Graphics::PCBlendState _blendState;
    Graphics::PCDepthStencilState _depthStencilState;
    Graphics::PCRasterizerState _rasterizerState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
