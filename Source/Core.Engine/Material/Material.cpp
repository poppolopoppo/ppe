#include "stdafx.h"

#include "Material.h"

#include "Parameters/AbstractMaterialParameter.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Material, );
//----------------------------------------------------------------------------
Material::Material(const Graphics::BindName& name)
:   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
Material::Material(
    const Graphics::BindName& name,
    const String& description,
    VECTOR(Material, Graphics::BindName)&& tags,
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename)&& textures,
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PAbstractMaterialParameter)&& parameters )
:   _name(name)
,   _description(description)
,   _tags(std::move(tags))
,   _textures(std::move(textures))
,   _parameters(std::move(parameters)) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
Material::~Material() {}
//----------------------------------------------------------------------------
void Material::AddTag(const Graphics::BindName& name) {
    Assert(!name.empty());

    Insert_AssertUnique(_tags, name);
}
//----------------------------------------------------------------------------
void Material::AddTexture(const Graphics::BindName& name, const Filename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Insert_AssertUnique(name, filename);
}
//----------------------------------------------------------------------------
void Material::AddParameter(const Graphics::BindName& name, const PAbstractMaterialParameter& parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters.Insert_AssertUnique(name, parameter);
}
//----------------------------------------------------------------------------
void Material::SetTexture(const Graphics::BindName& name, const Filename& filename) {
    Assert(!name.empty());
    Assert(!filename.empty());

    _textures.Get(name) = filename;
}
//----------------------------------------------------------------------------
void Material::SetParameter(const Graphics::BindName& name, const PAbstractMaterialParameter& parameter) {
    Assert(!name.empty());
    Assert(parameter);

    _parameters[name] = parameter;
}
//----------------------------------------------------------------------------
bool Material::Equals(const Material& other) const {
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
