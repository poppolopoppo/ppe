#pragma once

#include "RTTI_fwd.h"

#include "MetaFunction.h"
#include "MetaObject.h"

#include "Container/TupleHelpers.h" // CallTuple()

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
static PTypeTraits MakeFunctionResultTraits() {
    IF_CONSTEXPR(std::is_same_v<void, T>) // void as a special case, only for function result type
        return PTypeTraits{};
    else
        return MakeTraits<T>();
}
//----------------------------------------------------------------------------
template <typename T>
FMetaParameter MakeParameter(TTypeTag< T >, FStringLiteral name) {
    static_assert(not std::is_pointer<T>::value, "pointers are not supported, use non-const references instead");
    return FMetaParameter(
        FName(name),
        MakeTraits<T>(),
        (std::is_lvalue_reference<T>::value && not std::is_const< Meta::TDecay<T> >::value)
            ? EParameterFlags::Output
            : EParameterFlags::Default );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Function>
struct TMakeFunction;
#define PPE_RTTI_MAKEFUNCTION_DEF(_CONST, _NOEXCEPT) \
    template <typename _Result, class _Class, typename... _Args> \
    struct TMakeFunction<_Result (_Class::*)(_Args...) _CONST _NOEXCEPT> { \
        template <_Result (_Class::* _Member)(_Args...) _CONST _NOEXCEPT> \
        static FMetaFunction Make(const FName& name, EFunctionFlags flags, std::initializer_list<FStringLiteral> parametersName) { \
            Assert(sizeof...(_Args) == parametersName.size()); \
            \
            auto nameIt = std::begin(parametersName); \
            Unused(nameIt); \
            \
            CONSTEXPR const EFunctionFlags opt_ = Default; \
            CONSTEXPR const EFunctionFlags opt_const = EFunctionFlags::Const; \
            CONSTEXPR const EFunctionFlags CONCAT(opt_, NOEXCEPT) = EFunctionFlags::NoExcept; \
            Unused(opt_), Unused(opt_const), Unused(CONCAT(opt_, NOEXCEPT)); \
            flags += CONCAT(opt_, _CONST) + CONCAT(opt_, _NOEXCEPT); \
            \
            return FMetaFunction( \
                name, \
                flags, \
                MakeFunctionResultTraits<_Result>(), \
                { MakeParameter(TypeTag< _Args >, *nameIt++)... }, \
                [](const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) _NOEXCEPT { \
                    _CONST _Class* const concrete = const_cast<_CONST _Class*>(RTTI::CastChecked<_Class>(&obj)); \
                    \
                    const FAtom* parg = arguments.data(); \
                    TTuple<Meta::TReference<_Args>...> args{ (*parg++).TypedData<Meta::TDecay<_Args>>()... }; \
                    Unused(parg); \
                    \
                    IF_CONSTEXPR(std::is_void_v<_Result>) \
                        Unused(result), CallTuple(_Member, concrete, std::move(args)); \
                    else \
                        result.TypedData<_Result>() = CallTuple(_Member, concrete, std::move(args)); \
                } \
            ); \
        } \
    };
PPE_RTTI_MAKEFUNCTION_DEF(const ,         )
PPE_RTTI_MAKEFUNCTION_DEF(const , NOEXCEPT)
PPE_RTTI_MAKEFUNCTION_DEF(      ,         )
PPE_RTTI_MAKEFUNCTION_DEF(      , NOEXCEPT)
#undef PPE_RTTI_MAKEFUNCTION_DEF
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <auto _Function>
FMetaFunction MakeFunction(
    const FName& name,
    std::initializer_list<FStringLiteral> parametersName,
    EFunctionFlags flags = EFunctionFlags::Public ) {
#if !PPE_VA_OPT_SUPPORTED
    if (parametersName.size() == 1 && parametersName.begin()->empty())
        parametersName = {}; // #TODO: workaround an issue with preprocessor and __VA_ARGS__ (remove when __VA_OPT_ can be used)
#endif
    return details::TMakeFunction<decltype(_Function)>::template Make<_Function>(name, flags, parametersName);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
