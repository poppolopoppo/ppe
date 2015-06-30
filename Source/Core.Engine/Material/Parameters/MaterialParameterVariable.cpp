#include "stdafx.h"

#include "MaterialParameterVariable.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, MaterialParameterVariable<T>, template <typename T>);
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(MaterialParameterVariable, );
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterVariable<T>::MaterialParameterVariable(T&& rvalue)
:   _value(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterVariable<T>::MaterialParameterVariable(const T& value)
:   _value(value) {}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterVariable<T>::~MaterialParameterVariable() {}
//----------------------------------------------------------------------------
template <typename T>
const T& MaterialParameterVariable<T>::Value() const {
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterVariable<T>::SetValue(const T& value) {
    Assert(IsInMainThread());

    _value = value;
}
//----------------------------------------------------------------------------
template <typename T>
MaterialParameterInfo MaterialParameterVariable<T>::Info() const {
    const MaterialParameterInfo info{
        ITypedMaterialParameter<T>::Type(), 
        MaterialVariability::Always
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename T>
void MaterialParameterVariable<T>::Eval(const MaterialParameterContext& , void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(T));

    *reinterpret_cast<T *>(dst) = _value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
