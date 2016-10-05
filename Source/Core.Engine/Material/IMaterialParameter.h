#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialParameter_fwd.h"

namespace Core {

namespace Graphics {
class FBindName;
struct FConstantField;
}

namespace Engine {
class FMaterialDatabase;
FWD_REFPTR(MaterialEffect);
FWD_REFPTR(Scene);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMaterialParameterInfo {
    Graphics::EConstantFieldType EType;
    EMaterialVariability Variability;
};
//----------------------------------------------------------------------------
struct FMaterialParameterContext {
    SCScene FScene;
    SCMaterialEffect FMaterialEffect;
    const FMaterialDatabase *Database;
};
//----------------------------------------------------------------------------
struct FMaterialParameterMutableContext {
    SCScene FScene;
    SMaterialEffect FMaterialEffect;
    FMaterialDatabase *Database;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMaterialParameter : public FRefCountable {
public:
    virtual ~IMaterialParameter() {}

    virtual FMaterialParameterInfo Info() const = 0;

    virtual void Eval(const FMaterialParameterContext& context, void *dst, size_t sizeInBytes) = 0;

    template <typename T>
    ITypedMaterialParameter<T> *Cast() { return checked_cast<ITypedMaterialParameter<T> *>(this); }
    template <typename T>
    const ITypedMaterialParameter<T> *Cast() const { return checked_cast<ITypedMaterialParameter<T> *>(this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ITypedMaterialParameter : public IMaterialParameter {
public:
    virtual ~ITypedMaterialParameter() {}

    Graphics::EConstantFieldType EType() const { return Graphics::ConstantFieldTraits<T>::EType; }

    void TypedEval(const FMaterialParameterContext& context, T& dst) { 
        IMaterialParameter::Eval(context, &dst, sizeof(T)); 
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterDefaultMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
bool TryCreateDefaultMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
bool GetOrCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
