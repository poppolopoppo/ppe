#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/IEffectPasses.h"
#include "Core.Engine/Material/MaterialParameter_fwd.h"

#include "Core/Container/HashMap.h"

namespace Core {
class Filename;

namespace Graphics {
class BindName;
}

namespace Engine {
FWD_REFPTR(EffectConstantBuffer);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class MaterialPotType {
    Constant    = 0,
    Effect,
    Parameter,
    Texture,
};
//----------------------------------------------------------------------------
template <MaterialPotType _Type, typename _Key, typename _Value>
class BaseMaterialPot {
public:
    typedef key_type _Key;
    typedef value_type _Value;
    typedef HASHMAP(Material, _Key, _Value) container_type;

    BaseMaterialPot(const BaseMaterialPot *parent = nullptr)
    ~BaseMaterialPot();

    MaterialPotType Type() const { return _Type; }

    const BaseMaterialPot *Parent() const { return _parent; }
    const container_type& Values() const { return _values; }

    void Bind(const _Key& key, const T& value, bool allowOverride = false);
    void Unbind(const _Key& key, const T& value);

    bool TryGet(T *pvalue, const _Key& key, bool allowParentRecursion = true) const;

    void Clear();

private:
    container_type _values;
    const BaseMaterialPot *_parent;
};
//----------------------------------------------------------------------------
struct MaterialConstantKey;
struct MaterialEffectKey;
struct MaterialParameterKey;
struct MaterialTextureKey;
//----------------------------------------------------------------------------
extern template class BaseMaterialPot<MaterialPotType::Constant,    MaterialConstantKey,    PEffectConstantBuffer >;
extern template class BaseMaterialPot<MaterialPotType::Effect,      MaterialEffectKey,      PCEffectPasses >;
extern template class BaseMaterialPot<MaterialPotType::Parameter,   MaterialParameterKey,   PMaterialParameter >;
extern template class BaseMaterialPot<MaterialPotType::Texture,     MaterialTextureKey,     Filename >;
//----------------------------------------------------------------------------
typedef BaseMaterialPot<MaterialPotType::Constant,    MaterialConstantKey,    PEffectConstantBuffer >   MaterialPotConstant;
typedef BaseMaterialPot<MaterialPotType::Effect,      MaterialEffectKey,      PCEffectPasses >          MaterialPotEffect;
typedef BaseMaterialPot<MaterialPotType::Parameter,   MaterialParameterKey,   PMaterialParameter >      MaterialPotParameter;
typedef BaseMaterialPot<MaterialPotType::Texture,     MaterialTextureKey,     Filename >                MaterialPotTexture;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
