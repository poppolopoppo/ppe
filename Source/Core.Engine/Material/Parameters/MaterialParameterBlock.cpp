#include "stdafx.h"

#include "MaterialParameterBlock.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterBlock<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterBlock<T>::MaterialParameterBlock()
:   MaterialParameterBlock(T()) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterBlock<T>::~MaterialParameterBlock() {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterBlock<T>::MaterialParameterBlock(T&& rvalue)
:   TypedMaterialParameter<T>(MaterialVariability::Frame)
,   _value(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterBlock<T>::MaterialParameterBlock(const T& value)
:   TypedMaterialParameter<T>(MaterialVariability::Frame)
,   _value(value) {}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterBlock<T>::SetValue(T&& value) {
    SetDirty();
    _value = std::move(value);
}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterBlock<T>::SetValue(const T& value) {
    SetDirty();
    _value = value;
}
//----------------------------------------------------------------------------
template <typename T>
bool MaterialParameterBlock<T>::EvalIFN_ReturnIfChanged_(const MaterialContext& context) {
    return Dirty();
}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterBlock<T>::CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const {
    Assert(sizeInBytes == sizeof(T));
    *reinterpret_cast<T *>(dst) = _value;
}
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(MaterialParameterBlock, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static AbstractMaterialParameter *CreateOptionalParameter_(Graphics::ConstantFieldType type) {
    switch (type)
    {
    case Graphics::ConstantFieldType::Int:
        return new MaterialParameterBlock<i32>(i32(0));
    case Graphics::ConstantFieldType::Int2:
        return new MaterialParameterBlock<i322>(i322(0));
    case Graphics::ConstantFieldType::Int3:
        return new MaterialParameterBlock<i323>(i323(0));
    case Graphics::ConstantFieldType::Int4:
        return new MaterialParameterBlock<i324>(i324(0));

    case Graphics::ConstantFieldType::UInt:
        return new MaterialParameterBlock<u32>(u32(0));
    case Graphics::ConstantFieldType::UInt2:
        return new MaterialParameterBlock<u322>(u322(0));
    case Graphics::ConstantFieldType::UInt3:
        return new MaterialParameterBlock<u323>(u323(0));
    case Graphics::ConstantFieldType::UInt4:
        return new MaterialParameterBlock<u324>(u324(0));

    case Graphics::ConstantFieldType::Float:
        return new MaterialParameterBlock<float>(float(0));
    case Graphics::ConstantFieldType::Float2:
        return new MaterialParameterBlock<float2>(float2(0));
    case Graphics::ConstantFieldType::Float3:
        return new MaterialParameterBlock<float3>(float3(0));
    case Graphics::ConstantFieldType::Float4:
        return new MaterialParameterBlock<float4>(float4(0));

    case Graphics::ConstantFieldType::Float3x3:
        return new MaterialParameterBlock<float3x3>(float3x3::Identity());
    case Graphics::ConstantFieldType::Float4x3:
        return new MaterialParameterBlock<float4x3>(float4x3::Identity());
    case Graphics::ConstantFieldType::Float4x4:
        return new MaterialParameterBlock<float4x4>(float4x4::Identity());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    AbstractMaterialParameter **param,
    MaterialEffect *materialEffect,
    MaterialDatabase *materialDatabase,
    const Scene *scene,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field ) {
    Assert(param);
    Assert(materialEffect);
    Assert(materialDatabase);
    Assert(scene);
    Assert(!name.empty());

    const char *cstr = name.cstr();

    const char uniOptional[] = "uniOptional_";

    if (!StartsWith(cstr, uniOptional)) {
        Assert(!*param);
        return false;
    }

    const Graphics::BindName parameterName(&cstr[lengthof(uniOptional) - 1]);

    PAbstractMaterialParameter source;
    // Local source parameter search :
    if (materialEffect->Material()->Parameters().TryGet(parameterName, &source) ||
        materialEffect->Parameters().TryGet(parameterName, &source) ) {
        Assert(source);
        *param = source.get();
        return true;
    }
    // Global source parameter search :
    else if (materialDatabase->TryGetParameter(parameterName, source)) {
        Assert(source);
        *param = source.get();
        return true;
    }

    // If failed try to create a special parameter from that name (may exists with a modifier, like uniInvert_*) :
    if (TryCreateDefaultMaterialParameter(param, materialEffect, materialDatabase, scene, parameterName, field)) {
        Assert(*param);
        return true;
    }

    // If really failed create a place holder with a default value :
    *param = CreateOptionalParameter_(field.Type());
    materialEffect->BindParameter(name, *param);

    Assert(*param);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
