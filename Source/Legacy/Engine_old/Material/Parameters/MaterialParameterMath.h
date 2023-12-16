#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"

#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_MATH(_Macro) \
    // _Macro(EMaterialVariability::FWorld, float,   WorldElapsedSeconds) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MATH(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMath {
//----------------------------------------------------------------------------
template <typename T>
struct Invert {
    TSTypedMaterialParameter<T> Source;
    Invert(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst);
};
template <typename T>
using TMemoizer_Invert = TMaterialParameterMemoizer<Invert<T> >;
extern template class TMaterialParameterMemoizer<Invert<float3x3> >;
extern template class TMaterialParameterMemoizer<Invert<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct InvertTranspose {
    TSTypedMaterialParameter<T> Source;
    InvertTranspose(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst);
};
template <typename T>
using TMemoizer_InvertTranspose = TMaterialParameterMemoizer<InvertTranspose<T> >;
extern template class TMaterialParameterMemoizer<InvertTranspose<float3x3> >;
extern template class TMaterialParameterMemoizer<InvertTranspose<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct TTranspose {
    TSTypedMaterialParameter<T> Source;
    TTranspose(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst);
};
template <typename T>
using TMemoizer_Transpose = TMaterialParameterMemoizer<TTranspose<T> >;
extern template class TMaterialParameterMemoizer<TTranspose<float3x3> >;
extern template class TMaterialParameterMemoizer<TTranspose<float4x4> >;
//----------------------------------------------------------------------------
template <typename T>
struct TRcp {
    TSTypedMaterialParameter<T> Source;
    TRcp(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst);
};
template <typename T>
using TMemoizer_Rcp = TMaterialParameterMemoizer<TRcp<T> >;
extern template class TMaterialParameterMemoizer<TRcp<float> >;
extern template class TMaterialParameterMemoizer<TRcp<float2> >;
extern template class TMaterialParameterMemoizer<TRcp<float3> >;
extern template class TMaterialParameterMemoizer<TRcp<float4> >;
//----------------------------------------------------------------------------
struct SRGB {
    TSTypedMaterialParameter<float3> Source;
    SRGB(ITypedMaterialParameter<float3> *source);

    typedef float3 value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, float3& dst);
};
typedef TMaterialParameterMemoizer<SRGB> Memoizer_SRGB;
extern template class TMaterialParameterMemoizer<SRGB>;
//----------------------------------------------------------------------------
struct SRGBA {
    TSTypedMaterialParameter<float4> Source;
    SRGBA(ITypedMaterialParameter<float4> *source);

    typedef float4 value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, float4& dst);
};
typedef TMaterialParameterMemoizer<SRGBA> Memoizer_SRGBA;
extern template class TMaterialParameterMemoizer<SRGBA>;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterMath
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
