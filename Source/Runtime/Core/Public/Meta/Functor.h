#pragma once

#include "Meta/TypeTraits.h"

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
    using func_type = _Ret(_Args...);
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, _Args... args, _Unused...) {
        return f(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename... _Args>
struct TFunctorTraits_<_Ret(_Args...) noexcept> {
    using func_type = _Ret(_Args...);
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, _Args... args, _Unused...) noexcept {
        return f(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename... _Args>
struct TFunctorTraits_<_Ret(*)(_Args...)> {
    using func_type = _Ret(*)(_Args...);
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, _Args... args, _Unused...) {
        return f(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename... _Args>
struct TFunctorTraits_<_Ret(*)(_Args...) noexcept> {
    using func_type = _Ret(*)(_Args...);
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, _Args... args, _Unused...) noexcept {
        return f(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename _Class, typename... _Args>
struct TFunctorTraits_<_Ret(_Class::*)(_Args...)> {
    using func_type = _Ret(_Class::*)(_Args...);
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, _Class* obj, _Args... args, _Unused...) {
        return (obj->*f)(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename _Class, typename... _Args>
struct TFunctorTraits_<_Ret(_Class::*)(_Args...) const> {
    using func_type = _Ret(_Class::*)(_Args...) const;
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, const _Class* obj, _Args... args, _Unused...) {
        return (obj->*f)(std::forward<_Args>(args)...);
    }
};
template <typename _Ret, typename _Class, typename... _Args>
struct TFunctorTraits_<_Ret(_Class::*)(_Args...) const noexcept> {
    using func_type = _Ret(_Class::*)(_Args...) const noexcept;
    using return_type = _Ret;
    static constexpr size_t arity = sizeof...(_Args);
    template <typename... _Unused>
    static constexpr return_type varcall(func_type f, const _Class* obj, _Args... args, _Unused...) noexcept {
        return (obj->*f)(std::forward<_Args>(args)...);
    }
};
template <typename T>
struct TFunctorTraits_ : TFunctorTraits_<decltype(&T::operator())> {
    using parent_type = TFunctorTraits_<decltype(&T::operator())>;
    using typename parent_type::return_type;
    using parent_type::arity;
    template <typename... _Args>
    static constexpr return_type varcall(T& obj, _Args... args) {
        return parent_type::template varcall(&T::operator(), &obj, std::forward<_Args>(args)...);
    }
    template <typename... _Args>
    static constexpr return_type varcall(T&& obj, _Args... args) {
        return parent_type::template varcall(&T::operator(), &obj, std::forward<_Args>(args)...);
    }
    template <typename... _Args>
    static constexpr return_type varcall(const T& obj, _Args... args) {
        return parent_type::template varcall(&T::operator(), &obj, std::forward<_Args>(args)...);
    }
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
template <typename _Functor, typename... _Args>
constexpr auto VariadicFunctor(_Functor func, _Args&&... args) {
    using traits_t = details::TFunctorTraits_<_Functor>;
    IF_CONSTEXPR(std::is_same_v<void, Meta::TDecay<typename traits_t::return_type>>) {
        traits_t::template varcall(std::forward<_Functor>(func), std::forward<_Args>(args)...);
        return true;
    }
    else {
        return traits_t::template varcall(std::forward<_Functor>(func), std::forward<_Args>(args)...);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
