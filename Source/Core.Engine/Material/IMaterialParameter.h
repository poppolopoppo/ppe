#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialParameter_fwd.h"

namespace Core {

namespace Graphics {
class BindName;
struct ConstantField;
}

namespace Engine {
class MaterialDatabase;
FWD_REFPTR(MaterialEffect);
FWD_REFPTR(Scene);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MaterialParameterInfo {
    Graphics::ConstantFieldType Type;
    MaterialVariability Variability;
};
//----------------------------------------------------------------------------
struct MaterialParameterContext {
    SCScene Scene;
    SCMaterialEffect MaterialEffect;
    const MaterialDatabase *Database;
};
//----------------------------------------------------------------------------
struct MaterialParameterMutableContext {
    SCScene Scene;
    SMaterialEffect MaterialEffect;
    MaterialDatabase *Database;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMaterialParameter : public RefCountable {
public:
    virtual ~IMaterialParameter() {}

    virtual MaterialParameterInfo Info() const = 0;

    virtual void Eval(const MaterialParameterContext& context, void *dst, size_t sizeInBytes) = 0;

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

    Graphics::ConstantFieldType Type() const { return Graphics::ConstantFieldTraits<T>::Type; }

    void TypedEval(const MaterialParameterContext& context, T& dst) { 
        IMaterialParameter::Eval(context, &dst, sizeof(T)); 
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterDefaultMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
bool TryCreateDefaultMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
bool GetOrCreateMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
