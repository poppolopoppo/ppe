#include "stdafx.h"

#include "MaterialParameterBlock.h"

#include "Material/Material.h"

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
bool TryCreateOptionalMaterialParameter(
    AbstractMaterialParameter **param,
    const Material *material,
    const Scene *scene,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field ) {
    Assert(param);
    Assert(material);
    Assert(scene);
    Assert(!name.empty());

    const char *cstr = name.cstr();

    const char uniOptional[] = "uniOptional_";

    if (!StartsWith(cstr, uniOptional)) {
        Assert(!*param);
        return false;
    }

    const Graphics::BindName parameterName(&cstr[lengthof(uniOptional) - 1]);

    // Try to find a parameter with this name :
    const auto it = material->Parameters().Find(parameterName);
    if (material->Parameters().end() != it) {
        Assert(it->second);
        *param = it->second.get();
        return true;
    }

    // If failed try to create a special parameter from that name (may exists with a modifier, like uniInvert_*) :
    if (TryCreateDefaultMaterialParameter(param, material, scene, parameterName, field)) {
        Assert(*param);
        return true;
    }

    // If really failed create a place holder with a default value :
    switch (field.Type())
    {
    case Graphics::ConstantFieldType::Int:
        *param = new MaterialParameterBlock<i32>(i32(0));
        break;
    case Graphics::ConstantFieldType::Int2:
        *param = new MaterialParameterBlock<i322>(i322(0));
        break;
    case Graphics::ConstantFieldType::Int3:
        *param = new MaterialParameterBlock<i323>(i323(0));
        break;
    case Graphics::ConstantFieldType::Int4:
        *param = new MaterialParameterBlock<i324>(i324(0));
        break;

    case Graphics::ConstantFieldType::UInt:
        *param = new MaterialParameterBlock<u32>(u32(0));
        break;
    case Graphics::ConstantFieldType::UInt2:
        *param = new MaterialParameterBlock<u322>(u322(0));
        break;
    case Graphics::ConstantFieldType::UInt3:
        *param = new MaterialParameterBlock<u323>(u323(0));
        break;
    case Graphics::ConstantFieldType::UInt4:
        *param = new MaterialParameterBlock<u324>(u324(0));
        break;

    case Graphics::ConstantFieldType::Float:
        *param = new MaterialParameterBlock<float>(float(0));
        break;
    case Graphics::ConstantFieldType::Float2:
        *param = new MaterialParameterBlock<float2>(float2(0));
        break;
    case Graphics::ConstantFieldType::Float3:
        *param = new MaterialParameterBlock<float3>(float3(0));
        break;
    case Graphics::ConstantFieldType::Float4:
        *param = new MaterialParameterBlock<float4>(float4(0));
        break;

    case Graphics::ConstantFieldType::Float3x3:
        *param = new MaterialParameterBlock<float3x3>(float3x3::Identity());
        break;
    case Graphics::ConstantFieldType::Float4x3:
        *param = new MaterialParameterBlock<float4x3>(float4x3::Identity());
        break;
    case Graphics::ConstantFieldType::Float4x4:
        *param = new MaterialParameterBlock<float4x4>(float4x4::Identity());
        break;

    default:
        AssertNotImplemented();
    }

    Assert(*param);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
