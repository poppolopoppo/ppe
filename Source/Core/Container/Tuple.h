#pragma once

#include "Core/Core.h"

#include <functional>
#include <iosfwd>
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
template<typename _Return, typename... _Args, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), const TTupleDecay<_Args...>& args, std::index_sequence<_Index...>) {
    return func(std::get<_Index>(args)...);
}
template<typename _Return, typename... _Args, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), TTupleDecay<_Args...>& args, std::index_sequence<_Index...>) {
    return func(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (_Class::*member)(_Args...), _Class* src, const TTupleDecay<_Args...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...), _Class* src, TTupleDecay<_Args...>& args, std::index_sequence<_Index...>) {
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args>
_Return Call(_Return (*func)(_Args...), const TTupleDecay<_Args...>& args) {
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename... _Args>
_Return Call(_Return(*func)(_Args...), TTupleDecay<_Args...>& args) {
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args>
_Return Call(_Return (_Class::*member)(_Args...), _Class* src, const TTupleDecay<_Args...>& args) {
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args>
_Return Call(_Return(_Class::*member)(_Args...), _Class* src, TTupleDecay<_Args...>& args) {
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/*template <
    typename _Char,
    typename _Traits,
    typename... _Args
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const std::tuple<_Args...>& tuple) {
    // TODO (01/14) : grmpf
}*/
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
