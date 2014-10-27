#include "stdafx.h"

#include "MaterialParameterMemoizer.h"

#include "Material/MaterialContext.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(MaterialParameterFunctionMemoizer<T>, template <typename T>);
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterFunctionMemoizer<T>::MaterialParameterFunctionMemoizer(MaterialVariability variability, eval_type eval)
:   AbstractMaterialParameterMemoizer<T>(variability)
,   _eval(eval) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterFunctionMemoizer<T>::MaterialParameterFunctionMemoizer(MaterialVariability variability, std::function<eval_type>&& eval)
:   AbstractMaterialParameterMemoizer<T>(variability)
,   _eval(std::move(eval)) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterFunctionMemoizer<T>::~MaterialParameterFunctionMemoizer() {}
//----------------------------------------------------------------------------
template <typename T>
bool MaterialParameterFunctionMemoizer<T>::Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) {
    return _eval(cached, context);
}
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(MaterialParameterFunctionMemoizer, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
