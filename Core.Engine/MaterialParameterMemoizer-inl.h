#pragma once

#include "MaterialParameterMemoizer.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
AbstractMaterialParameterMemoizer<T>::AbstractMaterialParameterMemoizer(MaterialVariability variability)
:   TypedMaterialParameter<T>(variability) {}
//----------------------------------------------------------------------------
template <typename T>
AbstractMaterialParameterMemoizer<T>::~AbstractMaterialParameterMemoizer() {}
//----------------------------------------------------------------------------
template <typename T>
bool AbstractMaterialParameterMemoizer<T>::EvalIFN_ReturnIfChanged_(const MaterialContext& context) {
    const MaterialVariability variability = Variability();
    const VariabilitySeed seed = context.Seeds[size_t(variability)];

    if (MaterialVariability::Always == variability ||
        seed != _seed || Dirty()) {
        _seed = seed;
        return Memoize_ReturnIfChanged_(&_cached, context);
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T>
void AbstractMaterialParameterMemoizer<T>::CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const {
    Assert(!Dirty());
    Assert(sizeInBytes == sizeof(T));
    Assert(_seed.Value != VariabilitySeed::Invalid);

    *reinterpret_cast<T *>(dst) = _cached;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
