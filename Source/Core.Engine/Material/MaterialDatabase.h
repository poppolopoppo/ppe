#pragma once

#include "Core.Engine/Engine.h"

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
FWD_REFPTR(AbstractMaterialParameter);
FWD_REFPTR(EffectDescriptor);
//----------------------------------------------------------------------------
class MaterialDatabase : public Meta::ThreadResource {
public:
    explicit MaterialDatabase(const MaterialDatabase *parent = nullptr);
    ~MaterialDatabase();

    const MaterialDatabase *Parent() const { return _parent; }

    void BindParameter(const Graphics::BindName& name, AbstractMaterialParameter *parameter, bool allowOverride = false);
    void UnbindParameter(const Graphics::BindName& name, const AbstractMaterialParameter *parameter);
    bool TryGetParameter(const Graphics::BindName& name, AbstractMaterialParameter **parameter) const;
    bool TryGetParameter(const Graphics::BindName& name, PAbstractMaterialParameter& parameter) const;

    void BindTexture(const Graphics::BindName& name, const Filename& path, bool allowOverride = false);
    void UnbindTexture(const Graphics::BindName& name, const Filename& path);
    bool TryGetTexture(const Graphics::BindName& name, Filename *path) const;

    void BindEffect(const Graphics::BindName& name, EffectDescriptor *effect, bool allowOverride = false);
    void UnbindEffect(const Graphics::BindName& name, const EffectDescriptor *effect);
    bool TryGetEffect(const Graphics::BindName& name, EffectDescriptor **effect) const;
    bool TryGetEffect(const Graphics::BindName& name, PEffectDescriptor& effect) const;

    void Clear();

private:
    const MaterialDatabase *_parent;

    HASHMAP(Material, Graphics::BindName, PAbstractMaterialParameter) _parameters;
    HASHMAP(Material, Graphics::BindName, Filename) _textures;
    HASHMAP(Material, Graphics::BindName, PEffectDescriptor) _effects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
