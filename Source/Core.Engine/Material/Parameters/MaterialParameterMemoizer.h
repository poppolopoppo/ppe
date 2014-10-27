#pragma once

#include "Core.Engine/Engine.h"

#include <functional>

#include "Core.Engine/Material/Parameters/AbstractMaterialParameter.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class AbstractMaterialParameterMemoizer : public TypedMaterialParameter<T> {
public:
    explicit AbstractMaterialParameterMemoizer(MaterialVariability variability);
    virtual ~AbstractMaterialParameterMemoizer();

    AbstractMaterialParameterMemoizer(const AbstractMaterialParameterMemoizer& ) = delete;
    AbstractMaterialParameterMemoizer& operator =(const AbstractMaterialParameterMemoizer& ) = delete;

    const T& Cached() const { return _cached; }

protected:
    virtual bool EvalIFN_ReturnIfChanged_(const MaterialContext& context) override;
    virtual void CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const override;

    virtual bool Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) = 0;

private:
    T _cached;
    VariabilitySeed _seed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MaterialParameterFunctionMemoizer : public AbstractMaterialParameterMemoizer<T> {
public:
    typedef bool (eval_type)(T *dst, const MaterialContext& context );

    MaterialParameterFunctionMemoizer(MaterialVariability variability, eval_type eval);
    MaterialParameterFunctionMemoizer(MaterialVariability variability, std::function<eval_type>&& eval);
    virtual ~MaterialParameterFunctionMemoizer();

    template <typename _Arg0, typename... _Args>
    MaterialParameterFunctionMemoizer(
        MaterialVariability variability,
        bool (*func)(T *, const MaterialContext *, const _Arg0& , const _Args&... ),
        _Arg0&& arg0, _Args&&... args)
    :   MaterialParameterFunctionMemoizer(variability, std::bind(func, _1, _2, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...))
    {}

    template <typename _Object, typename _Arg0, typename... _Args>
    MaterialParameterFunctionMemoizer(
        MaterialVariability variability,
        bool (_Object::* member)(T *, const MaterialContext *, const _Arg0& , const _Args&... ),
        _Object *instance, _Arg0&& arg0, _Args&&... args)
    :   MaterialParameterFunctionMemoizer(variability, std::bind(member, instance, _1, _2, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...))
    {}

    SINGLETON_POOL_ALLOCATED_DECL(MaterialParameterFunctionMemoizer);

protected:
    virtual bool Memoize_ReturnIfChanged_(T *cached, const MaterialContext& context) override;

private:
    std::function<eval_type> _eval;
};
//----------------------------------------------------------------------------
CONSTANTFIELD_EXTERNALTEMPLATE_DECL(MaterialParameterFunctionMemoizer, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Material/Parameters/MaterialParameterMemoizer-inl.h"
