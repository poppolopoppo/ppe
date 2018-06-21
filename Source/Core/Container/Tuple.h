#pragma once

#include "Core/Core.h"

#include <functional>
#include <tuple>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename... _Args>
using TTuple = std::tuple<_Args...>;
//----------------------------------------------------------------------------
template <typename... _Args>
using TTupleDecay = TTuple< Meta::TDecay<_Args>... >;
//----------------------------------------------------------------------------
template <typename... _Args>
std::tuple<Meta::TRemoveReference<_Args>... > MakeTuple(_Args&&... args) {
    return std::make_tuple(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename... _Args>
struct TIsPod< TTuple<_Args...> > : std::bool_constant< TIsPod_v<_Args...> >
{};
template <typename... _Args>
TTuple<_Args...> NoInitType(TType< TTuple<_Args...> >) {
	return MakeTuple(MakeNoInit<_Args>()...);
}
template <typename... _Args>
TTuple<_Args...> ForceInitType(TType< TTuple<_Args...> >) {
    return MakeTuple(MakeForceInit<_Args>()...);
}
template <typename... _Args>
void Construct(TTuple<_Args...>* p, FNoInit) {
	Construct(p, MakeNoInit<_Args>()...);
}
template <typename... _Args>
void Construct(TTuple<_Args...>* p, FForceInit) {
    Construct(p, MakeForceInit<_Args>()...);
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename... _Args, size_t... _Idx>
hash_t hash_tuple_(const TTuple<_Args...>& tuple, std::index_sequence<_Idx...>) {
    hash_t h{ CORE_HASH_VALUE_SEED };
    hash_combine(h, std::get<_Idx>(tuple)...);
    return h;
}
} //!details
template <typename... _Args>
FORCE_INLINE hash_t hash_value(const TTuple<_Args...>& tuple) {
    return details::hash_tuple_(tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
struct TTupleMerger {
    template <typename T>
    struct TTupleWrap {
        typedef TTuple< Meta::TRemoveReference<T> > type;
        type operator ()(Meta::TRemoveReference<T>&& value) const {
            return std::make_tuple(std::forward<T>(value));
        }
    };

    template <typename... _Args>
    struct TTupleWrap< TTuple<_Args...> > {
        typedef TTuple<_Args...> type;
        type operator ()(type&& value) const {
            return std::forward<type>(value);
        }
    };

    template <typename U, typename V>
    struct TTupleMerged {};

    template <typename... _LhsArgs, typename... _RhsArgs>
    struct TTupleMerged< TTuple<_LhsArgs...>, TTuple<_RhsArgs...> > {
        typedef std::tuple<_LhsArgs..., _RhsArgs...> type;
    };

    typedef TTupleWrap< Meta::TRemoveReference<_Lhs> > lhs_tuple;
    typedef TTupleWrap< Meta::TRemoveReference<_Rhs> > rhs_tuple;

    typedef typename TTupleMerged<
        typename lhs_tuple::type,
        typename rhs_tuple::type
    >::type type;

    type operator ()(_Lhs&& lhs, _Rhs&& rhs) const {
        return std::tuple_cat(
            lhs_tuple()(std::move(lhs)),
            rhs_tuple()(std::move(rhs)) );
    }
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
typename TTupleMerger<_Lhs, _Rhs>::type MergeTuple(_Lhs&& lhs, _Rhs&& rhs) {
    return TTupleMerger<_Lhs, _Rhs>()(std::forward<_Lhs>(lhs), std::forward<_Rhs>(rhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return func(std::get<_Index>(args)...);
}
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return func(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms>
_Return Call(_Return (*func)(_Args...), const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename... _Args, typename... _Prms>
_Return Call(_Return(*func)(_Args...), TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return Call(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return Call(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return Call(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return Call(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <size_t _Index, typename _Char, typename... _Args>
void PrintTuple_(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple, std::false_type) {}
template <size_t _Index, typename _Char, typename... _Args>
void PrintTuple_(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple, std::true_type) {
    oss << std::get<_Index>(tuple);
    PrintTuple_<_Index + 1>(oss, tuple);
}
template <size_t _Index, typename _Char, typename... _Args>
void PrintTuple_(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple) {
    PrintTuple_(oss, tuple, typename std::integral_constant<bool, _Index < sizeof...(_Args)>::type{});
}
} //!details
template <typename _Char, typename... _Args>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple) {
    details::PrintTuple_<0>(oss, tuple);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
