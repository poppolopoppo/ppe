// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MaterialParameterConstant.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, TMaterialParameterConstant<T>, template <typename T>);
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(TMaterialParameterConstant, );
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterConstant<T>::TMaterialParameterConstant(T&& rvalue)
:   _value(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterConstant<T>::TMaterialParameterConstant(const T& value)
:   _value(value) {}
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterConstant<T>::~TMaterialParameterConstant() {}
//----------------------------------------------------------------------------
template <typename T>
FMaterialParameterInfo TMaterialParameterConstant<T>::Info() const {
    const FMaterialParameterInfo info{
        ITypedMaterialParameter<T>::EType(), 
        EMaterialVariability::Batch
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename T>
void TMaterialParameterConstant<T>::Eval(const FMaterialParameterContext& , void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(T));

    *reinterpret_cast<T *>(dst) = _value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static IMaterialParameter *CreateOptionalParameter_(Graphics::EConstantFieldType type) {
    switch (type)
    {
    case Graphics::EConstantFieldType::Bool:
        return new TMaterialParameterConstant<bool>(bool(0));

    case Graphics::EConstantFieldType::Int:
        return new TMaterialParameterConstant<i32>(i32(0));
    case Graphics::EConstantFieldType::Int2:
        return new TMaterialParameterConstant<i322>(i322(0));
    case Graphics::EConstantFieldType::Int3:
        return new TMaterialParameterConstant<i323>(i323(0));
    case Graphics::EConstantFieldType::Int4:
        return new TMaterialParameterConstant<i324>(i324(0));

    case Graphics::EConstantFieldType::UInt:
        return new TMaterialParameterConstant<u32>(u32(0));
    case Graphics::EConstantFieldType::UInt2:
        return new TMaterialParameterConstant<u322>(u322(0));
    case Graphics::EConstantFieldType::UInt3:
        return new TMaterialParameterConstant<u323>(u323(0));
    case Graphics::EConstantFieldType::UInt4:
        return new TMaterialParameterConstant<u324>(u324(0));

    case Graphics::EConstantFieldType::Float:
        return new TMaterialParameterConstant<float>(float(0));
    case Graphics::EConstantFieldType::Float2:
        return new TMaterialParameterConstant<float2>(float2(0));
    case Graphics::EConstantFieldType::Float3:
        return new TMaterialParameterConstant<float3>(float3(0));
    case Graphics::EConstantFieldType::Float4:
        return new TMaterialParameterConstant<float4>(float4(0));

    case Graphics::EConstantFieldType::Float3x3:
        return new TMaterialParameterConstant<float3x3>(float3x3::Identity());
    case Graphics::EConstantFieldType::Float4x3:
        return new TMaterialParameterConstant<float4x3>(float4x3::Identity());
    case Graphics::EConstantFieldType::Float3x4:
        return new TMaterialParameterConstant<float3x4>(float3x4::Identity());
    case Graphics::EConstantFieldType::Float4x4:
        return new TMaterialParameterConstant<float4x4>(float4x4::Identity());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field ) {
    Assert(param);
    Assert(context.Scene);
    Assert(context.MaterialEffect);
    Assert(context.Database);
    Assert(!name.empty());

    const char *cstr = name.c_str();

    const char uniOptional[] = "uniOptional_";

    if (!StartsWith(cstr, uniOptional)) {
        Assert(!*param);
        return false;
    }

    const Graphics::FBindName parameterName(&cstr[lengthof(uniOptional) - 1]);

    PMaterialParameter source;
    if (GetOrCreateMaterialParameter(&source, context, parameterName, field))
        return true;

    // If really failed create a place holder with a default value :
    *param = CreateOptionalParameter_(field.Type());
    Assert(*param);

    context.MaterialEffect->BindParameter(name, *param); // TODO : bind in local context.MaterialParameterDatabase

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
