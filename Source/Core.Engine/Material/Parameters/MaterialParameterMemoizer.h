#pragma once

#include "Core.Engine/Engine.h"

#include <functional>

#include "Core.Engine/Material/IMaterialParameter.h"
#include "Core.Engine/Material/MaterialVariability.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Transform/ScalarMatrix_fwd.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class MaterialParameterMemoizer : public ITypedMaterialParameter<typename _Functor::value_type>, private _Functor {
public:
    typedef ITypedMaterialParameter<typename _Functor::value_type> parent_type;
    typedef _Functor functor_type;

    using functor_type::value_type;
    using functor_type::Variability;
    using functor_type::TypedEval;

    template <typename... _Args>
    MaterialParameterMemoizer(_Args&&... args) : functor_type(std::forward<_Args>(args)...) {}
    virtual ~MaterialParameterMemoizer() {}

    MaterialParameterMemoizer(_Functor&& functor) : functor_type(std::move(functor)) {}
    MaterialParameterMemoizer(const _Functor& functor) : functor_type(functor) {}

    virtual MaterialParameterInfo Info() const override;

    virtual void Eval(const MaterialParameterContext& context, void *dst, size_t sizeInBytes) override;

    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterMemoizer);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <MaterialVariability _Variability, typename T, void (&_Fn)(const MaterialParameterContext& , T& )>
struct MaterialFunctor {
    typedef T value_type;
    MaterialVariability Variability() const { return _Variability; }
    void TypedEval(const MaterialParameterContext& context, T& dst) {
        _Fn(context, dst);
    }
};
//----------------------------------------------------------------------------
#define MATERIALPARAMETER_FN(_Variability, _Type, _Fn) \
    Core::Engine::MaterialParameterMemoizer< Core::Engine::MaterialFunctor<_Variability, _Type, _Fn> >
//----------------------------------------------------------------------------
#define MATERIALPARAMETER_FN_DECL(_Variability, _Type, _Name) \
    void _Name(const MaterialParameterContext& context, _Type& dst); \
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
