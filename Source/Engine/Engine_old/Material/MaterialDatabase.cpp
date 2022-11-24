// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MaterialDatabase.h"

#include "Effect/EffectDescriptor.h"
#include "IMaterialParameter.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMaterialDatabase::FMaterialDatabase(const FMaterialDatabase *parent /* = nullptr */)
:   _parent(parent) {}
//----------------------------------------------------------------------------
FMaterialDatabase::~FMaterialDatabase() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FMaterialDatabase::BindParameter(const Graphics::FBindName& name, IMaterialParameter *parameter, bool allowOverride) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(parameter);

    if (!allowOverride)
        AssertRelease(_parameters.end() == _parameters.find(name));

    _parameters[name] = parameter;
}
//----------------------------------------------------------------------------
void FMaterialDatabase::UnbindParameter(const Graphics::FBindName& name, const IMaterialParameter *parameter) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(parameter);

    const auto it = _parameters.find(name);
    Assert(_parameters.end() != it);
    AssertRelease(it->second.get() == parameter);

    _parameters.erase(it);
}
//----------------------------------------------------------------------------
bool FMaterialDatabase::TryGetParameter(const Graphics::FBindName& name, IMaterialParameter **parameter) const {
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
bool FMaterialDatabase::TryGetParameter(const Graphics::FBindName& name, PMaterialParameter& parameter) const {
    IMaterialParameter *p = nullptr;
    const bool result = TryGetParameter(name, &p);

    parameter = p;
    return result;
}
//----------------------------------------------------------------------------
void FMaterialDatabase::BindTexture( const Graphics::FBindName& name, const FFilename& path,
                                    bool allowOverride /* = false */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(!path.empty());

    if (!allowOverride)
        AssertRelease(_textures.end() == _textures.find(name));

    _textures[name] = path;
}
//----------------------------------------------------------------------------
void FMaterialDatabase::UnbindTexture(const Graphics::FBindName& name, const FFilename& path) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(!path.empty());

    const auto it = _textures.find(name);
    Assert(_textures.end() != it);
    AssertRelease(it->second == path);

    _textures.erase(it);
}
//----------------------------------------------------------------------------
bool FMaterialDatabase::TryGetTexture(const Graphics::FBindName& name, FFilename *path) const {
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
void FMaterialDatabase::BindEffect(const Graphics::FBindName& name, const IEffectPasses *effect, bool allowOverride /* = false */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(effect);

    if (!allowOverride)
        AssertRelease(_effects.end() == _effects.find(name));

    _effects[name] = effect;
}
//----------------------------------------------------------------------------
void FMaterialDatabase::UnbindEffect(const Graphics::FBindName& name, const IEffectPasses *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!name.empty());
    Assert(effect);

    const auto it = _effects.find(name);
    Assert(_effects.end() != it);
    AssertRelease(it->second.get() == effect);

    _effects.erase(it);
}
//----------------------------------------------------------------------------
bool FMaterialDatabase::TryGetEffect(const Graphics::FBindName& name, const IEffectPasses **effect) const {
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
bool FMaterialDatabase::TryGetEffect(const Graphics::FBindName& name, PCEffectPasses& effect) const {
    const IEffectPasses *e = nullptr;
    const bool result = TryGetEffect(name, &e);

    effect = e;
    return result;
}
//----------------------------------------------------------------------------
void FMaterialDatabase::Clear() {
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
