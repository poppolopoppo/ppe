#pragma once

#include "Core.RTTI/MetaFunction.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/Tuple.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result, typename... _Args>
TMemoryView<const FMetaTypeInfo> TMetaFunctionImpl<_Result, _Args...>::SignatureInfos() const {
    static const FMetaTypeInfo gSignatureTypeInfos[] = {
        TypeInfo< _Result >(),
        TypeInfo< _Args >()...
    };
    return gSignatureTypeInfos;
}
//----------------------------------------------------------------------------
template <typename _Result, typename... _Args>
TMemoryView<const IMetaTypeVirtualTraits* const> TMetaFunctionImpl<_Result, _Args...>::SignatureTraits() const {
    static const IMetaTypeVirtualTraits* const gSignatureTraits[] = {
        TMetaTypeTraits< _Result >::VirtualTraits(),
        TMetaTypeTraits< _Args >::VirtualTraits()...
    };
    return gSignatureTraits;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedFunction<_Result COMMA _Class COMMA _Args...>, template <typename _Result COMMA typename _Class COMMA typename... _Args>)
//----------------------------------------------------------------------------
namespace details {
template <size_t _Index, typename... _Args>
struct TFunctionOutputFlag_ : std::integral_constant<size_t, 0> {};
template <size_t _Index, typename _Arg0, typename... _Args>
struct TFunctionOutputFlag_<_Index, _Arg0, _Args...> : std::integral_constant<size_t, TFunctionOutputFlag_<_Index, _Arg0>::value | TFunctionOutputFlag_<_Index + 1, _Args...>::value > {};
template <size_t _Index, typename _Arg0>
struct TFunctionOutputFlag_<_Index, _Arg0> : std::integral_constant<size_t, size_t( not std::is_const< Meta::TRemoveReference<_Arg0> >::value && std::is_lvalue_reference<_Arg0>::value ? 1 : 0 ) << _Index > {};
} //!details
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
TMetaTypedFunction<_Result, _Class, _Args...>::TMetaTypedFunction(const FName& name, EFlags attributes, func_type func)
    : parent_type(name, attributes, sizeof...(_Args))
    , _func(func) {
    Assert(_func);
    SetOutputFlags_(TFunctionOutputFlag_<0, _Args...>::value);
}
//----------------------------------------------------------------------------
namespace details {
template<typename... _Args, size_t... _Index>
static bool UnwrapFunctionArgs_(TTuple< Meta::TDecay<_Args>...>& unwrap, const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
    bool succeed = true;
    FOLD_EXPR( succeed &= pargs[_Index] && AssignMove(&std::get<_Index>(unwrap), pargs[_Index].get()) );
    return succeed;
}
template<typename... _Args, size_t... _Index>
static void WrapFunctionArgs_(TTuple< Meta::TDecay<_Args>...>& unwrap, const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
    FOLD_EXPR(TFunctionOutputFlag_<0, _Args>::value ? AssignMove(pargs[_Index].get(), &std::get<_Index>(unwrap)) : false);
}
} //!details
//----------------------------------------------------------------------------
namespace details {
template<typename _Result, typename _Class, typename... _Args>
struct TWrapFunctionCall_ {
    static bool Invoke(_Class* src, _Result (_Class::*func)(_Args&&...), PMetaAtom& presult, const TTuple< Meta::TDecay<_Args>...>& unwrap) {
        Meta::TDecay<_Result> result = Call(func, src, unwrap);

        if (presult) {
            return AssignMove(presult.get(), &result);
        }
        else {
            presult = MakeAtom(std::move(result));
            return true;
        }
    }
};
template<typename _Class, typename... _Args>
struct TWrapFunctionCall_<void, _Class, _Args...> {
    static bool Invoke(_Class* src, void (_Class::*func)(_Args&&...), PMetaAtom& , const TTuple< Meta::TDecay<_Args>...>& unwrap) {
        Call(func, src, unwrap);
    }
};
template<typename _Result, typename _Class, typename... _Args>
bool WrapFunctionCall_(_Class* src, _Result(_Class::*func)(_Args&&...), PMetaAtom& presult, const TTuple< Meta::TDecay<_Args>...>& unwrap) {
    return TWrapFunctionCall_<_Result, _Class, _Args...>::Invoke(src, func, presult, unwrap);
}
} //!details
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
bool TMetaTypedFunction<_Result, _Class, _Args...>::Invoke(FMetaObject* src, PMetaAtom& presult, const TMemoryView<const PMetaAtom>& args) const {
    Assert(src);
    Assert(args.size() == sizeof...(_Args));

    _Class* const typedSrc =  src->RTTI_Cast<_Class>();
    if (nullptr == typedSrc)
        return false;

    TTuple< Meta::TDecay<_Args>...> unwrap;
    if (not details::UnwrapFunctionArgs_(unwrap, args, std::index_sequence_for<_Args...>{}))
        return false;

    if (not details::WrapFunctionCall_(typedSrc, _func, presult, unwrap))
        return false;

    details::WrapFunctionArgs_(unwrap, args, std::index_sequence_for<_Args...>{});

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
