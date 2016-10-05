#pragma once

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, TMaterialParameterMemoizer<_Functor>, template <typename _Functor>);
//----------------------------------------------------------------------------
template <typename _Functor>
FMaterialParameterInfo TMaterialParameterMemoizer<_Functor>::Info() const {
    const FMaterialParameterInfo info{
        parent_type::EType(),
        functor_type::Variability()
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename _Functor>
void TMaterialParameterMemoizer<_Functor>::Eval(const FMaterialParameterContext& context, void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(functor_type::value_type));

    functor_type::TypedEval(context, *reinterpret_cast<value_type *>(dst));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
