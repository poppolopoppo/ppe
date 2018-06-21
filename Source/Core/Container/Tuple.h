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
template <typename _Char, typename... _Args, size_t... _Index>
void PrintTuple_(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple, std::index_sequence<_Index...>) {
    oss << ... << std::get<_Index>(tuple);
}
} //!details
template <typename _Char, typename... _Args>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TTuple<_Args...>& tuple) {
    details::PrintTuple_(oss, tuple, std::index_sequence_for<_Args...>{});
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Count the number of fields in a structure
// https://github.com/apolukhin/magic_get/blob/develop/include/boost/pfr/detail/fields_count.hpp
//----------------------------------------------------------------------------
namespace details {
template <size_t I>
using size_t_ = std::integral_constant<size_t, I>;
// Structure that can be converted to reference to anything
struct ubiq_constructor {
    size_t ignored;
    template <class T> constexpr operator T&() const noexcept; // Undefined, allows initialization of reference fields (T& and const T&)
    //template <class T> constexpr operator T&&() const noexcept; // Undefined, allows initialization of rvalue reference fields and move-only types
};
// Structure that can be converted to reference to anything except reference to T
template <class T>
struct ubiq_constructor_except {
    template <class Type> constexpr operator std::enable_if_t<!std::is_same<T, Type>::value, Type&>() const noexcept; // Undefined
};
// `std::is_constructible<T, ubiq_constructor_except<T>>` consumes a lot of time, so we made a separate lazy trait for it.
template <std::size_t N, class T> struct is_single_field_and_aggregate_initializable : std::false_type {};
template <class T> struct is_single_field_and_aggregate_initializable<1, T> : std::integral_constant<
    bool, !std::is_constructible<T, ubiq_constructor_except<T>>::value
> {};

// Hand-made is_aggregate<T> trait:
// Aggregates could be constructed from `decltype(ubiq_constructor{I})...` but report that there's no constructor from `decltype(ubiq_constructor{I})...`
// Special case for N == 1: `std::is_constructible<T, ubiq_constructor>` returns true if N == 1 and T is copy/move constructible.
template <class T, std::size_t N>
struct is_aggregate_initializable_n {
    template <std::size_t ...I>
    static constexpr bool is_not_constructible_n(std::index_sequence<I...>) noexcept {
        return !std::is_constructible<T, decltype(ubiq_constructor{ I })...>::value
            || is_single_field_and_aggregate_initializable<N, T>::value
            ;
    }

    static constexpr bool value =
        std::is_empty<T>::value
        || std::is_fundamental<T>::value
        || is_not_constructible_n(std::make_index_sequence<N>{})
        ;
};
template <class T, size_t... I>
constexpr auto enable_if_constructible_helper(std::index_sequence<I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_constructor{ I }... })>::type;
template <class T, size_t N, class /*Enable*/ = decltype(enable_if_constructible_helper<T>(std::make_index_sequence<N>())) >
using enable_if_constructible_helper_t = size_t;
} //!details
//----------------------------------------------------------------------------
namespace details {
// Deduce fields count from recursive template specialization using binary search
template <class T, std::size_t N>
constexpr auto detect_fields_count_greedy_remember(size_t_<N>, long) noexcept
    -> enable_if_constructible_helper_t<T, N> {
    return N;
}
template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_greedy_remember(size_t_<N>, int) noexcept {
    return 0;
}
template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_greedy(size_t_<N>, size_t_<N>) noexcept {
    return detect_fields_count_greedy_remember<T>(size_t_<N>{}, 1L);
}
template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(size_t_<Begin>, size_t_<Last>) noexcept {
    constexpr std::size_t middle = Begin + (Last - Begin) / 2;
    constexpr std::size_t fields_count_big = detect_fields_count_greedy<T>(size_t_<middle + 1>{}, size_t_<Last>{});
    constexpr std::size_t fields_count_small = detect_fields_count_greedy<T>(size_t_<Begin>{}, size_t_<fields_count_big ? Begin : middle>{});
    return fields_count_big ? fields_count_big : fields_count_small;
}
} //!details
//----------------------------------------------------------------------------
static constexpr size_t MaxArityForTieAsTuple = 8;
//----------------------------------------------------------------------------
template <typename T>
constexpr size_t struct_num_fields() noexcept {
    using type = std::remove_cv_t<T>;

    static_assert(
        !std::is_reference_v<type>,
        "Attempt to get fields count on a reference. This is not allowed because that could hide an issue and different library users expect different behavior in that case." );

    static_assert(
        std::is_copy_constructible_v<std::remove_all_extents_t<type>>,
        "Type and each field in the type must be copy constructible." );

    static_assert(
        !std::is_polymorphic_v<type>,
        "Type must have no virtual function, because otherwise it is not aggregate initializable." );

    constexpr size_t result = details::detect_fields_count_greedy<type>(
        details::size_t_<0>{},
        details::size_t_<MaxArityForTieAsTuple + 1>{} );

    static_assert(
        details::is_aggregate_initializable_n<type, result>::value,
        "Types with user specified constructors (non-aggregate initializable types) are not supported." );

    static_assert(
        result != 0 || std::is_empty_v<type> || std::is_fundamental_v<type> || std::is_reference_v<type>,
        "Something went wrong. Please report this issue to the github along with the structure you're reflecting." );

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
constexpr bool has_tie_as_tuple() noexcept {
    using type = std::remove_cv_t<T>;
    constexpr size_t result = details::detect_fields_count_greedy<type>(
        details::size_t_<0>{},
        details::size_t_<MaxArityForTieAsTuple + 1>{} );

    return (
        !std::is_reference_v<type> &&
        std::is_copy_constructible_v<std::remove_all_extents_t<type>> &&
        !std::is_polymorphic_v<type> &&
        details::is_aggregate_initializable_n<type, result>::value &&
        (result != 0 || std::is_empty_v<type> || std::is_fundamental_v<type> || std::is_reference_v<type>) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wrap a structure instance in a tuple of references
// https://github.com/apolukhin/magic_get/blob/develop/include/boost/pfr/detail/core17_generated.hpp
//----------------------------------------------------------------------------
#if _HAS_CXX17
//----------------------------------------------------------------------------
// Use new structured bindings with C++17
namespace details {
/*
** /!\ Forbid tying as tuple for types with a single or no element.
**     Those shouldn't be tied as a tuple and add ambiguity to overloads.
**
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<0>) noexcept {
    return TTuple<>{};
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<1>, Meta::TEnableIf< std::is_class_v< std::remove_cv_t<T> > >* = 0) noexcept {
    auto& [a] = val;
    return std::tie(a);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<1>, Meta::TEnableIf< not std::is_class_v< std::remove_cv_t<T> > >* = 0) noexcept {
    return std::tie(val);
}
*/
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<2>) noexcept {
    auto& [a,b] = val;
    return std::tie(a, b);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<3>) noexcept {
    auto& [a, b, c] = val;
    return std::tie(a, b, c);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<4>) noexcept {
    auto& [a, b, c, d] = val;
    return std::tie(a, b, c, d);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<5>) noexcept {
    auto& [a, b, c, d, e] = val;
    return std::tie(a, b, c, d, e);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<6>) noexcept {
    auto& [a, b, c, d, e, f] = val;
    return std::tie(a, b, c, d, e, f);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<7>) noexcept {
    auto& [a, b, c, d, e, f, g] = val;
    return std::tie(a, b, c, d, e, f, g);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<8>) noexcept {
    auto& [a, b, c, d, e, f, g, h] = val;
    return std::tie(a, b, c, d, e, f, g, h);
}
} //!details
template <
    typename T
,   class = Meta::TEnableIf<has_tie_as_tuple<T>()>
,   size_t _NumFields = struct_num_fields<T>()
,   class _Tuple = decltype(details::tie_as_tuple(std::declval<T&>(), details::size_t_<_NumFields>{}))
>
constexpr _Tuple tie_as_tuple(T& val) noexcept {
    STATIC_ASSERT(MaxArityForTieAsTuple == 8); // add details::tie_as_tuple() overloads if this number was incremented
    return details::tie_as_tuple(val, details::size_t_<_NumFields>{} );
}
//----------------------------------------------------------------------------
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
