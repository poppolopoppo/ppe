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
class FBindName;
enum class EShaderProfileType;
enum class EShaderProgramType;
FWD_REFPTR(VertexDeclaration);
}

namespace Engine {
FWD_REFPTR(RenderState);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEffectDescriptor : public IEffectPasses {
public:
    FEffectDescriptor();
    virtual ~FEffectDescriptor();

    FEffectDescriptor(   const char *name,
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
                        const TMemoryView<const TPair<Graphics::FBindName, FFilename>>& textures );

    const Engine::FRenderState *FRenderState() const { return _renderState.get(); }
    void SetRenderState(const Engine::FRenderState *value);

    const FFilename& HS() const { return _hs; }
    const FFilename& DS() const { return _ds; }
    const FFilename& GS() const { return _gs; }
    const FFilename& VS() const { return _vs; }
    const FFilename& PS() const { return _ps; }
    const FFilename& CS() const { return _cs; }

    void SetHS(const FFilename& filename) { _hs = filename; }
    void SetDS(const FFilename& filename) { _ds = filename; }
    void SetGS(const FFilename& filename) { _gs = filename; }
    void SetVS(const FFilename& filename) { _vs = filename; }
    void SetPS(const FFilename& filename) { _ps = filename; }
    void SetCS(const FFilename& filename) { _cs = filename; }

    const FString& Name() const { return _name; }
    void SetName(const char *name);

    const FFilename& ProgramFilename(Graphics::EShaderProgramType programType) const;
    void SetProgramFilename(Graphics::EShaderProgramType, const FFilename& filename);

    Graphics::EShaderProfileType ShaderProfile() const { return _shaderProfile; }
    void SetShaderProfile(Graphics::EShaderProfileType value) { _shaderProfile = value; }

    const VECTOR(FEffect, Graphics::PCVertexDeclaration)& VertexDeclarations() const { return _vertexDeclarations; }

    const ASSOCIATIVE_VECTOR(FEffect, FString, FString)& Defines() const { return _defines; }
    const ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, FString)& Substitutions() const { return _substitutions; }
    const ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, PMaterialParameter)& Parameters() const { return _parameters; }
    const ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, FFilename)& Textures() const { return _textures; }

    size_t RenderLayerOffset() const { return _renderLayerOffset; }
    void SetRenderLayerOffset(size_t value);

    void AddVertexDeclaration(const Graphics::FVertexDeclaration *declaration);
    void AddDefine(const FString& name, const FString& value);
    void AddSubstitution(const Graphics::FBindName& tag, const FString& defines);
    void AddTexture(const Graphics::FBindName& name, const FFilename& filename);
    void AddParameter(const Graphics::FBindName& name, IMaterialParameter *parameter);

    virtual size_t FillEffectPasses(const FEffectDescriptor **pOutPasses, const size_t capacity) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FString _name;

    PCRenderState _renderState;

    FFilename _hs;
    FFilename _ds;
    FFilename _gs;
    FFilename _vs;
    FFilename _ps;
    FFilename _cs;

    Graphics::EShaderProfileType _shaderProfile;

    VECTOR(FEffect, Graphics::PCVertexDeclaration) _vertexDeclarations;

    ASSOCIATIVE_VECTOR(FEffect, FString, FString) _defines;
    ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, FString) _substitutions;
    ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, FFilename) _textures;
    ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, PMaterialParameter) _parameters;

    size_t _renderLayerOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
