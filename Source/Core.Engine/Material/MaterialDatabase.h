#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/IEffectPasses.h"
#include "Core.Engine/Material/MaterialParameter_fwd.h"

#include "Core/Container/HashMap.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
class Filename;

namespace Graphics {
class BindName;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MaterialDatabase : public Meta::ThreadResource {
public:
    explicit MaterialDatabase(const MaterialDatabase *parent = nullptr);
    ~MaterialDatabase();

    const MaterialDatabase *Parent() const { return _parent; }

    void BindParameter(const Graphics::BindName& name, IMaterialParameter *parameter, bool allowOverride = false);
    void UnbindParameter(const Graphics::BindName& name, const IMaterialParameter *parameter);
    bool TryGetParameter(const Graphics::BindName& name, IMaterialParameter **parameter) const;
    bool TryGetParameter(const Graphics::BindName& name, PMaterialParameter& parameter) const;

    void BindTexture(const Graphics::BindName& name, const Filename& path, bool allowOverride = false);
    void UnbindTexture(const Graphics::BindName& name, const Filename& path);
    bool TryGetTexture(const Graphics::BindName& name, Filename *path) const;

    void BindEffect(const Graphics::BindName& name, const IEffectPasses *effect, bool allowOverride = false);
    void UnbindEffect(const Graphics::BindName& name, const IEffectPasses *effect);
    bool TryGetEffect(const Graphics::BindName& name, const IEffectPasses **effect) const;
    bool TryGetEffect(const Graphics::BindName& name, PCEffectPasses& effect) const;

    void Clear();

private:
    const MaterialDatabase *_parent;

    HASHMAP(Material, Graphics::BindName, PMaterialParameter) _parameters;
    HASHMAP(Material, Graphics::BindName, Filename) _textures;
    HASHMAP(Material, Graphics::BindName, PCEffectPasses) _effects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
