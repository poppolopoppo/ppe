#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"


namespace Core {
namespace Engine {
class MaterialDatabase;

#define EACH_MATERIALPARAMETER_BOOL(_Macro) \
    // _Macro(MaterialVariability::World, float,   WorldElapsedSeconds) \

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_BOOL(MATERIALPARAMETER_FN_DECL)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(MaterialDatabase *database);
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterBool {
//----------------------------------------------------------------------------
template <typename T>
struct Not {
    STypedMaterialParameter<T> Source;
    Not(ITypedMaterialParameter<T> *source);

    typedef T value_type;
    MaterialVariability Variability() const { return Source->Info().Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst);
};
template <typename T>
using Memoizer_Not = MaterialParameterMemoizer<Not<T> >;
extern template class MaterialParameterMemoizer<Not<bool> >;
extern template class MaterialParameterMemoizer<Not<i32> >;
extern template class MaterialParameterMemoizer<Not<u32> >;
//----------------------------------------------------------------------------
bool TryCreateMaterialParameter(
    PMaterialParameter *param,
    const MaterialParameterMutableContext& context,
    const Graphics::BindName& name,
    const Graphics::ConstantField& field );
//----------------------------------------------------------------------------
} //!MaterialParameterBool
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
