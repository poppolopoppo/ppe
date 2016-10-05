#include "stdafx.h"

#include "Material.h"

#include "IMaterialParameter.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FMaterial, );
//----------------------------------------------------------------------------
FMaterial::FMaterial(const Graphics::FBindName& name)
:   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
FMaterial::FMaterial(
    const Graphics::FBindName& name,
    const FString& description,
    VECTOR(FMaterial, Graphics::FBindName)&& tags,
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename)&& textures,
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, PMaterialParameter)&& parameters )
:   _name(name)
,   _description(description)
,   _tags(std::move(tags))
,   _textures(std::move(textures))
,   _parameters(std::move(parameters)) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
FMaterial::~FMaterial() {}
//----------------------------------------------------------------------------
void FMaterial::AddTag(const Graphics::FBindName& name) {
    Assert(!name.empty());

    Add_AssertUnique(_tags, name);
}
//----------------------------------------------------------------------------
void FMaterial::AddTexture(const Graphics::FBindName& name, const FFilename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Insert_AssertUnique(name, filename);
}
//----------------------------------------------------------------------------
void FMaterial::AddParameter(const Graphics::FBindName& name, const PMaterialParameter& parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters.Insert_AssertUnique(name, parameter);
}
//----------------------------------------------------------------------------
void FMaterial::SetTexture(const Graphics::FBindName& name, const FFilename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Get(name) = filename;
}
//----------------------------------------------------------------------------
void FMaterial::SetParameter(const Graphics::FBindName& name, const PMaterialParameter& parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters[name] = parameter;
}
//----------------------------------------------------------------------------
bool FMaterial::Equals(const FMaterial& other) const {
    return (_name == other._name && 
            _tags == other._tags &&
            _textures == other._textures &&
            _parameters == other._parameters );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
