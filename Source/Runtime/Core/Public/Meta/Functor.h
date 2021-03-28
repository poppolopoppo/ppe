#pragma once

#include "Meta/TypeTraits.h"

#include <tuple>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TFunctorTraits_;
template <typename _Ret, typename... _Args>
struct TFunctorTraits_<_Ret(_Args...)> {
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
};
template <typename _Ret, typename... _Args>
struct TFunctorTraits_<_Ret(*)(_Args...)> {
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
};
template <typename _Ret, typename _Class, typename... _Args>
struct TFunctorTraits_<_Ret(_Class::*)(_Args...)> {
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
};
template <typename _Ret, typename _Class, typename... _Args>
struct TFunctorTraits_<_Ret(_Class::*)(_Args...) const> {
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
};
template <typename T>
struct TFunctorTraits_ : TFunctorTraits_<decltype(&T::operator())> {
    using typename TFunctorTraits_<decltype(&T::operator())>::return_type;
    using TFunctorTraits_<decltype(&T::operator())>::arity;
};
} //!details
//----------------------------------------------------------------------------
template <typename T>
using TFunctorReturn = typename details::TFunctorTraits_<T>::return_type;
//----------------------------------------------------------------------------
template <typename T>
constexpr size_t FunctorArity() {
    return details::TFunctorTraits_<T>::arity;
}
//----------------------------------------------------------------------------
namespace details {
template <typename _Functor, size_t... _Idx, typename... _Args>
constexpr auto VariadicCall_(_Functor func, std::index_sequence<_Idx...>, std::tuple<_Args...>&& args) {
    return func(std::get<_Idx>(args)...);
}
} //!details
//----------------------------------------------------------------------------
template <typename _Functor, typename... _Args>
constexpr auto VariadicFunctor(_Functor func, _Args&&... args) {
    constexpr size_t arity = FunctorArity<_Functor>();
    return details::VariadicCall_(
        std::forward<_Functor>(func),
        std::make_index_sequence<arity>(),
        std::make_tuple(std::forward<_Args>(args)...) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
