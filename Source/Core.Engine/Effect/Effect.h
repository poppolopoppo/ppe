#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/Shader/ShaderEffect.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Graphics {
class BindName;
FWD_REFPTR(BlendState);
FWD_REFPTR(DepthStencilState);
FWD_REFPTR(RasterizerState);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EffectProgram;
struct MaterialContext;
FWD_REFPTR(Effect);
FWD_REFPTR(EffectDescriptor);
//----------------------------------------------------------------------------
class Effect : public Graphics::ShaderEffect {
public:
    Effect( const EffectDescriptor *descriptor,
            const Graphics::VertexDeclaration *vertexDeclaration,
            const MemoryView<const Graphics::BindName>& tags );
    virtual ~Effect();

    const PCEffectDescriptor& Descriptor() const { return _descriptor; }

    const VECTOR(Effect, Graphics::BindName)& Tags() const { return _tags; }

    const Graphics::BlendState *BlendState() const { return _blendState.get(); }
    const Graphics::DepthStencilState *DepthStencilState() const { return _depthStencilState.get(); }
    const Graphics::RasterizerState *RasterizerState() const;

    EffectProgram *StageProgram(Graphics::ShaderProgramType stage);
    const EffectProgram *StageProgram(Graphics::ShaderProgramType stage) const;

    virtual void Create(Graphics::IDeviceAPIEncapsulator *device) override;
    virtual void Destroy(Graphics::IDeviceAPIEncapsulator *device) override;

    void Set(Graphics::IDeviceAPIContextEncapsulator *context) const;

    static const Graphics::RasterizerState *AutomaticRasterizerState;
    static const Graphics::RasterizerState *DefaultRasterizerState;

    static void SwitchAutomaticFillMode();

    static void Start();
    static void Shutdown();

    SINGLETON_POOL_ALLOCATED_DECL(Effect);

private:
    PCEffectDescriptor _descriptor;

    VECTOR(Effect, Graphics::BindName) _tags;

    Graphics::PCBlendState _blendState;
    Graphics::PCDepthStencilState _depthStencilState;
    Graphics::PCRasterizerState _rasterizerState;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
