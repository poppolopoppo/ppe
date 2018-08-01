#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/IEffectPasses.h"
#include "Core.Engine/Material/MaterialParameter_fwd.h"

#include "Core/Container/HashMap.h"

namespace Core {
class FFilename;

namespace Graphics {
class FBindName;
}

namespace Engine {
FWD_REFPTR(EffectConstantBuffer);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EMaterialPotType {
    Constant    = 0,
    FEffect,
    Parameter,
    FTexture,
};
//----------------------------------------------------------------------------
template <EMaterialPotType _Type, typename _Key, typename _Value>
class TBaseMaterialPot {
public:
    typedef key_type _Key;
    typedef value_type _Value;
    typedef HASHMAP(FMaterial, _Key, _Value) container_type;

    TBaseMaterialPot(const TBaseMaterialPot *parent = nullptr)
    ~TBaseMaterialPot();

    EMaterialPotType EType() const { return _Type; }

    const TBaseMaterialPot *Parent() const { return _parent; }
    const container_type& Values() const { return _values; }

    void Bind(const _Key& key, const T& value, bool allowOverride = false);
    void Unbind(const _Key& key, const T& value);

    bool TryGet(T *pvalue, const _Key& key, bool allowParentRecursion = true) const;

    void Clear();

private:
    container_type _values;
    const TBaseMaterialPot *_parent;
};
//----------------------------------------------------------------------------
struct FMaterialConstantKey;
struct FMaterialEffectKey;
struct FMaterialParameterKey;
struct FMaterialTextureKey;
//----------------------------------------------------------------------------
extern template class TBaseMaterialPot<EMaterialPotType::Constant,    FMaterialConstantKey,    PEffectConstantBuffer >;
extern template class TBaseMaterialPot<EMaterialPotType::FEffect,      FMaterialEffectKey,      PCEffectPasses >;
extern template class TBaseMaterialPot<EMaterialPotType::Parameter,   FMaterialParameterKey,   PMaterialParameter >;
extern template class TBaseMaterialPot<EMaterialPotType::FTexture,     FMaterialTextureKey,     FFilename >;
//----------------------------------------------------------------------------
typedef TBaseMaterialPot<EMaterialPotType::Constant,    FMaterialConstantKey,    PEffectConstantBuffer >   MaterialPotConstant;
typedef TBaseMaterialPot<EMaterialPotType::FEffect,      FMaterialEffectKey,      PCEffectPasses >          MaterialPotEffect;
typedef TBaseMaterialPot<EMaterialPotType::Parameter,   FMaterialParameterKey,   PMaterialParameter >      MaterialPotParameter;
typedef TBaseMaterialPot<EMaterialPotType::FTexture,     FMaterialTextureKey,     FFilename >                MaterialPotTexture;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
