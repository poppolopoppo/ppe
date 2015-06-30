#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

#include "Core.Engine/Effect/IEffectPasses.h"
#include "Core.Engine/Material/MaterialParameter_fwd.h"

namespace Core {
namespace Graphics {
class BindName;
enum class ShaderProfileType;
enum class ShaderProgramType;
FWD_REFPTR(VertexDeclaration);
}

namespace Engine {
FWD_REFPTR(RenderState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EffectDescriptor : public IEffectPasses {
public:
    EffectDescriptor();
    virtual ~EffectDescriptor();

    EffectDescriptor(   const char *name,
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
                        const MemoryView<const Pair<Graphics::BindName, Filename>>& textures );

    const Engine::RenderState *RenderState() const { return _renderState.get(); }
    void SetRenderState(const Engine::RenderState *value);

    const Filename& HS() const { return _hs; }
    const Filename& DS() const { return _ds; }
    const Filename& GS() const { return _gs; }
    const Filename& VS() const { return _vs; }
    const Filename& PS() const { return _ps; }
    const Filename& CS() const { return _cs; }

    void SetHS(const Filename& filename) { _hs = filename; }
    void SetDS(const Filename& filename) { _ds = filename; }
    void SetGS(const Filename& filename) { _gs = filename; }
    void SetVS(const Filename& filename) { _vs = filename; }
    void SetPS(const Filename& filename) { _ps = filename; }
    void SetCS(const Filename& filename) { _cs = filename; }

    const String& Name() const { return _name; }
    void SetName(const char *name);

    const Filename& ProgramFilename(Graphics::ShaderProgramType programType) const;
    void SetProgramFilename(Graphics::ShaderProgramType, const Filename& filename);

    Graphics::ShaderProfileType ShaderProfile() const { return _shaderProfile; }
    void SetShaderProfile(Graphics::ShaderProfileType value) { _shaderProfile = value; }

    const VECTOR(Effect, Graphics::PCVertexDeclaration)& VertexDeclarations() const { return _vertexDeclarations; }

    const ASSOCIATIVE_VECTOR(Effect, String, String)& Defines() const { return _defines; }
    const ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, String)& Substitutions() const { return _substitutions; }
    const ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, PMaterialParameter)& Parameters() const { return _parameters; }
    const ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, Filename)& Textures() const { return _textures; }

    size_t RenderLayerOffset() const { return _renderLayerOffset; }
    void SetRenderLayerOffset(size_t value);

    void AddVertexDeclaration(const Graphics::VertexDeclaration *declaration);
    void AddDefine(const String& name, const String& value);
    void AddSubstitution(const Graphics::BindName& tag, const String& defines);
    void AddTexture(const Graphics::BindName& name, const Filename& filename);
    void AddParameter(const Graphics::BindName& name, IMaterialParameter *parameter);

    virtual size_t FillEffectPasses(const EffectDescriptor **pOutPasses, const size_t capacity) const override;

    SINGLETON_POOL_ALLOCATED_DECL(EffectDescriptor);

private:
    String _name;

    PCRenderState _renderState;

    Filename _hs;
    Filename _ds;
    Filename _gs;
    Filename _vs;
    Filename _ps;
    Filename _cs;

    Graphics::ShaderProfileType _shaderProfile;

    VECTOR(Effect, Graphics::PCVertexDeclaration) _vertexDeclarations;

    ASSOCIATIVE_VECTOR(Effect, String, String) _defines;
    ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, String) _substitutions;
    ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, Filename) _textures;
    ASSOCIATIVE_VECTOR(Effect, Graphics::BindName, PMaterialParameter) _parameters;

    size_t _renderLayerOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
