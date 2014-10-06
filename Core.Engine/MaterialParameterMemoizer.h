#pragma once

#include "Engine.h"

#include <functional>

#include "AbstractMaterialParameter.h"

#include "Core/Allocation.h"
#include "Core/ScalarMatrix.h"
#include "Core/ScalarVector.h"

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

    const T& Cached() const { return _cached; }

protected:
    virtual bool EvalIFN_ReturnIfChanged_(const MaterialContext& context) override sealed;
    virtual void CopyTo_AssumeEvaluated_(void *dst, size_t sizeInBytes) const override sealed;

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

#include "MaterialParameterMemoizer-inl.h"
