#include "stdafx.h"

#include "MaterialParameterVariable.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, TMaterialParameterVariable<T>, template <typename T>);
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DEF(TMaterialParameterVariable, );
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterVariable<T>::TMaterialParameterVariable(T&& rvalue)
:   _value(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterVariable<T>::TMaterialParameterVariable(const T& value)
:   _value(value) {}
//----------------------------------------------------------------------------
template <typename T>
TMaterialParameterVariable<T>::~TMaterialParameterVariable() {}
//----------------------------------------------------------------------------
template <typename T>
const T& TMaterialParameterVariable<T>::FValue() const {
    return _value;
}
//----------------------------------------------------------------------------
template <typename T>
void TMaterialParameterVariable<T>::SetValue(const T& value) {
    Assert(IsInMainThread());

    _value = value;
}
//----------------------------------------------------------------------------
template <typename T>
FMaterialParameterInfo TMaterialParameterVariable<T>::Info() const {
    const FMaterialParameterInfo info{
        ITypedMaterialParameter<T>::EType(), 
        EMaterialVariability::Always
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename T>
void TMaterialParameterVariable<T>::Eval(const FMaterialParameterContext& , void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(T));

    *reinterpret_cast<T *>(dst) = _value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
