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
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, EffectDescriptor, );
//----------------------------------------------------------------------------
EffectDescriptor::EffectDescriptor() 
:   _renderLayerOffset(0) {}
//----------------------------------------------------------------------------
EffectDescriptor::~EffectDescriptor() {}
//----------------------------------------------------------------------------
EffectDescriptor::EffectDescriptor(
    const char *name,
    const Engine::RenderState *renderState,
    const Filename& hs,
    const Filename& ds,
    const Filename& gs,
    const Filename& vs,
    const Filename& ps,
    const Filename& cs,
    Graphics::ShaderProfileType shaderProfile,
    const Graphics::VertexDeclaration *vertexDeclaration,
    const MemoryView<const Pair<String, String>>& defines,
    const MemoryView<const Pair<Graphics::BindName, String>>& substitutions,
    const MemoryView<const Pair<Graphics::BindName, PMaterialParameter>>& parameters,
    const MemoryView<const Pair<Graphics::BindName, Filename>>& textures )
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
void EffectDescriptor::SetName(const char *name) {
    Assert(name);
    _name = name;
}
//----------------------------------------------------------------------------
void EffectDescriptor::SetRenderState(const Engine::RenderState *value) {
    _renderState = value;
}
//----------------------------------------------------------------------------
const Filename& EffectDescriptor::ProgramFilename(Graphics::ShaderProgramType programType) const {
    switch (programType)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return _vs;
    case Core::Graphics::ShaderProgramType::Hull:
        return _hs;
    case Core::Graphics::ShaderProgramType::Domain:
        return _ds;
    case Core::Graphics::ShaderProgramType::Pixel:
        return _ps;
    case Core::Graphics::ShaderProgramType::Geometry:
        return _gs;
    case Core::Graphics::ShaderProgramType::Compute:
        return _cs;
    default:
        AssertNotImplemented();
        return _vs; // const ref expected ...
    }
}
//----------------------------------------------------------------------------
void EffectDescriptor::SetProgramFilename(Graphics::ShaderProgramType programType, const Filename& filename) {
    switch (programType)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        _vs = filename;
        break;
    case Core::Graphics::ShaderProgramType::Hull:
        _hs = filename;
        break;
    case Core::Graphics::ShaderProgramType::Domain:
        _ds = filename;
        break;
    case Core::Graphics::ShaderProgramType::Pixel:
        _ps = filename;
        break;
    case Core::Graphics::ShaderProgramType::Geometry:
        _gs = filename;
        break;
    case Core::Graphics::ShaderProgramType::Compute:
        _cs = filename;
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
void EffectDescriptor::SetRenderLayerOffset(size_t value) {
    _renderLayerOffset = value;
}
//----------------------------------------------------------------------------
void EffectDescriptor::AddVertexDeclaration(const Graphics::VertexDeclaration *declaration) {
    Assert(declaration);

    Graphics::PCVertexDeclaration pdeclaration(declaration);
    Assert(!Contains(_vertexDeclarations, pdeclaration));

    _vertexDeclarations.emplace_back(std::move(pdeclaration));
}
//----------------------------------------------------------------------------
void EffectDescriptor::AddDefine(const String& name, const String& value) {
    Assert(!name.empty());
    Assert(!value.empty());

    _defines.Insert_AssertUnique(name, value);
}
//----------------------------------------------------------------------------
void EffectDescriptor::AddSubstitution(const Graphics::BindName& tag, const String& defines) {
    Assert(!tag.empty());
    Assert(!defines.empty());

    _substitutions.Insert_AssertUnique(tag, defines);
}
//----------------------------------------------------------------------------
void EffectDescriptor::AddTexture(const Graphics::BindName& name, const Filename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Insert_AssertUnique(name, filename);
}
//----------------------------------------------------------------------------
void EffectDescriptor::AddParameter(const Graphics::BindName& name, IMaterialParameter *parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters.Insert_AssertUnique(name, parameter);
}
//----------------------------------------------------------------------------
size_t EffectDescriptor::FillEffectPasses(const EffectDescriptor **pOutPasses, const size_t capacity) const {
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
