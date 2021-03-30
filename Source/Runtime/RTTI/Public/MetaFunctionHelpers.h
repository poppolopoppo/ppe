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
FMetaParameter MakeParameter(TTypeTag< T >, const FStringView& name) {
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
template <typename _Result, class _Class, typename... _Args>
struct TMakeFunction<_Result (_Class::*)(_Args...)> {

    template <_Result (_Class::* _Member)(_Args...)>
    static FMetaFunction Make(const FName& name, EFunctionFlags flags, std::initializer_list<FStringView> parametersName) {
        Assert(sizeof...(_Args) == parametersName.size());
        auto nameIt = std::begin(parametersName);
        UNUSED(nameIt);
        return FMetaFunction(
            name,
            flags,
            MakeFunctionResultTraits<_Result>(),
            { MakeParameter(TypeTag< _Args >, *nameIt++)... },
            &TMemberFunction_<_Member, typename std::is_void<_Result>::type>::Invoke
        );
    }

private:
    template <_Result (_Class::* _Member)(_Args...), typename T = std::false_type>
    struct TMemberFunction_ {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            const FAtom* parg = arguments.data();
            UNUSED(parg);
            result.TypedData<_Result>() = CallTuple(_Member,
                const_cast<_Class*>(RTTI::CastChecked<_Class>(&obj)),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };

    template <_Result (_Class::* _Member)(_Args...)>
    struct TMemberFunction_<_Member, std::true_type> {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            Assert(not result);
            const FAtom* parg = arguments.data();
            UNUSED(parg);
            CallTuple(_Member,
                const_cast<_Class*>(RTTI::CastChecked<_Class>(&obj)),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };
};
template <typename _Result, class _Class, typename... _Args>
struct TMakeFunction<_Result (_Class::*)(_Args...) const> {
    template <_Result(_Class::* _Member)(_Args...) const>
    static FMetaFunction Make(const FName& name, EFunctionFlags flags, std::initializer_list<FStringView> parametersName) {
        Assert(sizeof...(_Args) == parametersName.size());
        auto nameIt = std::begin(parametersName);
        UNUSED(nameIt);
        return FMetaFunction(
            name,
            flags,
            MakeFunctionResultTraits<_Result>(),
            { MakeParameter(TypeTag< _Args >, *nameIt++)... },
            &TMemberFunction_<_Member>::Invoke
        );
    }

private:
    template <_Result(_Class::* _Member)(_Args...) const, bool = std::is_void<_Result>::value >
    struct TMemberFunction_ {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            const FAtom* parg = arguments.data();
            UNUSED(parg);
            result.TypedData<_Result>() = CallTuple(_Member,
                RTTI::CastChecked<_Class>(&obj),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };

    template <_Result(_Class::* _Member)(_Args...) const>
    struct TMemberFunction_<_Member, true> {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            Assert(not result);
            const FAtom* parg = arguments.data();
            UNUSED(parg);
            CallTuple(_Member,
                RTTI::CastChecked<_Class>(&obj),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <auto _Function>
FMetaFunction MakeFunction(
    const FName& name,
    std::initializer_list<FStringView> parametersName,
    EFunctionFlags flags = EFunctionFlags::Public ) {
    return details::TMakeFunction<decltype(_Function)>::template Make<_Function>(name, flags, parametersName);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
