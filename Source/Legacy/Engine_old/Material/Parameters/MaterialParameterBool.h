#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"


namespace Core {
namespace Engine {
class FMaterialDatabase;

#define EACH_MATERIALPARAMETER_BOOL(_Macro) \
    // _Macro(EMaterialVariability::FWorld, float,   WorldElapsedSeconds) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_BOOL(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
template <typename T>
struct TNot {
    TSTypedMaterialParameter<T> Source;
    TNot(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    EMaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst);
};
template <typename T>
using TMemoizer_Not = TMaterialParameterMemoizer<TNot<T> >;
extern template class TMaterialParameterMemoizer<TNot<bool> >;
extern template class TMaterialParameterMemoizer<TNot<i32> >;
extern template class TMaterialParameterMemoizer<TNot<u32> >;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const FMaterialParameterMutableContext& context,
    const Graphics::FBindName& name,
    const Graphics::FConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
