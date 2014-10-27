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
Material::Material(const char *name)
:   _name(name) {
    Assert(name);
}
//----------------------------------------------------------------------------
Material::Material(
    const char *name,
    const MemoryView<const Pair<Graphics::BindName, Filename>>& textures,
    const MemoryView<const Pair<Graphics::BindName, PAbstractMaterialParameter>>& parameters )
:   _name(name)
,   _textures(textures.begin(), textures.end())
,   _parameters(parameters.begin(), parameters.end()) {
    Assert(name);
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
Material::~Material() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
