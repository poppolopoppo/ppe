#include "stdafx.h"

#include "MaterialParameterBlock.h"

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
} //!namespace Engine
} //!namespace Core
