#include "stdafx.h"

#include "Effect.h"

#include "EffectDescriptor.h"
#include "EffectProgram.h"
#include "SharedConstantBuffer.h"
#include "SharedConstantBufferFactory.h"
#include "Material/Material.h"
#include "Material/MaterialVariability.h"
#include "Render/RenderState.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Shader/ShaderProgram.h"
#include "Core.Graphics/Device/Shader/ShaderSource.h"
#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"
#include "Core.Graphics/Device/Texture/Texture.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void CreateEffectProgramIFN_(
    PEffectProgram *pprogram,
    Effect *owner,
    Graphics::ShaderProgramType stage,
    const Graphics::PCVertexDeclaration& vertexDeclaration,
    const EffectDescriptor *effectDescriptor,
    Graphics::ShaderCompilerFlags compilerFlags,
    const MemoryView<const Pair<String, String>>& defines,
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    Assert(pprogram);
    Assert(!*pprogram);

    const Filename& filename = effectDescriptor->ProgramFilename(stage);
    if (filename.empty())
        return;

    *pprogram = new EffectProgram(owner, effectDescriptor->ShaderProfile(), stage);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    (*pprogram)->SetResourceName(StringFormat("{0}_{1}", effectDescriptor->Name(), ShaderProgramTypeToCStr(stage)));
#endif
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
        defines );

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
    default:
        break;
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
static void AppendTagSubstitutions_(
    VECTOR_THREAD_LOCAL(Effect, Pair<String COMMA String>)& defines, 
    const ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, String)& substitutions,
    const VECTOR(Effect, Graphics::BindName)& tags ) {

    for (const Pair<Graphics::BindName, String>& substitution : substitutions) {
        if (!Contains(tags, substitution.first))
            continue;

        Assert(!substitution.second.empty());
        const char *cstr = substitution.second.c_str();
        size_t size = substitution.second.size();

        StringSlice define;
        while (Split(&cstr, &size, ';', define)) {
            Assert(!define.empty());

            const char *pValue = nullptr;
            for (const char& ch : define)
                if ('=' == ch) {
                    Assert(!pValue);
                    pValue = &ch + 1;
                    break;
                }

            if (pValue) {
                String key(define.Pointer(), std::distance(define.Pointer(), pValue) - 1);
                String value(pValue, std::distance(pValue, define.end()));
                defines.emplace_back(std::move(key), std::move(value));
            }
            else {
                String key(define.Pointer(), define.size());
                defines.emplace_back(std::move(key), "1");
            }
        }
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, Effect, );
//----------------------------------------------------------------------------
const Graphics::RasterizerState *Effect::AutomaticRasterizerState = nullptr;
const Graphics::RasterizerState *Effect::DefaultRasterizerState = nullptr;
//----------------------------------------------------------------------------
Effect::Effect( const EffectDescriptor *descriptor,
                const Graphics::VertexDeclaration *vertexDeclaration,
                const MemoryView<const Graphics::BindName>& tags )
:   ShaderEffect(vertexDeclaration)
,   _descriptor(descriptor)
,   _tags(tags.begin(), tags.end())
,   _blendState(BlendStateFromRenderState_(descriptor->RenderState()) )
,   _depthStencilState(DepthStencilStateFromRenderState_(descriptor->RenderState()) )
,   _rasterizerState(RasterizerStateFromRenderState_(descriptor->RenderState()) ) {
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
    SetResourceName(descriptor->Name().c_str());
#endif // WITH_GRAPHICS_DEVICERESOURCE_NAME
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

    Graphics::IDeviceAPIShaderCompiler *const compiler = device->Encapsulator()->Compiler();
    const Graphics::ShaderCompilerFlags compilerFlags =
#ifdef _DEBUG
        Graphics::ShaderCompilerFlags::DefaultForDebug
#else
        Graphics::ShaderCompilerFlags::Default
#endif
        ;

    const Graphics::VertexDeclaration *vertexDeclaration = this->VertexDeclaration();

    VECTOR_THREAD_LOCAL(Effect, Pair<String COMMA String>) defines;
    defines.reserve(_descriptor->Defines().size() + _tags.size());
    defines.insert(defines.end(), _descriptor->Defines().begin(), _descriptor->Defines().end());
    AppendTagSubstitutions_(defines, _descriptor->Substitutions(), _tags);

    Unfreeze(); // enables ShaderEffect::SetStageProgram()

    for (ShaderProgramType stage : EachShaderProgramType()) {
        PEffectProgram program;
        CreateEffectProgramIFN_(&program, this, stage, vertexDeclaration, _descriptor, compilerFlags, MakeConstView(defines), compiler);

        if (program)
            ShaderEffect::SetStageProgram(stage, std::move(program));
    }

    Freeze(); // disables ShaderEffect::SetStageProgram()

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
void Effect::LinkReflectedData(
    SharedConstantBufferFactory *sharedBufferFactory, 
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_sharedBuffers.empty());

    using namespace Graphics;

    for (ShaderProgramType stage : EachShaderProgramType()) {
        EffectProgram *const program = StageProgram(stage);
        if (program)
            program->LinkReflectedData(_sharedBuffers, sharedBufferFactory, compiler);
    }
}
//----------------------------------------------------------------------------
void Effect::UnlinkReflectedData(SharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();

    using namespace Graphics;

    for (ShaderProgramType stage : EachShaderProgramType()) {
        EffectProgram *const program = StageProgram(stage);
        if (program)
            program->UnlinkReflectedData();
    }

    for (PSharedConstantBuffer& sharedBuffer : _sharedBuffers)
        sharedBufferFactory->ReleaseDestroyIFN(sharedBuffer);

    _sharedBuffers.clear();
}
//----------------------------------------------------------------------------
void Effect::Set(Graphics::IDeviceAPIContext *context) const {
    context->SetBlendState(this->BlendState());
    context->SetDepthStencilState(this->DepthStencilState());
    context->SetRasterizerState(this->RasterizerState());
    context->SetShaderEffect(this);

    using namespace Graphics;

    for (ShaderProgramType stage : EachShaderProgramType()) {
        const EffectProgram *const program = StageProgram(stage);
        if (program)
            program->Set(context, this);
    }
}
//----------------------------------------------------------------------------
void Effect::SwitchAutomaticFillMode() {
    const bool wireframe = (AutomaticRasterizerState == DefaultRasterizerState);
    LOG(Info, L"[Effect] Toggle {0} fill mode for automatic rasterizer state ...",
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
