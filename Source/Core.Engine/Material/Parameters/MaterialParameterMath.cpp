#include "stdafx.h"

#include "MaterialParameterMath.h"

#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Color/Color.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static IMaterialParameter *CreateInvertParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Float3x3:
        return new MaterialParameterMath::TMemoizer_Invert<float3x3>(source->Cast<float3x3>());
    case Graphics::EConstantFieldType::Float4x4:
        return new MaterialParameterMath::TMemoizer_Invert<float4x4>(source->Cast<float4x4>());

    case Graphics::EConstantFieldType::Float4x3:
    case Graphics::EConstantFieldType::Float3x4:
        break; // no support

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static IMaterialParameter *CreateInvertTranposeParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Float3x3:
        return new MaterialParameterMath::TMemoizer_InvertTranspose<float3x3>(source->Cast<float3x3>());
    case Graphics::EConstantFieldType::Float4x4:
        return new MaterialParameterMath::TMemoizer_InvertTranspose<float4x4>(source->Cast<float4x4>());

    case Graphics::EConstantFieldType::Float4x3:
    case Graphics::EConstantFieldType::Float3x4:
        break; // no support

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static IMaterialParameter *CreateTranposeParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Float3x3:
        return new MaterialParameterMath::TMemoizer_Transpose<float3x3>(source->Cast<float3x3>());
    case Graphics::EConstantFieldType::Float4x4:
        return new MaterialParameterMath::TMemoizer_Transpose<float4x4>(source->Cast<float4x4>());

    case Graphics::EConstantFieldType::Float4x3:
    case Graphics::EConstantFieldType::Float3x4:
        break; // no support

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static IMaterialParameter *CreateRcpParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Float:
        return new MaterialParameterMath::TMemoizer_Rcp<float>(source->Cast<float>());
    case Graphics::EConstantFieldType::Float2:
        return new MaterialParameterMath::TMemoizer_Rcp<float2>(source->Cast<float2>());
    case Graphics::EConstantFieldType::Float3:
        return new MaterialParameterMath::TMemoizer_Rcp<float3>(source->Cast<float3>());
    case Graphics::EConstantFieldType::Float4:
        return new MaterialParameterMath::TMemoizer_Rcp<float4>(source->Cast<float4>());

    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
static IMaterialParameter *CreateSRGBParam_(IMaterialParameter *source) {
    Assert(source);

    switch (source->Info().Type) {
    case Graphics::EConstantFieldType::Float3:
        return new MaterialParameterMath::Memoizer_SRGB(source->Cast<float3>());
    case Graphics::EConstantFieldType::Float4:
        return new MaterialParameterMath::Memoizer_SRGBA(source->Cast<float4>());

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
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MATH(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_MATH(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
// Invert
//----------------------------------------------------------------------------
template <typename T>
Invert<T>::Invert(ITypedMaterialParameter<T> *source) : Source(source) { Assert(source); }
//----------------------------------------------------------------------------
template <typename T>
void Invert<T>::TypedEval(const FMaterialParameterContext& context, T& dst) {
    T original;
    Source->TypedEval(context, original);
    dst = Invert(original);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<Invert<float3x3> >;
template class TMaterialParameterMemoizer<Invert<float4x4> >;
//----------------------------------------------------------------------------
// InvertTranspose
//----------------------------------------------------------------------------
template <typename T>
InvertTranspose<T>::InvertTranspose(ITypedMaterialParameter<T> *source) : Source(source) { Assert(source); }
//----------------------------------------------------------------------------
template <typename T>
void InvertTranspose<T>::TypedEval(const FMaterialParameterContext& context, T& dst) {
    T original;
    Source->TypedEval(context, original);
    dst = InvertTranspose(original);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<InvertTranspose<float3x3> >;
template class TMaterialParameterMemoizer<InvertTranspose<float4x4> >;
//----------------------------------------------------------------------------
// TTranspose
//----------------------------------------------------------------------------
template <typename T>
TTranspose<T>::TTranspose(ITypedMaterialParameter<T> *source) : Source(source) { Assert(source); }
//----------------------------------------------------------------------------
template <typename T>
void TTranspose<T>::TypedEval(const FMaterialParameterContext& context, T& dst) {
    T original;
    Source->TypedEval(context, original);
    dst = original.Transpose();
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<TTranspose<float3x3> >;
template class TMaterialParameterMemoizer<TTranspose<float4x4> >;
//----------------------------------------------------------------------------
// TRcp
//----------------------------------------------------------------------------
template <typename T>
TRcp<T>::TRcp(ITypedMaterialParameter<T> *source) : Source(source) { Assert(source); }
//----------------------------------------------------------------------------
template <typename T>
void TRcp<T>::TypedEval(const FMaterialParameterContext& context, T& dst) {
    T original;
    Source->TypedEval(context, original);
    dst = TRcp(original);
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<TRcp<float> >;
template class TMaterialParameterMemoizer<TRcp<float2> >;
template class TMaterialParameterMemoizer<TRcp<float3> >;
template class TMaterialParameterMemoizer<TRcp<float4> >;
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
// SRGB
//----------------------------------------------------------------------------
SRGB::SRGB(ITypedMaterialParameter<float3> *source)
:   Source(source) {
    Assert(source);
}
//----------------------------------------------------------------------------
void SRGB::TypedEval(const FMaterialParameterContext& context, float3& dst) {
    float3 original;
    Source->TypedEval(context, original);

    const ColorRGBAF srgb(original.xyz(), 1.0f);
    dst = srgb.ToLinear().Data().xyz();
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<SRGB>;
//----------------------------------------------------------------------------
// SRGBA
//----------------------------------------------------------------------------
SRGBA::SRGBA(ITypedMaterialParameter<float4> *source)
:   Source(source) {
    Assert(source);
}
//----------------------------------------------------------------------------
void SRGBA::TypedEval(const FMaterialParameterContext& context, float4& dst) {
    float4 original;
    Source->TypedEval(context, original);

    const ColorRGBAF srgb(original);
    dst = srgb.ToLinear().Data();
}
//----------------------------------------------------------------------------
template class TMaterialParameterMemoizer<SRGBA>;
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field ) {
    Assert(param);
    Assert(context.MaterialEffect);
    Assert(context.Database);
    Assert(context.Scene);
    Assert(!name.empty());

    const char *cstr = name.c_str();

    static const char uniInvertTranspose[] = "uniInvertTranspose_";
    static const char uniInvert[] = "uniInvert_";
    static const char uniTranspose[] = "uniTranspose_";
    static const char uniRcp[] = "uniRcp_";
    static const char uniSRGB[] = "uniSRGB_";

    IMaterialParameter *(*paramFunc)(IMaterialParameter *) = nullptr;
    Graphics::FBindName parameterName;

    if (StartsWith(cstr, uniInvertTranspose)) {
        paramFunc = &CreateInvertTranposeParam_;
        parameterName = &cstr[lengthof(uniInvertTranspose) - 1];
    }
    else if (StartsWith(cstr, uniInvert)) {
        paramFunc = &CreateInvertParam_;
        parameterName = &cstr[lengthof(uniInvert) - 1];
    }
    else if (StartsWith(cstr, uniTranspose)) {
        paramFunc = &CreateTranposeParam_;
        parameterName = &cstr[lengthof(uniTranspose) - 1];
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

    PMaterialParameter source;
    if (!GetOrCreateMaterialParameter(&source, context, parameterName, field))
        return false;

    Assert(source);
    *param = (paramFunc)(source.get());

    if (nullptr == param->get())
        return false;

    context.MaterialEffect->BindParameter(name, *param); // TODO : bind in local context.MaterialParameterDatabase
    return true;
}
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
