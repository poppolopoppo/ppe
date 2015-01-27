#include "stdafx.h"

#include "MaterialParameterMath.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Color/Color.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static AbstractMaterialParameter *CreateInvertParam_(AbstractMaterialParameter *source) {
    Assert(source);

    switch (source->FieldType()) {
    case Graphics::ConstantFieldType::Float3x3:
        return new MaterialParameterMath_Invert<float3x3>(source->Cast<float3x3>());
    case Graphics::ConstantFieldType::Float4x4:
        return new MaterialParameterMath_Invert<float4x4>(source->Cast<float4x4>());

    case Graphics::ConstantFieldType::Float4x3:
        break; // no support

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static AbstractMaterialParameter *CreateInvertTranposeParam_(AbstractMaterialParameter *source) {
    Assert(source);

    switch (source->FieldType()) {
    case Graphics::ConstantFieldType::Float3x3:
        return new MaterialParameterMath_InvertTranspose<float3x3>(source->Cast<float3x3>());
    case Graphics::ConstantFieldType::Float4x4:
        return new MaterialParameterMath_InvertTranspose<float4x4>(source->Cast<float4x4>());

    case Graphics::ConstantFieldType::Float4x3:
        break; // no support

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static AbstractMaterialParameter *CreateRcpParam_(AbstractMaterialParameter *source) {
    Assert(source);

    switch (source->FieldType()) {
    case Graphics::ConstantFieldType::Int:
        return new MaterialParameterMath_Rcp<i32>(source->Cast<i32>());
    case Graphics::ConstantFieldType::Int2:
        return new MaterialParameterMath_Rcp<i322>(source->Cast<i322>());
    case Graphics::ConstantFieldType::Int3:
        return new MaterialParameterMath_Rcp<i323>(source->Cast<i323>());
    case Graphics::ConstantFieldType::Int4:
        return new MaterialParameterMath_Rcp<i324>(source->Cast<i324>());

    case Graphics::ConstantFieldType::UInt:
        return new MaterialParameterMath_Rcp<u32>(source->Cast<u32>());
    case Graphics::ConstantFieldType::UInt2:
        return new MaterialParameterMath_Rcp<u322>(source->Cast<u322>());
    case Graphics::ConstantFieldType::UInt3:
        return new MaterialParameterMath_Rcp<u323>(source->Cast<u323>());
    case Graphics::ConstantFieldType::UInt4:
        return new MaterialParameterMath_Rcp<u324>(source->Cast<u324>());

    case Graphics::ConstantFieldType::Float:
        return new MaterialParameterMath_Rcp<float>(source->Cast<float>());
    case Graphics::ConstantFieldType::Float2:
        return new MaterialParameterMath_Rcp<float2>(source->Cast<float2>());
    case Graphics::ConstantFieldType::Float3:
        return new MaterialParameterMath_Rcp<float3>(source->Cast<float3>());
    case Graphics::ConstantFieldType::Float4:
        return new MaterialParameterMath_Rcp<float4>(source->Cast<float4>());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static AbstractMaterialParameter *CreateSRGBParam_(AbstractMaterialParameter *source) {
    Assert(source);

    switch (source->FieldType()) {
    case Graphics::ConstantFieldType::Float3:
        return new MaterialParameterMath_SRGB(source->Cast<float3>());
    case Graphics::ConstantFieldType::Float4:
        return new MaterialParameterMath_SRGBA(source->Cast<float4>());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterMath_Invert<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
bool MaterialParameterMath_Invert<T>::Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) {
    bool changed = false;
    _source->Eval(context, &changed);

    T original;
    _source->TypedCopyTo(&original);

    const T value = Invert(original);

    changed |= (value != *cached);
    *cached = value;

    return changed;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(    , MaterialParameterMath_Invert, , float3x3);
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(    , MaterialParameterMath_Invert, , float4x4);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterMath_InvertTranspose<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
bool MaterialParameterMath_InvertTranspose<T>::Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) {
    bool changed = false;
    _source->Eval(context, &changed);

    T original;
    _source->TypedCopyTo(&original);

    const T value = InvertTranspose(original);

    changed |= (value != *cached);
    *cached = value;

    return changed;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(    , MaterialParameterMath_InvertTranspose, , float3x3);
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_(    , MaterialParameterMath_InvertTranspose, , float4x4);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterMath_Rcp<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
bool MaterialParameterMath_Rcp<T>::Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) {
    bool changed = false;
    _source->Eval(context, &changed);

    T original;
    _source->TypedCopyTo(&original);

    const T value = Rcp(original);

    changed |= (value != *cached);
    *cached = value;

    return changed;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_SCALAR_(   , MaterialParameterMath_Rcp, );
CONSTANTFIELD_EXTERNALTEMPLATE_IMPL_VECTOR_(   , MaterialParameterMath_Rcp, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterMath_SRGB, );
//----------------------------------------------------------------------------
bool MaterialParameterMath_SRGB::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    bool changed = false;
    _source->Eval(context, &changed);

    float3 original;
    _source->TypedCopyTo(&original);

    const ColorRGBAF srgb(original.xyz(), 1.0f);
    const float3 value = srgb.ToLinear().Data().xyz();

    changed |= (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterMath_SRGBA, );
//----------------------------------------------------------------------------
bool MaterialParameterMath_SRGBA::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) {
    bool changed = false;
    _source->Eval(context, &changed);

    float4 original;
    _source->TypedCopyTo(&original);

    const ColorRGBAF srgb(original);
    const float4 value = srgb.ToLinear().Data();

    changed |= (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterMathMaterialParameters(MaterialDatabase *database) {
    Assert(database);
}
//----------------------------------------------------------------------------
bool TryCreateMathMaterialParameter(
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

    const char uniInvertTranspose[] = "uniInvertTranspose_";
    const char uniInvert[] = "uniInvert_";
    const char uniRcp[] = "uniRcp_";
    const char uniSRGB[] = "uniSRGB_";

    AbstractMaterialParameter *(*paramFunc)(AbstractMaterialParameter *) = nullptr;
    Graphics::BindName parameterName;

    if (StartsWith(cstr, uniInvertTranspose)) {
        paramFunc = &CreateInvertTranposeParam_;
        parameterName = &cstr[lengthof(uniInvertTranspose) - 1];
    }
    else if (StartsWith(cstr, uniInvert)) {
        paramFunc = &CreateInvertParam_;
        parameterName = &cstr[lengthof(uniInvert) - 1];
    }
    else if (StartsWith(cstr, uniRcp)) {
        paramFunc = &CreateRcpParam_;
        parameterName = &cstr[lengthof(uniRcp) - 1];
    }
    else if (StartsWith(cstr, uniSRGB)) {
        paramFunc = &CreateSRGBParam_;
        parameterName = &cstr[lengthof(uniSRGB) - 1];
    }
    else {
        Assert(!*param);
        return false;
    }
    Assert(paramFunc);
    Assert(!parameterName.empty());

    PAbstractMaterialParameter source;
    // Local source parameter search :
    if (materialEffect->Material()->Parameters().TryGet(parameterName, &source) ||
        materialEffect->Parameters().TryGet(parameterName, &source) ) {
        Assert(source);
        *param = (*paramFunc)(source);
        materialEffect->BindParameter(name, *param);
    }
    // Global source parameter search :
    else if (materialDatabase->TryGetParameter(parameterName, source)) {
        Assert(source);
        *param = (*paramFunc)(source);
        materialDatabase->BindParameter(name, *param);
    }
    else {
        return false;
    }

    Assert(*param);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
