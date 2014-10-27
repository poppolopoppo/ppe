#pragma once

#include "Core/Core.h"

#include <tuple>
#include <type_traits>

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename... _Args>
using Tuple = std::tuple<_Args...>;
//----------------------------------------------------------------------------
template <typename... _Args>
std::tuple<typename std::remove_reference<_Args>::type... > MakeTuple(_Args&&... args) {
    return std::make_tuple(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
struct TupleMerger {
    template <typename T>
    struct TupleWrap {
        typedef std::tuple< typename std::remove_reference<T>::type > type;
        type operator ()(typename std::remove_reference<T>::type&& value) const {
            return std::make_tuple(std::forward<T>(value));
        }
    };

    template <typename... _Args>
    struct TupleWrap< std::tuple<_Args...> > {
        typedef std::tuple<_Args...> type;
        type operator ()(type&& value) const {
            return std::forward<type>(value);
        }
    };

    template <typename U, typename V>
    struct TupleMerged {};

    template <typename... _LhsArgs, typename... _RhsArgs>
    struct TupleMerged< std::tuple<_LhsArgs...>, std::tuple<_RhsArgs...> > {
        typedef std::tuple<_LhsArgs..., _RhsArgs...> type;
    };

    typedef TupleWrap< typename std::remove_reference<_Lhs>::type > lhs_tuple;
    typedef TupleWrap< typename std::remove_reference<_Rhs>::type > rhs_tuple;

    typedef typename TupleMerged<
            typename lhs_tuple::type,
            typename rhs_tuple::type    >::type
            type;

    type operator ()(_Lhs&& lhs, _Rhs&& rhs) const {
        return std::tuple_cat(
            lhs_tuple()(std::move(lhs)),
            rhs_tuple()(std::move(rhs))
            );
    }
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
typename TupleMerger<_Lhs, _Rhs>::type MergeTuple(_Lhs&& lhs, _Rhs&& rhs) {
    return TupleMerger<_Lhs, _Rhs>()(std::forward<_Lhs>(lhs), std::forward<_Rhs>(rhs));
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
