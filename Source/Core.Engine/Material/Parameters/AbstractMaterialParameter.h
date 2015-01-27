#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialVariability.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
class BindName;
struct ConstantField;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Material;
struct MaterialContext;
class MaterialDatabase;
class MaterialEffect;
class Scene;
template <typename T>
class TypedMaterialParameter;
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractMaterialParameter);
class AbstractMaterialParameter : public RefCountable {
protected:
    AbstractMaterialParameter(Graphics::ConstantFieldType fieldType, MaterialVariability variability);

public:
    virtual ~AbstractMaterialParameter();

    bool Changed() const { return bitchanged_type::Get(_data); }
    bool Dirty() const { return bitdirty_type::Get(_data); }
    MaterialVariability Variability() const { return static_cast<MaterialVariability>(bitvariability_type::Get(_data)); }
    Graphics::ConstantFieldType FieldType() const { return static_cast<Graphics::ConstantFieldType>(bitfield_type::Get(_data)); }

    template <typename T>
    TypedMaterialParameter<T> *Cast() {
        return checked_cast<TypedMaterialParameter<T> *>(this);
    }

    template <typename T>
    const TypedMaterialParameter<T> *Cast() const {
        return checked_cast<const TypedMaterialParameter<T> *>(this);
    }

    void Eval(const MaterialContext& context, bool *changed = nullptr);
    void CopyTo(void *dst, size_t sizeInBytes) const;

protected:
    virtual bool EvalIFN_ReturnIfChanged_(const MaterialContext& context) = 0;
    virtual void CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const = 0;

    void SetDirty() { bitdirty_type::InplaceTrue(_data); }

private:
    typedef Meta::Bit<u32>::First<1>::type bitchanged_type;
    typedef Meta::Bit<u32>::After<bitchanged_type>::Field<1>::type bitdirty_type;
    typedef Meta::Bit<u32>::After<bitdirty_type>::Field<3>::type bitvariability_type;
    typedef Meta::Bit<u32>::After<bitvariability_type>::Remain::type bitfield_type;

    u32 _data;
};
//----------------------------------------------------------------------------
template <typename T>
class TypedMaterialParameter : public AbstractMaterialParameter {
protected:
    explicit TypedMaterialParameter(MaterialVariability variability)
    :   AbstractMaterialParameter(Graphics::ConstantFieldTraits<T>::Type, variability) {}

public:
    virtual ~TypedMaterialParameter() {}

    void TypedCopyTo(T *dst) const { AbstractMaterialParameter::CopyTo(dst, sizeof(T)); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterDefaultMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
bool TryCreateDefaultMaterialParameter(
    AbstractMaterialParameter **param,
    MaterialEffect *materialEffect,
    MaterialDatabase *materialDatabase,
    const Scene *scene,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
