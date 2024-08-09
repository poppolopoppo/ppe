#pragma once

#include "Meta/Aliases.h"
#include "Meta/ForRange.h"
#include "Meta/TypeTraits.h"

#include "Memory/PtrRef.h"
#include "Memory/RefPtr.h"

#include <functional>
#include <tuple>
#include <variant>

// https://godbolt.org/z/88oqEYq3j

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TCallableObject<>
//----------------------------------------------------------------------------
template <typename T, class = decltype(&T::operator ())>
struct TCallableObject;
//----------------------------------------------------------------------------
// Free function
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
struct TCallableObject<_Return(_Args...), void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = false;

    template <typename T>
    using value_type = T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_invocable_r_v<_Return, T..., _Args...>;

    _Return (*FreeFunction)(_Args...) {};

    CONSTEXPR TCallableObject(_Return (*fn)(_Args...)) NOEXCEPT
        : FreeFunction(fn)
    {}

    CONSTEXPR operator bool () const NOEXCEPT {
        return (!!FreeFunction);
    }

    CONSTEXPR _Return operator ()(_Args... args) const {
        return (FreeFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
TCallableObject(_Return (*fn)(_Args...)) NOEXCEPT -> TCallableObject<_Return(_Args...), void>;
//----------------------------------------------------------------------------
// Free function with noexcept
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
struct TCallableObject<_Return(_Args...) NOEXCEPT, void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = true;

    template <typename T>
    using value_type = T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_nothrow_invocable_r_v<_Return, T..., _Args...>;

    _Return (*FreeFunction)(_Args...) NOEXCEPT {};

    CONSTEXPR TCallableObject(_Return (*fn)(_Args...) NOEXCEPT) NOEXCEPT
        : FreeFunction(fn)
    {}

    CONSTEXPR _Return operator ()(_Args... args) const NOEXCEPT {
        return (FreeFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename... _Args>
TCallableObject(_Return (*fn)(_Args...) NOEXCEPT) NOEXCEPT -> TCallableObject<_Return(_Args...) NOEXCEPT, void>;
//----------------------------------------------------------------------------
// Member function
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
struct TCallableObject<_Return(_Class::*)(_Args...), void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = 1/* object reference */ + sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = false;

    template <typename T>
    using value_type = T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_invocable_r_v<_Return, T..., _Args...>;

    _Return (_Class::*MemberFunction)(_Args...) {};

    CONSTEXPR TCallableObject(_Return (_Class::*fn)(_Args...)) NOEXCEPT
        : MemberFunction(fn)
    {}

    CONSTEXPR _Return operator ()(TPtrRef<_Class> obj, _Args... args) const {
        return (obj->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TRefPtr<_Class>& obj, _Args... args) const {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TSafePtr<_Class>& obj, _Args... args) const {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
TCallableObject(_Return (_Class::*fn)(_Args...)) NOEXCEPT -> TCallableObject<_Return(_Class::*)(_Args...), void>;
//----------------------------------------------------------------------------
// Const member function
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
struct TCallableObject<_Return(_Class::*)(_Args...) const, void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = 1/* object reference */ + sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = false;

    template <typename T>
    using value_type = const T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_invocable_r_v<_Return, T..., _Args...>;

    _Return (_Class::*MemberFunction)(_Args...) const {};

    CONSTEXPR TCallableObject(_Return (_Class::*fn)(_Args...) const) NOEXCEPT
        : MemberFunction(fn)
    {}

    CONSTEXPR _Return operator ()(TPtrRef<const _Class> obj, _Args... args) const {
        return (obj->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TRefPtr<const _Class>& obj, _Args... args) const {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TSafePtr<const _Class>& obj, _Args... args) const {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
TCallableObject(_Return (_Class::*fn)(_Args...) const) NOEXCEPT -> TCallableObject<_Return(_Class::*)(_Args...) const, void>;
//----------------------------------------------------------------------------
// Member function with noexcept
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
struct TCallableObject<_Return(_Class::*)(_Args...) NOEXCEPT, void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = 1/* object reference */ + sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = true;

    template <typename T>
    using value_type = T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_nothrow_invocable_r_v<_Return, T..., _Args...>;

    _Return (_Class::*MemberFunction)(_Args...) NOEXCEPT {};

    CONSTEXPR TCallableObject(_Return (_Class::*fn)(_Args...) NOEXCEPT) NOEXCEPT
        : MemberFunction(fn)
    {}

    CONSTEXPR _Return operator ()(TPtrRef<_Class> obj, _Args... args) const NOEXCEPT {
        return (obj->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TRefPtr<_Class>& obj, _Args... args) const NOEXCEPT  {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TSafePtr<_Class>& obj, _Args... args) const NOEXCEPT {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
TCallableObject(_Return (_Class::*fn)(_Args...) NOEXCEPT) NOEXCEPT -> TCallableObject<_Return(_Class::*)(_Args...) NOEXCEPT, void>;
//----------------------------------------------------------------------------
// Const member function with noexcept
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
struct TCallableObject<_Return(_Class::*)(_Args...) const NOEXCEPT, void> {
    using return_type = _Return;
    using function_type = _Return(_Args...);

    static CONSTEXPR size_t arity_v = 1/* object reference */ + sizeof...(_Args);
    static CONSTEXPR bool is_noexcept_v = true;

    template <typename T>
    using value_type = const T;
    template <typename... T>
    static constexpr bool is_invocable_v = std::is_nothrow_invocable_r_v<_Return, T..., _Args...>;

    _Return (_Class::*MemberFunction)(_Args...) const NOEXCEPT {};

    CONSTEXPR TCallableObject(_Return (_Class::*fn)(_Args...) const NOEXCEPT) NOEXCEPT
        : MemberFunction(fn)
    {}

    CONSTEXPR _Return operator ()(TPtrRef<const _Class> obj, _Args... args) const NOEXCEPT {
        return (obj->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TRefPtr<const _Class>& obj, _Args... args) const NOEXCEPT  {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
    CONSTEXPR _Return operator ()(const TSafePtr<const _Class>& obj, _Args... args) const NOEXCEPT {
        return (obj.get()->*MemberFunction)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Return, typename _Class, typename... _Args>
TCallableObject(_Return (_Class::*fn)(_Args...) const NOEXCEPT) NOEXCEPT -> TCallableObject<_Return(_Class::*)(_Args...) const NOEXCEPT, void>;
//----------------------------------------------------------------------------
// Functor overloading operator()
//----------------------------------------------------------------------------
template <typename T, class _Operator>
struct TCallableObject : TCallableObject<_Operator, void> {
    using parent_type = TCallableObject<_Operator, void>;

    using typename parent_type::return_type;
    using typename parent_type::function_type;

    static CONSTEXPR size_t arity_v = parent_type::arity_v;
    static CONSTEXPR bool is_noexcept_v = parent_type::is_noexcept_v;

    template <typename T>
    using value_type = typename parent_type::template value_type<T>;
    template <typename... T>
    static constexpr bool is_invocable_v = parent_type::template is_invocable_v<T...>;

    CONSTEXPR TCallableObject(const T& ) NOEXCEPT
        : parent_type(&T::operator ())
    {}

    using parent_type::operator ();
};
//----------------------------------------------------------------------------
template <typename T>
TCallableObject(const T& ) NOEXCEPT -> TCallableObject<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TStaticFunction<> avoid storing a pointer by using retrieving the function from its type
//----------------------------------------------------------------------------
template <auto _Callable, decltype(TCallableObject{ _Callable })* = nullptr>
struct TStaticFunction {
    using callable_type = decltype(TCallableObject{ _Callable });

    using return_type = typename callable_type::return_type;
    using function_type = typename callable_type::function_type;

    static CONSTEXPR size_t arity_v = callable_type::arity_v;
    static CONSTEXPR bool is_noexcept_v = callable_type::is_noexcept_v;

    template <typename T>
    using value_type = typename callable_type::template value_type<T>;
    template <typename... T>
    static constexpr bool is_invocable_v = callable_type::template is_invocable_v<T...>;

    NODISCARD CONSTEXPR callable_type operator *() const NOEXCEPT {
        return { _Callable };
    }

    template <typename... _Args>
    CONSTEXPR auto operator ()(_Args&&... args) const NOEXCEPT_IF(callable_type::is_noexcept_v) {
        return (callable_type{ _Callable })(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <auto _Callable>
inline CONSTEXPR const TStaticFunction<_Callable> StaticFunction{};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Ignore extra-arguments iff prefix signature matches
//----------------------------------------------------------------------------
template <typename _Functor, typename... _Args, class _Callable = decltype(TCallableObject{ std::declval<_Functor&&>() }) >
CONSTEXPR auto VariadicFunctor(_Functor&& rfunc, _Args&&... args) NOEXCEPT_IF(_Callable::is_noexcept_v) {
    _Callable callable{ rfunc };
    return Meta::static_for<_Callable::arity_v>([&](auto... index) {
        auto full_args = std::forward_as_tuple(std::forward<_Args>(args)...);
        auto call_args = std::forward_as_tuple(std::move(rfunc), std::get<index>(full_args)...);

        IF_CONSTEXPR(std::is_void_v<typename _Callable::return_type>) {
            std::apply(callable, std::move(call_args));
            return true;
        }
        else {
            return std::apply(callable, std::move(call_args));
        }
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://en.cppreference.com/w/cpp/utility/variant/visit
//----------------------------------------------------------------------------
namespace details {
template<class... Ts> struct FOverloaded_ : Ts... { using Ts::operator()...; };
template<class... Ts> FOverloaded_(Ts...) -> FOverloaded_<Ts...>;
} //!details
//----------------------------------------------------------------------------
template <typename... _Visitor>
NODISCARD CONSTEXPR auto Overloaded(_Visitor&&... visitor) {
    return details::FOverloaded_{ std::forward<_Visitor>(visitor)... };
}
//----------------------------------------------------------------------------
template <typename... _Variant, typename... _Visitor>
CONSTEXPR auto Visit(std::variant<_Variant...>& variant, _Visitor&&... visitor) {
    return std::visit(
        Overloaded( std::forward<_Visitor>(visitor)... ),
        variant );
}
//----------------------------------------------------------------------------
template <typename... _Variant, typename... _Visitor>
CONSTEXPR auto Visit(const std::variant<_Variant...>& variant, _Visitor&&... visitor) {
    return std::visit(
        Overloaded( std::forward<_Visitor>(visitor)... ),
        variant );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
