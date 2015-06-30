#pragma once

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, MaterialParameterMemoizer<_Functor>, template <typename _Functor>);
//----------------------------------------------------------------------------
template <typename _Functor>
MaterialParameterInfo MaterialParameterMemoizer<_Functor>::Info() const {
    const MaterialParameterInfo info{
        parent_type::Type(),
        functor_type::Variability()
    };
    return info;
}
//----------------------------------------------------------------------------
template <typename _Functor>
void MaterialParameterMemoizer<_Functor>::Eval(const MaterialParameterContext& context, void *dst, size_t sizeInBytes) {
    Assert(dst);
    Assert(sizeInBytes == sizeof(functor_type::value_type));

    functor_type::TypedEval(context, *reinterpret_cast<value_type *>(dst));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
