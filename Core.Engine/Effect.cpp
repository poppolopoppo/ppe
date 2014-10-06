#include "stdafx.h"

#include "Effect.h"

#include "EffectDescriptor.h"
#include "EffectProgram.h"
#include "Material.h"
#include "MaterialContext.h"
#include "MaterialVariability.h"
#include "RenderState.h"
#include "Scene.h"
#include "TextureCache.h"

#include "Core.Graphics/BindName.h"
#include "Core.Graphics/BlendState.h"
#include "Core.Graphics/DepthStencilState.h"
#include "Core.Graphics/DeviceEncapsulator.h"
#include "Core.Graphics/RasterizerState.h"
#include "Core.Graphics/ShaderProgram.h"
#include "Core.Graphics/ShaderSource.h"
#include "Core.Graphics/Texture.h"
#include "Core.Graphics/Texture2D.h"
#include "Core.Graphics/VertexDeclaration.h"

#include "Core/Filename.h"
#include "Core/Logger.h"
#include "Core/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void CreateEffectProgramIFN_(
    PEffectProgram *pprogram,
    Graphics::ShaderProgramType stage,
    const Graphics::PCVertexDeclaration& vertexDeclaration,
    const EffectDescriptor *effectDescriptor,
    Graphics::ShaderCompilerFlags compilerFlags,
    Graphics::IDeviceAPIShaderCompilerEncapsulator *compiler) {
    Assert(pprogram);
    Assert(!*pprogram);

    const Filename& filename = effectDescriptor->ProgramFilename(stage);
    if (filename.empty())
        return;

    *pprogram = new EffectProgram(effectDescriptor->ShaderProfile(), stage);
    (*pprogram)->SetResourceName(StringFormat("{0}_{1}", effectDescriptor->Name(), ShaderProgramTypeToCStr(stage)));
    (*pprogram)->Freeze();

    if (!Contains(effectDescriptor->VertexDeclarations(), vertexDeclaration))
        AssertNotReached();

    Graphics::CompileShaderProgram(
        compiler,
        *pprogram,
        Graphics::ShaderProgramTypeToEntryPoint(stage),
        compilerFlags,
        filename,
        vertexDeclaration,
        MakeView(effectDescriptor->Defines()) );

    Assert((*pprogram)->Available());
}
//----------------------------------------------------------------------------
static const Graphics::BlendState *BlendStateFromRenderState_(const RenderState *renderState) {
    switch (renderState->Blend())
    {
    case RenderState::Blending::Additive:
        return Graphics::BlendState::Additive;
    case RenderState::Blending::AlphaBlend:
        return Graphics::BlendState::AlphaBlend;
    case RenderState::Blending::NonPremultiplied:
        return Graphics::BlendState::NonPremultiplied;
    case RenderState::Blending::Opaque:
        return Graphics::BlendState::Opaque;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
static const Graphics::DepthStencilState *DepthStencilStateFromRenderState_(const RenderState *renderState) {
    switch (renderState->Depth())
    {
    case RenderState::DepthTest::Default:
        return Graphics::DepthStencilState::Default;
    case RenderState::DepthTest::None:
        return Graphics::DepthStencilState::None;
    case RenderState::DepthTest::Read:
        return Graphics::DepthStencilState::DepthRead;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
static const Graphics::RasterizerState *RasterizerStateFromRenderState_(const RenderState *renderState) {
    switch (renderState->Fill())
    {
    case RenderState::FillMode::Automatic:
        return nullptr; // accessed every time from Effect::AutomaticRasterizerState
    case RenderState::FillMode::Wireframe:
        return Graphics::RasterizerState::Wireframe;
    }

    switch (renderState->Cull())
    {
    case RenderState::Culling::Clockwise:
        return Graphics::RasterizerState::CullClockwise;
    case RenderState::Culling::CounterClockwise:
        return Graphics::RasterizerState::CullCounterClockwise;
    case RenderState::Culling::None:
        return Graphics::RasterizerState::CullNone;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Effect, );
//----------------------------------------------------------------------------
const Graphics::RasterizerState *Effect::AutomaticRasterizerState = nullptr;
const Graphics::RasterizerState *Effect::DefaultRasterizerState = nullptr;
//----------------------------------------------------------------------------
Effect::Effect( const EffectDescriptor *descriptor,
                const Graphics::VertexDeclaration *vertexDeclaration )
:   ShaderEffect(vertexDeclaration)
,   _descriptor(descriptor)
,   _blendState(BlendStateFromRenderState_(descriptor->RenderState()) )
,   _depthStencilState(DepthStencilStateFromRenderState_(descriptor->RenderState()) )
,   _rasterizerState(RasterizerStateFromRenderState_(descriptor->RenderState()) ) {
    SetResourceName(descriptor->Name().c_str());
    Freeze();
}
//----------------------------------------------------------------------------
Effect::~Effect() {}
//----------------------------------------------------------------------------
const Graphics::RasterizerState *Effect::RasterizerState() const {
    return  _rasterizerState
        ?   _rasterizerState.get()
        :   AutomaticRasterizerState;
}
//----------------------------------------------------------------------------
EffectProgram *Effect::StageProgram(Graphics::ShaderProgramType stage) {
     return checked_cast<EffectProgram *>(Graphics::ShaderEffect::StageProgram(stage));
}
//----------------------------------------------------------------------------
const EffectProgram *Effect::StageProgram(Graphics::ShaderProgramType stage) const {
     return checked_cast<const EffectProgram *>(Graphics::ShaderEffect::StageProgram(stage));
}
//----------------------------------------------------------------------------
void Effect::Create(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    using namespace Graphics;

    Graphics::IDeviceAPIShaderCompilerEncapsulator *const compiler = device->Encapsulator()->Compiler();
    const Graphics::ShaderCompilerFlags compilerFlags =
#ifdef _DEBUG
        Graphics::ShaderCompilerFlags::DefaultForDebug
#else
        Graphics::ShaderCompilerFlags::Default
#endif
        ;

    const Graphics::VertexDeclaration *vertexDeclaration = this->VertexDeclaration();

    Unfreeze();

    for (ShaderProgramType stage : EachShaderProgramType()) {
        PEffectProgram program;
        CreateEffectProgramIFN_(&program, stage, vertexDeclaration, _descriptor, compilerFlags, compiler);

        if (!program)
            continue;

        ShaderEffect::SetStageProgram(stage, std::move(program));
    }

    Freeze();

    ShaderEffect::Create(device);
}
//----------------------------------------------------------------------------
void Effect::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    using namespace Graphics;

    ShaderEffect::Destroy(device);

    for (ShaderProgramType stage : EachShaderProgramType())
        ShaderEffect::ResetStageProgram(stage);
}
//----------------------------------------------------------------------------
void Effect::Set(Graphics::IDeviceAPIContextEncapsulator *context) const {
    context->SetBlendState(this->BlendState());
    context->SetDepthStencilState(this->DepthStencilState());
    context->SetRasterizerState(this->RasterizerState());
    context->SetShaderEffect(this);
}
//----------------------------------------------------------------------------
void Effect::SwitchAutomaticFillMode() {
    const bool wireframe = (AutomaticRasterizerState == DefaultRasterizerState);
    LOG(Information, L"[Effect] Toggle {0} fill mode for automatic rasterizer state ...",
        wireframe ? "wireframe" : "solid");

    AutomaticRasterizerState = (wireframe)
        ? Graphics::RasterizerState::Wireframe
        : DefaultRasterizerState;
}
//----------------------------------------------------------------------------
void Effect::Start() {
    DefaultRasterizerState = Graphics::RasterizerState::CullCounterClockwise;
    AutomaticRasterizerState = DefaultRasterizerState;
}
//----------------------------------------------------------------------------
void Effect::Shutdown() {
    AutomaticRasterizerState = nullptr;
    DefaultRasterizerState = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
