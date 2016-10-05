#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/IEffectPasses.h"
#include "Core.Engine/Material/MaterialParameter_fwd.h"

#include "Core/Container/HashMap.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
class FFilename;

namespace Graphics {
class FBindName;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMaterialDatabase : public Meta::FThreadResource {
public:
    explicit FMaterialDatabase(const FMaterialDatabase *parent = nullptr);
    ~FMaterialDatabase();

    const FMaterialDatabase *Parent() const { return _parent; }

    void BindParameter(const Graphics::FBindName& name, IMaterialParameter *parameter, bool allowOverride = false);
    void UnbindParameter(const Graphics::FBindName& name, const IMaterialParameter *parameter);
    bool TryGetParameter(const Graphics::FBindName& name, IMaterialParameter **parameter) const;
    bool TryGetParameter(const Graphics::FBindName& name, PMaterialParameter& parameter) const;

    void BindTexture(const Graphics::FBindName& name, const FFilename& path, bool allowOverride = false);
    void UnbindTexture(const Graphics::FBindName& name, const FFilename& path);
    bool TryGetTexture(const Graphics::FBindName& name, FFilename *path) const;

    void BindEffect(const Graphics::FBindName& name, const IEffectPasses *effect, bool allowOverride = false);
    void UnbindEffect(const Graphics::FBindName& name, const IEffectPasses *effect);
    bool TryGetEffect(const Graphics::FBindName& name, const IEffectPasses **effect) const;
    bool TryGetEffect(const Graphics::FBindName& name, PCEffectPasses& effect) const;

    void Clear();

private:
    const FMaterialDatabase *_parent;

    HASHMAP(FMaterial, Graphics::FBindName, PMaterialParameter) _parameters;
    HASHMAP(FMaterial, Graphics::FBindName, FFilename) _textures;
    HASHMAP(FMaterial, Graphics::FBindName, PCEffectPasses) _effects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
