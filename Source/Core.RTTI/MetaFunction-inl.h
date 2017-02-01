#pragma once

#include "Core.RTTI/MetaFunction.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/Tuple.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TMetaFunctionArg_ {
    typedef TMetaTypeTraits<T> traits_type;
    typedef typename traits_type::wrapper_type wrapper_type;
    static FMetaTypeInfo TypeInfo() { return RTTI::TypeInfo<T>(); }
    static const IMetaTypeVirtualTraits* VirtualTraits() { return traits_type::VirtualTraits(); }
};
template <>
struct TMetaFunctionArg_<void> {
    typedef void wrapper_type;
    static FMetaTypeInfo TypeInfo() { return FMetaTypeInfo{ FMetaTypeId(0), EMetaTypeFlags(), "void" }; }
    static const IMetaTypeVirtualTraits* VirtualTraits() { return nullptr; }
};
template <typename T> static FMetaTypeInfo MetaFunctionArgTypeInfo_() { return TMetaFunctionArg_<T>::TypeInfo(); }
template <typename T> static const IMetaTypeVirtualTraits* MetaFunctionArgVirtualTraits_() { return TMetaFunctionArg_<T>::VirtualTraits(); }
} //!details
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
namespace details {
template <typename... _Args>
struct TMetaFunctionPack_ {
    template <size_t... _Index>
    bool Unwrap(const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Index));
        bool succeed = true;
        FOLD_EXPR(succeed &= args[_Index] && AssignMove(&std::get<_Index>(UnwrappedPack), args[_Index].get()));
        return succeed;
    }

    template <size_t... _Index>
    void Wrap(const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Index));
        FOLD_EXPR(TFunctionOutputFlag_<0, _Args>::value ? AssignMove(args[_Index].get(), &std::get<_Index>(UnwrappedPack)) : false);
    }

    template <size_t... _Index>
    bool Promote(const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Index));
        bool succeed = true;
        FOLD_EXPR(succeed &= args[_Index] && PromoteMove(&std::get<_Index>(UnwrappedPack), args[_Index].get()));
        return succeed;
    }

    template <size_t... _Index>
    void Demote(const TMemoryView<const PMetaAtom>& args, std::index_sequence<_Index...>) {
        STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Index));
        FOLD_EXPR(TFunctionOutputFlag_<0, _Args>::value ? PromoteMove(args[_Index].get(), &std::get<_Index>(UnwrappedPack)) : false);
    }

    template <typename _Result, typename _Class>
    _Result Call(_Result(_Class::*func)(_Args...), _Class* src) {
        return Core::Call(func, src, UnwrappedPack);
    }

    TTuple< Meta::TDecay<_Args>... > UnwrappedPack;
};
template <>
struct TMetaFunctionPack_<> {
    bool Unwrap(const TMemoryView<const PMetaAtom>&, std::index_sequence<>) { return true; }
    void Wrap(const TMemoryView<const PMetaAtom>& , std::index_sequence<>) {}

    bool Promote(const TMemoryView<const PMetaAtom>&, std::index_sequence<>) { return true; }
    void Demote(const TMemoryView<const PMetaAtom>&, std::index_sequence<>) {}

    template <typename _Result, typename _Class>
    _Result Call(_Result(_Class::*func)(), _Class* src) {
        return (src->*func)();
    }
};
} //!details
//----------------------------------------------------------------------------
namespace details {
template <bool _Promote, typename _Result, typename _Class, typename... _Args>
struct TMetaFunctionCall_ {
    static bool Invoke(_Class* src, _Result (_Class::*func)(_Args...), PMetaAtom& presult, TMetaFunctionPack_<_Args... >& pack) {
        Meta::TDecay<_Result> result = pack.Call(func, src);
        if (presult) {
            return (_Promote
                ? PromoteMove(presult.get(), &result)
                : AssignMove(presult.get(), &result) );
        }
        else {
            presult = MakeAtom(std::move(result));
            return true;
        }
    }
};
template <bool _Promote, typename _Class, typename... _Args>
struct TMetaFunctionCall_<_Promote, void, _Class, _Args...> {
    static bool Invoke(_Class* src, void (_Class::*proc)(_Args...), PMetaAtom& , TMetaFunctionPack_<_Args...>& pack) {
        pack.Call(proc, src);
        return true;
    }
};
template <typename _Result, typename _Class, typename... _Args>
bool MetaFunctionCall_(_Class* src, _Result(_Class::*func)(_Args...), PMetaAtom& presult, TMetaFunctionPack_<_Args...>& pack) {
    return TMetaFunctionCall_<false, _Result, _Class, _Args...>::Invoke(src, func, presult, pack);
}
template <typename _Result, typename _Class, typename... _Args>
bool MetaFunctionCallPromote_(_Class* src, _Result(_Class::*func)(_Args...), PMetaAtom& presult, TMetaFunctionPack_<_Args...>& pack) {
    return TMetaFunctionCall_<true, _Result, _Class, _Args...>::Invoke(src, func, presult, pack);
}
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result, typename... _Args>
TMemoryView<const FMetaTypeInfo> TMetaFunctionImpl<_Result, _Args...>::SignatureInfos() const {
    static const FMetaTypeInfo gSignatureTypeInfos[] = {
        details::MetaFunctionArgTypeInfo_< _Result >(),
        details::MetaFunctionArgTypeInfo_< _Args   >()...
    };
    return MakeView(gSignatureTypeInfos);
}
//----------------------------------------------------------------------------
template <typename _Result, typename... _Args>
TMemoryView<const IMetaTypeVirtualTraits* const> TMetaFunctionImpl<_Result, _Args...>::SignatureTraits() const {
    static const IMetaTypeVirtualTraits* const gSignatureTraits[] = {
        details::MetaFunctionArgVirtualTraits_< _Result >(),
        details::MetaFunctionArgVirtualTraits_< _Args   >()...
    };
    return MakeView(gSignatureTraits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedFunction<_Result COMMA _Class COMMA _Args...>, template <typename _Result COMMA typename _Class COMMA typename... _Args>)
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
TMetaTypedFunction<_Result, _Class, _Args...>::TMetaTypedFunction(const FName& name, EFlags attributes, func_type func)
    : parent_type(name, attributes, sizeof...(_Args))
    , _func(func) {
    Assert(_func);
    SetOutputFlags_(details::TFunctionOutputFlag_<0, _Args...>::value);
}
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
bool TMetaTypedFunction<_Result, _Class, _Args...>::Invoke(FMetaObject* src, PMetaAtom& presult, const TMemoryView<const PMetaAtom>& args) const {
    Assert(src);
    Assert(args.size() == sizeof...(_Args));

    _Class* const typedSrc = src->RTTI_Cast<_Class>();
    if (nullptr == typedSrc)
        return false;

    details::TMetaFunctionPack_<_Args...> pack;
    if (not pack.Unwrap(args, std::index_sequence_for<_Args...>{}))
        return false;

    if (not details::MetaFunctionCall_(typedSrc, _func, presult, pack))
        return false;

    pack.Wrap(args, std::index_sequence_for<_Args...>{});

    return true;
}
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
bool TMetaTypedFunction<_Result, _Class, _Args...>::PromoteInvoke(FMetaObject* src, PMetaAtom& presult, const TMemoryView<const PMetaAtom>& args) const {
    Assert(src);
    Assert(args.size() == sizeof...(_Args));

    _Class* const typedSrc = src->RTTI_Cast<_Class>();
    if (nullptr == typedSrc)
        return false;

    details::TMetaFunctionPack_<_Args...> pack;
    if (not pack.Promote(args, std::index_sequence_for<_Args...>{}))
        return false;

    if (not details::MetaFunctionCallPromote_(typedSrc, _func, presult, pack))
        return false;

    pack.Demote(args, std::index_sequence_for<_Args...>{});

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
