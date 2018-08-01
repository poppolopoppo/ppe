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
#include "Core/IO/StringView.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void CreateEffectProgramIFN_(
    PEffectProgram *pprogram,
    FEffect *owner,
    Graphics::EShaderProgramType stage,
    const Graphics::PCVertexDeclaration& vertexDeclaration,
    const FEffectDescriptor *effectDescriptor,
    Graphics::EShaderCompilerFlags compilerFlags,
    const TMemoryView<const TPair<FString, FString>>& defines,
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    Assert(pprogram);
    Assert(!*pprogram);

    const FFilename& filename = effectDescriptor->ProgramFilename(stage);
    if (filename.empty())
        return;

    *pprogram = new FEffectProgram(owner, effectDescriptor->ShaderProfile(), stage);
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
static const Graphics::FBlendState *BlendStateFromRenderState_(const FRenderState *renderState) {
    switch (renderState->Blend())
    {
    case FRenderState::EBlending::Additive:
        return Graphics::FBlendState::Additive;
    case FRenderState::EBlending::AlphaBlend:
        return Graphics::FBlendState::AlphaBlend;
    case FRenderState::EBlending::NonPremultiplied:
        return Graphics::FBlendState::NonPremultiplied;
    case FRenderState::EBlending::Opaque:
        return Graphics::FBlendState::Opaque;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
static const Graphics::FDepthStencilState *DepthStencilStateFromRenderState_(const FRenderState *renderState) {
    switch (renderState->Depth())
    {
    case FRenderState::EDepthTest::Default:
        return Graphics::FDepthStencilState::Default;
    case FRenderState::EDepthTest::None:
        return Graphics::FDepthStencilState::None;
    case FRenderState::EDepthTest::Read:
        return Graphics::FDepthStencilState::DepthRead;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
static const Graphics::FRasterizerState *RasterizerStateFromRenderState_(const FRenderState *renderState) {
    switch (renderState->Fill())
    {
    case FRenderState::EFillMode::Automatic:
        return nullptr; // accessed every time from FEffect::AutomaticRasterizerState
    case FRenderState::EFillMode::Wireframe:
        return Graphics::FRasterizerState::Wireframe;
    default:
        break;
    }

    switch (renderState->Cull())
    {
    case FRenderState::ECulling::Clockwise:
        return Graphics::FRasterizerState::CullClockwise;
    case FRenderState::ECulling::CounterClockwise:
        return Graphics::FRasterizerState::CullCounterClockwise;
    case FRenderState::ECulling::None:
        return Graphics::FRasterizerState::CullNone;
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
static void AppendTagSubstitutions_(
    VECTOR_THREAD_LOCAL(FEffect, TPair<FString COMMA FString>)& defines, 
    const ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, FString)& substitutions,
    const VECTOR(FEffect, Graphics::FBindName)& tags ) {

    for (const TPair<Graphics::FBindName, FString>& substitution : substitutions) {
        if (!Contains(tags, substitution.first))
            continue;

        Assert(!substitution.second.empty());
        const char *cstr = substitution.second.c_str();
        size_t size = substitution.second.size();

        FStringView define;
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
                FString key(define.Pointer(), std::distance(define.Pointer(), pValue) - 1);
                FString value(pValue, std::distance(pValue, define.end()));
                defines.emplace_back(std::move(key), std::move(value));
            }
            else {
                FString key(define.Pointer(), define.size());
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
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FEffect, );
//----------------------------------------------------------------------------
const Graphics::FRasterizerState *FEffect::AutomaticRasterizerState = nullptr;
const Graphics::FRasterizerState *FEffect::DefaultRasterizerState = nullptr;
//----------------------------------------------------------------------------
FEffect::FEffect( const FEffectDescriptor *descriptor,
                const Graphics::FVertexDeclaration *vertexDeclaration,
                const TMemoryView<const Graphics::FBindName>& tags )
:   FShaderEffect(vertexDeclaration)
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
FEffect::~FEffect() {}
//----------------------------------------------------------------------------
const Graphics::FRasterizerState *FEffect::FRasterizerState() const {
    return  _rasterizerState
        ?   _rasterizerState.get()
        :   AutomaticRasterizerState;
}
//----------------------------------------------------------------------------
FEffectProgram *FEffect::StageProgram(Graphics::EShaderProgramType stage) {
     return checked_cast<FEffectProgram *>(Graphics::FShaderEffect::StageProgram(stage));
}
//----------------------------------------------------------------------------
const FEffectProgram *FEffect::StageProgram(Graphics::EShaderProgramType stage) const {
     return checked_cast<const FEffectProgram *>(Graphics::FShaderEffect::StageProgram(stage));
}
//----------------------------------------------------------------------------
void FEffect::Create(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    using namespace Graphics;

    Graphics::IDeviceAPIShaderCompiler *const compiler = device->Encapsulator()->Compiler();
    const Graphics::EShaderCompilerFlags compilerFlags =
#ifdef _DEBUG
        Graphics::EShaderCompilerFlags::DefaultForDebug
#else
        Graphics::EShaderCompilerFlags::Default
#endif
        ;

    const Graphics::FVertexDeclaration *vertexDeclaration = this->VertexDeclaration();

    VECTOR_THREAD_LOCAL(FEffect, TPair<FString COMMA FString>) defines;
    defines.reserve(_descriptor->Defines().size() + _tags.size());
    defines.insert(defines.end(), _descriptor->Defines().begin(), _descriptor->Defines().end());
    AppendTagSubstitutions_(defines, _descriptor->Substitutions(), _tags);

    Unfreeze(); // enables FShaderEffect::SetStageProgram()

    for (EShaderProgramType stage : EachShaderProgramType()) {
        PEffectProgram program;
        CreateEffectProgramIFN_(&program, this, stage, vertexDeclaration, _descriptor, compilerFlags, MakeConstView(defines), compiler);

        if (program)
            FShaderEffect::SetStageProgram(stage, std::move(program));
    }

    Freeze(); // disables FShaderEffect::SetStageProgram()

    FShaderEffect::Create(device);
}
//----------------------------------------------------------------------------
void FEffect::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());

    using namespace Graphics;

    FShaderEffect::Destroy(device);

    for (EShaderProgramType stage : EachShaderProgramType())
        FShaderEffect::ResetStageProgram(stage);
}
//----------------------------------------------------------------------------
void FEffect::LinkReflectedData(
    FSharedConstantBufferFactory *sharedBufferFactory, 
    Graphics::IDeviceAPIShaderCompiler *compiler) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_sharedBuffers.empty());

    using namespace Graphics;

    for (EShaderProgramType stage : EachShaderProgramType()) {
        FEffectProgram *const program = StageProgram(stage);
        if (program)
            program->LinkReflectedData(_sharedBuffers, sharedBufferFactory, compiler);
    }
}
//----------------------------------------------------------------------------
void FEffect::UnlinkReflectedData(FSharedConstantBufferFactory *sharedBufferFactory) {
    THIS_THREADRESOURCE_CHECKACCESS();

    using namespace Graphics;

    for (EShaderProgramType stage : EachShaderProgramType()) {
        FEffectProgram *const program = StageProgram(stage);
        if (program)
            program->UnlinkReflectedData();
    }

    for (PSharedConstantBuffer& sharedBuffer : _sharedBuffers)
        sharedBufferFactory->ReleaseDestroyIFN(sharedBuffer);

    _sharedBuffers.clear();
}
//----------------------------------------------------------------------------
void FEffect::Set(Graphics::IDeviceAPIContext *context) const {
    context->SetBlendState(this->BlendState());
    context->SetDepthStencilState(this->DepthStencilState());
    context->SetRasterizerState(this->RasterizerState());
    context->SetShaderEffect(this);

    using namespace Graphics;

    for (EShaderProgramType stage : EachShaderProgramType()) {
        const FEffectProgram *const program = StageProgram(stage);
        if (program)
            program->Set(context, this);
    }
}
//----------------------------------------------------------------------------
void FEffect::SwitchAutomaticFillMode() {
    const bool wireframe = (AutomaticRasterizerState == DefaultRasterizerState);
    LOG(Info, L"[FEffect] Toggle {0} fill mode for automatic rasterizer state ...",
        wireframe ? "wireframe" : "solid");

    AutomaticRasterizerState = (wireframe)
        ? Graphics::FRasterizerState::Wireframe
        : DefaultRasterizerState;
}
//----------------------------------------------------------------------------
void FEffect::Start() {
    DefaultRasterizerState = Graphics::FRasterizerState::CullCounterClockwise;
    AutomaticRasterizerState = DefaultRasterizerState;
}
//----------------------------------------------------------------------------
void FEffect::Shutdown() {
    AutomaticRasterizerState = nullptr;
    DefaultRasterizerState = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
