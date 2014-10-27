#include "stdafx.h"

#include "MaterialDatabase.h"

#include "Effect/EffectDescriptor.h"
#include "Parameters/AbstractMaterialParameter.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MaterialDatabase::MaterialDatabase(const MaterialDatabase *parent /* = nullptr */)
:   _parent(parent) {}
//----------------------------------------------------------------------------
MaterialDatabase::~MaterialDatabase() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void MaterialDatabase::BindParameter(const Graphics::BindName& name, AbstractMaterialParameter *parameter, bool allowOverride) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(parameter);

    if (!allowOverride)
        AssertRelease(_parameters.end() == _parameters.find(name));

    _parameters[name] = parameter;
}
//----------------------------------------------------------------------------
void MaterialDatabase::UnbindParameter(const Graphics::BindName& name, const AbstractMaterialParameter *parameter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(parameter);

    const auto it = _parameters.find(name);
    Assert(_parameters.end() != it);
    AssertRelease(it->second.get() == parameter);

    _parameters.erase(it);
}
//----------------------------------------------------------------------------
bool MaterialDatabase::TryGetParameter(const Graphics::BindName& name, AbstractMaterialParameter **parameter) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(parameter);

    const auto it = _parameters.find(name);
    if (_parameters.end() == it) {
        return (_parent != nullptr &&
                _parent->TryGetParameter(name, parameter) );
    }

    *parameter = it->second.get();
    Assert(*parameter);

    return true;
}
//----------------------------------------------------------------------------
bool MaterialDatabase::TryGetParameter(const Graphics::BindName& name, PAbstractMaterialParameter& parameter) const {
    AbstractMaterialParameter *p = nullptr;
    const bool result = TryGetParameter(name, &p);

    parameter = p;
    return result;
}
//----------------------------------------------------------------------------
void MaterialDatabase::BindTexture( const Graphics::BindName& name, const Filename& path,
                                    bool allowOverride /* = false */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(!path.empty());

    if (!allowOverride)
        AssertRelease(_textures.end() == _textures.find(name));

    _textures[name] = path;
}
//----------------------------------------------------------------------------
void MaterialDatabase::UnbindTexture(const Graphics::BindName& name, const Filename& path) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(!path.empty());

    const auto it = _textures.find(name);
    Assert(_textures.end() != it);
    AssertRelease(it->second == path);

    _textures.erase(it);
}
//----------------------------------------------------------------------------
bool MaterialDatabase::TryGetTexture(const Graphics::BindName& name, Filename *path) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(path);

    const auto it = _textures.find(name);
    if (_textures.end() == it) {
        return (_parent != nullptr &&
                _parent->TryGetTexture(name, path) );
    }

    *path = it->second;
    Assert(!path->empty());

    return true;
}
//----------------------------------------------------------------------------
void MaterialDatabase::BindEffect(const Graphics::BindName& name, EffectDescriptor *effect, bool allowOverride) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(effect);

    if (!allowOverride)
        AssertRelease(_effects.end() == _effects.find(name));

    _effects[name] = effect;
}
//----------------------------------------------------------------------------
void MaterialDatabase::UnbindEffect(const Graphics::BindName& name, const EffectDescriptor *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(effect);

    const auto it = _effects.find(name);
    Assert(_effects.end() != it);
    AssertRelease(it->second.get() == effect);

    _effects.erase(it);
}
//----------------------------------------------------------------------------
bool MaterialDatabase::TryGetEffect(const Graphics::BindName& name, EffectDescriptor **effect) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(effect);

    const auto it = _effects.find(name);
    if (_effects.end() == it) {
        return (_parent != nullptr &&
                _parent->TryGetEffect(name, effect) );
    }

    *effect = it->second.get();
    Assert(*effect);

    return true;
}
//----------------------------------------------------------------------------
bool MaterialDatabase::TryGetEffect(const Graphics::BindName& name, PEffectDescriptor& effect) const {
    EffectDescriptor *e = nullptr;
    const bool result = TryGetEffect(name, &e);

    effect = e;
    return result;
}
//----------------------------------------------------------------------------
void MaterialDatabase::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _effects.clear();
    _textures.clear();
    _parameters.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
