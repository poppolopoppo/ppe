#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_MATH(_Macro) \
    // _Macro(MaterialVariability::World, float,   WorldElapsedSeconds) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MATH(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
template <typename T>
struct Invert {
    STypedMaterialParameter<T> Source;
    Invert(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst);
};
template <typename T>
using Memoizer_Invert = MaterialParameterMemoizer<Invert<T> >;
extern template class MaterialParameterMemoizer<Invert<float3x3> >;
extern template class MaterialParameterMemoizer<Invert<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct InvertTranspose {
    STypedMaterialParameter<T> Source;
    InvertTranspose(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst);
};
template <typename T>
using Memoizer_InvertTranspose = MaterialParameterMemoizer<InvertTranspose<T> >;
extern template class MaterialParameterMemoizer<InvertTranspose<float3x3> >;
extern template class MaterialParameterMemoizer<InvertTranspose<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct Transpose {
    STypedMaterialParameter<T> Source;
    Transpose(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst);
};
template <typename T>
using Memoizer_Transpose = MaterialParameterMemoizer<Transpose<T> >;
extern template class MaterialParameterMemoizer<Transpose<float3x3> >;
extern template class MaterialParameterMemoizer<Transpose<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct Rcp {
    STypedMaterialParameter<T> Source;
    Rcp(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst);
};
template <typename T>
using Memoizer_Rcp = MaterialParameterMemoizer<Rcp<T> >;
extern template class MaterialParameterMemoizer<Rcp<float> >;
extern template class MaterialParameterMemoizer<Rcp<float2> >;
extern template class MaterialParameterMemoizer<Rcp<float3> >;
extern template class MaterialParameterMemoizer<Rcp<float4> >;
//----------------------------------------------------------------------------
struct SRGB {
    STypedMaterialParameter<float3> Source;
    SRGB(ITypedMaterialParameter<float3> *source);

    typedef float3 value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, float3& dst);
};
typedef MaterialParameterMemoizer<SRGB> Memoizer_SRGB;
extern template class MaterialParameterMemoizer<SRGB>;
//----------------------------------------------------------------------------
struct SRGBA {
    STypedMaterialParameter<float4> Source;
    SRGBA(ITypedMaterialParameter<float4> *source);

    typedef float4 value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, float4& dst);
};
typedef MaterialParameterMemoizer<SRGBA> Memoizer_SRGBA;
extern template class MaterialParameterMemoizer<SRGBA>;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
