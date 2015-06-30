#include "stdafx.h"

#include "MaterialParameterConstant.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, MaterialParameterConstant<T>, template <typename T>);
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(MaterialParameterConstant, );
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterConstant<T>::MaterialParameterConstant(T&& rvalue)
:   _value(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterConstant<T>::MaterialParameterConstant(const T& value)
:   _value(value) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterConstant<T>::~MaterialParameterConstant() {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterInfo MaterialParameterConstant<T>::Info() const {
    const MaterialParameterInfo info{
        ITypedMaterialParameter<T>::Type(), 
        MaterialVariability::Batch
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterConstant<T>::Eval(const MaterialParameterContext& , void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(T));

    *reinterpret_cast<T *>(dst) = _value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static IMaterialParameter *CreateOptionalParameter_(Graphics::ConstantFieldType type) {
    switch (type)
    {
    case Graphics::ConstantFieldType::Bool:
        return new MaterialParameterConstant<bool>(bool(0));

    case Graphics::ConstantFieldType::Int:
        return new MaterialParameterConstant<i32>(i32(0));
    case Graphics::ConstantFieldType::Int2:
        return new MaterialParameterConstant<i322>(i322(0));
    case Graphics::ConstantFieldType::Int3:
        return new MaterialParameterConstant<i323>(i323(0));
    case Graphics::ConstantFieldType::Int4:
        return new MaterialParameterConstant<i324>(i324(0));

    case Graphics::ConstantFieldType::UInt:
        return new MaterialParameterConstant<u32>(u32(0));
    case Graphics::ConstantFieldType::UInt2:
        return new MaterialParameterConstant<u322>(u322(0));
    case Graphics::ConstantFieldType::UInt3:
        return new MaterialParameterConstant<u323>(u323(0));
    case Graphics::ConstantFieldType::UInt4:
        return new MaterialParameterConstant<u324>(u324(0));

    case Graphics::ConstantFieldType::Float:
        return new MaterialParameterConstant<float>(float(0));
    case Graphics::ConstantFieldType::Float2:
        return new MaterialParameterConstant<float2>(float2(0));
    case Graphics::ConstantFieldType::Float3:
        return new MaterialParameterConstant<float3>(float3(0));
    case Graphics::ConstantFieldType::Float4:
        return new MaterialParameterConstant<float4>(float4(0));

    case Graphics::ConstantFieldType::Float3x3:
        return new MaterialParameterConstant<float3x3>(float3x3::Identity());
    case Graphics::ConstantFieldType::Float4x3:
        return new MaterialParameterConstant<float4x3>(float4x3::Identity());
    case Graphics::ConstantFieldType::Float3x4:
        return new MaterialParameterConstant<float3x4>(float3x4::Identity());
    case Graphics::ConstantFieldType::Float4x4:
        return new MaterialParameterConstant<float4x4>(float4x4::Identity());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
bool TryCreateOptionalMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field ) {
    Assert(param);
    Assert(context.Scene);
    Assert(context.MaterialEffect);
    Assert(context.Database);
    Assert(!name.empty());

    const char *cstr = name.cstr();

    const char uniOptional[] = "uniOptional_";

    if (!StartsWith(cstr, uniOptional)) {
        Assert(!*param);
        return false;
    }

    const Graphics::BindName parameterName(&cstr[lengthof(uniOptional) - 1]);

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
