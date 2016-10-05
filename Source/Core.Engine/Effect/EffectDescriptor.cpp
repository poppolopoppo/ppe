#include "stdafx.h"

#include "EffectDescriptor.h"

#include "Material/IMaterialParameter.h"
#include "Render/RenderState.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Shader/ShaderEffect.h"
#include "Core.Graphics/Device/Shader/ShaderProgram.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FEffectDescriptor, );
//----------------------------------------------------------------------------
FEffectDescriptor::FEffectDescriptor() 
:   _renderLayerOffset(0) {}
//----------------------------------------------------------------------------
FEffectDescriptor::~FEffectDescriptor() {}
//----------------------------------------------------------------------------
FEffectDescriptor::FEffectDescriptor(
    const char *name,
    const Engine::FRenderState *renderState,
    const FFilename& hs,
    const FFilename& ds,
    const FFilename& gs,
    const FFilename& vs,
    const FFilename& ps,
    const FFilename& cs,
    Graphics::EShaderProfileType shaderProfile,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    const TMemoryView<const TPair<FString, FString>>& defines,
    const TMemoryView<const TPair<Graphics::FBindName, FString>>& substitutions,
    const TMemoryView<const TPair<Graphics::FBindName, PMaterialParameter>>& parameters,
    const TMemoryView<const TPair<Graphics::FBindName, FFilename>>& textures )
:   _name(name)
,   _renderState(renderState)
,   _hs(hs)
,   _ds(ds)
,   _gs(gs)
,   _vs(vs)
,   _ps(ps)
,   _cs(cs)
,   _shaderProfile(shaderProfile)
,   _defines(defines.begin(), defines.end())
,   _substitutions(substitutions.begin(), substitutions.end())
,   _parameters(parameters.begin(), parameters.end())
,   _textures(textures.begin(), textures.end()) 
,   _renderLayerOffset(0) {
    Assert(name);
    Assert( !hs.empty() ||
            !ds.empty() ||
            !gs.empty() ||
            !vs.empty() ||
            !ps.empty() ||
            !cs.empty() );
    AddVertexDeclaration(vertexDeclaration);
}
//----------------------------------------------------------------------------
void FEffectDescriptor::SetName(const char *name) {
    Assert(name);
    _name = name;
}
//----------------------------------------------------------------------------
void FEffectDescriptor::SetRenderState(const Engine::FRenderState *value) {
    _renderState = value;
}
//----------------------------------------------------------------------------
const FFilename& FEffectDescriptor::ProgramFilename(Graphics::EShaderProgramType programType) const {
    switch (programType)
    {
    case Core::Graphics::EShaderProgramType::Vertex:
        return _vs;
    case Core::Graphics::EShaderProgramType::Hull:
        return _hs;
    case Core::Graphics::EShaderProgramType::Domain:
        return _ds;
    case Core::Graphics::EShaderProgramType::Pixel:
        return _ps;
    case Core::Graphics::EShaderProgramType::Geometry:
        return _gs;
    case Core::Graphics::EShaderProgramType::Compute:
        return _cs;
    default:
        AssertNotImplemented();
        return _vs; // const ref expected ...
    }
}
//----------------------------------------------------------------------------
void FEffectDescriptor::SetProgramFilename(Graphics::EShaderProgramType programType, const FFilename& filename) {
    switch (programType)
    {
    case Core::Graphics::EShaderProgramType::Vertex:
        _vs = filename;
        break;
    case Core::Graphics::EShaderProgramType::Hull:
        _hs = filename;
        break;
    case Core::Graphics::EShaderProgramType::Domain:
        _ds = filename;
        break;
    case Core::Graphics::EShaderProgramType::Pixel:
        _ps = filename;
        break;
    case Core::Graphics::EShaderProgramType::Geometry:
        _gs = filename;
        break;
    case Core::Graphics::EShaderProgramType::Compute:
        _cs = filename;
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void FEffectDescriptor::SetRenderLayerOffset(size_t value) {
    _renderLayerOffset = value;
}
//----------------------------------------------------------------------------
void FEffectDescriptor::AddVertexDeclaration(const Graphics::FVertexDeclaration *declaration) {
    Assert(declaration);

    Graphics::PCVertexDeclaration pdeclaration(declaration);
    Assert(!Contains(_vertexDeclarations, pdeclaration));

    _vertexDeclarations.emplace_back(std::move(pdeclaration));
}
//----------------------------------------------------------------------------
void FEffectDescriptor::AddDefine(const FString& name, const FString& value) {
    Assert(!name.empty());
    Assert(!value.empty());

    _defines.Insert_AssertUnique(name, value);
}
//----------------------------------------------------------------------------
void FEffectDescriptor::AddSubstitution(const Graphics::FBindName& tag, const FString& defines) {
    Assert(!tag.empty());
    Assert(!defines.empty());

    _substitutions.Insert_AssertUnique(tag, defines);
}
//----------------------------------------------------------------------------
void FEffectDescriptor::AddTexture(const Graphics::FBindName& name, const FFilename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Insert_AssertUnique(name, filename);
}
//----------------------------------------------------------------------------
void FEffectDescriptor::AddParameter(const Graphics::FBindName& name, IMaterialParameter *parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters.Insert_AssertUnique(name, parameter);
}
//----------------------------------------------------------------------------
size_t FEffectDescriptor::FillEffectPasses(const FEffectDescriptor **pOutPasses, const size_t capacity) const {
    Assert(pOutPasses);
    Assert(capacity > 0);

    *pOutPasses = this;
    return 1;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
