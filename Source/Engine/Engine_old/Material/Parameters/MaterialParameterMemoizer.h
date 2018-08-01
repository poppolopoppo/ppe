#pragma once

#include "Core.Engine/Engine.h"

#include <functional>

#include "Core.Engine/Material/IMaterialParameter.h"
#include "Core.Engine/Material/MaterialVariability.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TMaterialParameterMemoizer : public ITypedMaterialParameter<typename _Functor::value_type>, private _Functor {
public:
    typedef ITypedMaterialParameter<typename _Functor::value_type> parent_type;
    typedef _Functor functor_type;

    using functor_type::value_type;
    using functor_type::Variability;
    using functor_type::TypedEval;

    template <typename... _Args>
    TMaterialParameterMemoizer(_Args&&... args) : functor_type(std::forward<_Args>(args)...) {}
    virtual ~TMaterialParameterMemoizer() {}

    TMaterialParameterMemoizer(_Functor&& functor) : functor_type(std::move(functor)) {}
    TMaterialParameterMemoizer(const _Functor& functor) : functor_type(functor) {}

    virtual FMaterialParameterInfo Info() const override;

    virtual void Eval(const FMaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <EMaterialVariability _Variability, typename T, void (&_Fn)(const FMaterialParameterContext& , T& )>
struct TMaterialFunctor {
    typedef T value_type;
    EMaterialVariability Variability() const { return _Variability; }
    void TypedEval(const FMaterialParameterContext& context, T& dst) {
        _Fn(context, dst);
    }
};
//----------------------------------------------------------------------------
#define MATERIALPARAMETER_FN(_Variability, _Type, _Fn) \
    Core::Engine::TMaterialParameterMemoizer< Core::Engine::TMaterialFunctor<_Variability, _Type, _Fn> >
//----------------------------------------------------------------------------
#define MATERIALPARAMETER_FN_DECL(_Variability, _Type, _Name) \
    void _Name(const FMaterialParameterContext& context, _Type& dst); \
    extern template class MATERIALPARAMETER_FN(_Variability, _Type, _Name);
//----------------------------------------------------------------------------
#define MATERIALPARAMETER_FN_DEF(_Variability, _Type, _Name) \
    template class MATERIALPARAMETER_FN(_Variability, _Type, _Name);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer-inl.h"
