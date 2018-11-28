#pragma once

#include "Core_fwd.h"

#include "Container/Array.h"
#include "Container/Tuple.h"

#include <utility>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Merge 2 different tuples together
//----------------------------------------------------------------------------
namespace details {
template <typename _Lhs, typename _Rhs>
struct TTupleMerger {
    template <typename T>
    struct TTupleWrap {
        typedef TTuple< Meta::TRemoveReference<T> > type;
        static CONSTEXPR type make(Meta::TRemoveReference<T>&& value) NOEXCEPT {
            return std::make_tuple(std::forward<T>(value));
        }
    };

    template <typename... _Args>
    struct TTupleWrap< TTuple<_Args...> > {
        typedef TTuple<_Args...> type;
        static CONSTEXPR type make(type&& value) NOEXCEPT {
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

    static CONSTEXPR type cat(_Lhs&& lhs, _Rhs&& rhs) NOEXCEPT {
        return std::tuple_cat(
            lhs_tuple::make(std::move(lhs)),
            rhs_tuple::make(std::move(rhs)) );
    }
};
}//!details
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
CONSTEXPR auto MergeTuple(_Lhs&& lhs, _Rhs&& rhs) NOEXCEPT
-> typename details::TTupleMerger<_Lhs, _Rhs>::type {
    return details::TTupleMerger<_Lhs, _Rhs>::cat(
        std::forward<_Lhs>(lhs), std::forward<_Rhs>(rhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call any function or member function pointer with a tuple containing all args
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return func(std::get<_Index>(args)...);
}
template<typename _Return, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (*func)(_Args...), TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return func(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms, size_t... _Index>
FORCE_INLINE _Return CallHelper_(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args, std::index_sequence<_Index...>) {
    NOOP(args);
    return (src->*member)(std::get<_Index>(args)...);
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template<typename _Return, typename... _Args, typename... _Prms>
_Return CallTuple(_Return (*func)(_Args...), const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(*func)(_Args...), TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(func, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return (_Class::*member)(_Args...), _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...), _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...) const, const _Class* src, const TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
template<typename _Return, typename _Class, typename... _Args, typename... _Prms>
_Return CallTuple(_Return(_Class::*member)(_Args...) const, const _Class* src, TTuple<_Prms...>& args) {
    STATIC_ASSERT(sizeof...(_Args) == sizeof...(_Prms));
    return details::CallHelper_(member, src, args, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return an array of results
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Lambda, typename... _Args, size_t... _Idx>
CONSTEXPR TArray<T, sizeof...(_Args)> MapTuple_(_Lambda map, const TTuple<_Args...>& tuple, std::index_sequence<_Idx...>) {
    return { map(std::get<_Idx>(tuple))... };
}
} //!details
//----------------------------------------------------------------------------
template <typename T, typename... _Args, typename _Lambda>
CONSTEXPR TArray<T, sizeof...(_Args)> MapTuple(const TTuple<_Args...>& tuple, _Lambda map) {
    return details::MapTuple_<T>(map, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Call a function on every member of tuple and return a tuple of results
//----------------------------------------------------------------------------
namespace details {
template <typename _Lambda, typename... _Args, size_t... _Idx>
CONSTEXPR auto ProjectTuple_(_Lambda projector, const TTuple<_Args...>& tuple, std::index_sequence<_Idx...>) {
    return std::make_tuple(projector(std::get<_Idx>(tuple))... );
}
} //!details
//----------------------------------------------------------------------------
template <typename... _Args, typename _Lambda>
CONSTEXPR auto ProjectTuple(const TTuple<_Args...>& tuple, _Lambda projector) {
    return details::ProjectTuple_(projector, tuple, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Count the number of fields in a structure
// https://github.com/apolukhin/magic_get/blob/develop/include/boost/pfr/detail/fields_count.hpp
//----------------------------------------------------------------------------
#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-braces"
#   pragma clang diagnostic ignored "-Wundefined-inline"
#   pragma clang diagnostic ignored "-Wundefined-internal"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
//----------------------------------------------------------------------------
namespace details {
///////////////////// General utility stuff
template <std::size_t Index>
using size_t_ = std::integral_constant<std::size_t, Index >;

///////////////////// Structure that can be converted to reference to anything
struct ubiq_lref_constructor {
    std::size_t ignore;
    template <class Type> constexpr operator Type&() const noexcept; // Undefined, allows initialization of reference fields (T& and const T&)
};

///////////////////// Structure that can be converted to rvalue reference to anything
struct ubiq_rref_constructor {
    std::size_t ignore;
    template <class Type> constexpr operator Type&&() const noexcept; // Undefined, allows initialization of rvalue reference fields and move-only types
};

///////////////////// Structure that can be converted to reference to anything except reference to T
template <class T, bool IsCopyConstructible>
struct ubiq_constructor_except {
    template <class Type> constexpr operator std::enable_if_t<!std::is_same<T, Type>::value, Type&> () const noexcept; // Undefined
};

template <class T>
struct ubiq_constructor_except<T, false> {
    template <class Type> constexpr operator std::enable_if_t<!std::is_same<T, Type>::value, Type&&> () const noexcept; // Undefined
};

///////////////////// Hand-made is_aggregate_initializable_n<T> trait

// `std::is_constructible<T, ubiq_constructor_except<T>>` consumes a lot of time, so we made a separate lazy trait for it.
template <std::size_t N, class T> struct is_single_field_and_aggregate_initializable: std::false_type {};
template <class T> struct is_single_field_and_aggregate_initializable<1, T>: std::integral_constant<
    bool, !std::is_constructible<T, ubiq_constructor_except<T, std::is_copy_constructible<T>::value>>::value
> {};

// Hand-made is_aggregate<T> trait:
// Aggregates could be constructed from `decltype(ubiq_?ref_constructor{I})...` but report that there's no constructor from `decltype(ubiq_?ref_constructor{I})...`
// Special case for N == 1: `std::is_constructible<T, ubiq_?ref_constructor>` returns true if N == 1 and T is copy/move constructible.
template <class T, std::size_t N>
struct is_aggregate_initializable_n {
    template <std::size_t ...I>
    static constexpr bool is_not_constructible_n(std::index_sequence<I...>) noexcept {
        return (!std::is_constructible<T, decltype(ubiq_lref_constructor{I})...>::value && !std::is_constructible<T, decltype(ubiq_rref_constructor{I})...>::value)
            || is_single_field_and_aggregate_initializable<N, T>::value
        ;
    }

    static constexpr bool value =
           std::is_empty<T>::value
        || std::is_array<T>::value
        || std::is_fundamental<T>::value
        || is_not_constructible_n(std::make_index_sequence<N>{})
    ;
};

///////////////////// Helper for SFINAE on fields count
template <class T, std::size_t... I, class /*Enable*/ = std::enable_if_t<std::is_copy_constructible<T>::value> >
constexpr auto enable_if_constructible_helper(std::index_sequence<I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_lref_constructor{I}... })>::type;

template <class T, std::size_t... I, class /*Enable*/ = std::enable_if_t<!std::is_copy_constructible<T>::value> >
constexpr auto enable_if_constructible_helper(std::index_sequence<I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_rref_constructor{I}... })>::type;

template <class T, std::size_t N, class /*Enable*/ = decltype( enable_if_constructible_helper<T>(std::make_index_sequence<N>()) ) >
using enable_if_constructible_helper_t = std::size_t;

///////////////////// Non greedy fields count search. Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T)).

// PPE BEGIN (POP) - This is not working for me, replacing with a simpler classical trait
#if 0 // pfr :
template <class T, std::size_t N>
constexpr std::size_t detect_fields_count(size_t_<N>, size_t_<N>, long) noexcept {
    return N;
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(size_t_<Begin>, size_t_<Middle>, int) noexcept;

template <class T, std::size_t Begin, std::size_t Middle>
constexpr auto detect_fields_count(size_t_<Begin>, size_t_<Middle>, long) noexcept
    -> enable_if_constructible_helper_t<T, Middle> {
    using next_t = size_t_<Middle + (Middle - Begin + 1) / 2>;
    return details::detect_fields_count<T>(size_t_<Middle>{}, next_t{}, 1L);
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(size_t_<Begin>, size_t_<Middle>, int) noexcept {
    using next_t = size_t_<(Begin + Middle) / 2>;
    return details::detect_fields_count<T>(size_t_<Begin>{}, next_t{}, 1L);
}
#else // pop :
template <typename T, size_t Begin, size_t Middle>
struct detect_fields_count_t {
    template <size_t X, class = enable_if_constructible_helper_t<T, X> >
    static constexpr size_t Next(long) noexcept {
        return detect_fields_count_t<T, Middle, Middle + (Middle - Begin + 1) / 2>::value;
    }
    template <size_t X>
    static constexpr size_t Next(int) noexcept {
        return detect_fields_count_t<T, Begin, (Begin + Middle) / 2>::value;
    }
    STATIC_CONST_INTEGRAL(size_t, value, Next<Middle>(1L));
};

template <typename T, size_t N>
struct detect_fields_count_t<T, N, N> {
    STATIC_CONST_INTEGRAL(size_t, value, N);
};

template <class T, size_t Begin, size_t Middle>
constexpr std::size_t detect_fields_count(size_t_<Begin>, size_t_<Middle>, long) noexcept {
    return detect_fields_count_t<T, Begin, Middle>::value;
}
#endif
// PPE END

///////////////////// Greedy search. Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T))*T in worst case.
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
    return details::detect_fields_count_greedy_remember<T, N>(size_t_<N>{}, 1L);
}

template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(size_t_<Begin>, size_t_<Last>) noexcept {
    constexpr std::size_t middle = Begin + (Last - Begin) / 2;
    constexpr std::size_t fields_count_big = details::detect_fields_count_greedy<T>(size_t_<middle + 1>{}, size_t_<Last>{});
    constexpr std::size_t fields_count_small = details::detect_fields_count_greedy<T>(size_t_<Begin>{}, size_t_<fields_count_big ? Begin : middle>{});
    return fields_count_big ? fields_count_big : fields_count_small;
}

///////////////////// Choosing between array size, greedy and non greedy search.
template <class T, std::size_t N>
constexpr auto detect_fields_count_dispatch(size_t_<N>, long, long) noexcept
    -> typename std::enable_if<std::is_array<T>::value, std::size_t>::type {
    return sizeof(T) / sizeof(typename std::remove_all_extents<T>::type);
}

template <class T, std::size_t N>
constexpr auto detect_fields_count_dispatch(size_t_<N>, long, int) noexcept
    -> decltype(sizeof(T{})) {
    return details::detect_fields_count<T>(size_t_<0>{}, size_t_<N / 2 + 1>{}, 1L);
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_dispatch(size_t_<N>, int, int) noexcept {
    // T is not default aggregate initializable. It means that at least one of the members is not default constructible,
    // so we have to check all the aggregate initializations for T up to N parameters and return the biggest succeeded
    // (we can not use binary search for detecting fields count).
    return details::detect_fields_count_greedy<T>(size_t_<0>{}, size_t_<N>{});
}

} //!namespace details
///////////////////// Returns non-flattened fields count
template <class T>
constexpr std::size_t struct_num_fields() noexcept {
    using type = std::remove_cv_t<T>;

    static_assert(
        !std::is_reference<type>::value,
        "Attempt to get fields count on a reference. This is not allowed because that could hide an issue and different library users expect different behavior in that case."
    );

    static_assert(
        std::is_copy_constructible<std::remove_all_extents_t<type>>::value || (
            std::is_move_constructible<std::remove_all_extents_t<type>>::value
            && std::is_move_assignable<std::remove_all_extents_t<type>>::value
        ),
        "Type and each field in the type must be copy constructible (or move constructible and move assignable)."
    );

    static_assert(
        !std::is_polymorphic<type>::value,
        "Type must have no virtual function, because otherwise it is not aggregate initializable."
    );

#ifdef __cpp_lib_is_aggregate
    static_assert(
        std::is_aggregate<type>::value             // Does not return `true` for build in types.
        || std::is_standard_layout<type>::value,   // Does not return `true` for structs that have non standard layout members.
        "Type must be aggregate initializable."
    );
#endif

// Can't use the following. See the non_std_layout.cpp test.
//    static_assert(
//        std::is_standard_layout<type>::value,   // Does not return `true` for structs that have non standard layout members.
//        "Type must be aggregate initializable."
//    );

// PPE BEGIN (POP) - we don't handle bit fields
    //constexpr std::size_t max_fields_count = (sizeof(type) * CHAR_BIT); // We multiply by CHAR_BIT because the type may have bit fields in T
    constexpr std::size_t max_fields_count = sizeof(type);
// PPE END
    constexpr std::size_t result = details::detect_fields_count_dispatch<type>(details::size_t_<max_fields_count>{}, 1L, 1L);

    static_assert(
        details::is_aggregate_initializable_n<type, result>::value,
        "Types with user specified constructors (non-aggregate initializable types) are not supported."
    );

    static_assert(
        result != 0 || std::is_empty<type>::value || std::is_fundamental<type>::value || std::is_reference<type>::value,
        "If there's no other failed static asserts then something went wrong. Please report this issue to the github along with the structure you're reflecting."
    );

    return result;
}
//----------------------------------------------------------------------------
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wrap a structure instance in a tuple of references
// https://github.com/apolukhin/magic_get/blob/develop/include/boost/pfr/detail/core17_generated.hpp
//----------------------------------------------------------------------------
#if _HAS_CXX17
//----------------------------------------------------------------------------
static constexpr size_t MaxArityForTieAsTuple = 8;
//----------------------------------------------------------------------------
template <typename T>
constexpr bool has_tie_as_tuple() noexcept {
    using type = std::remove_cv_t<T>;
    if constexpr (not
        (!std::is_reference<type>::value) &&
        (std::is_copy_constructible<std::remove_all_extents_t<type>>::value || (
            std::is_move_constructible<std::remove_all_extents_t<type>>::value
            && std::is_move_assignable<std::remove_all_extents_t<type>>::value
            )) &&
#ifdef __cpp_lib_is_aggregate
        (std::is_aggregate<type>::value             // Does not return `true` for build in types.
        || std::is_standard_layout<type>::value) && // Does not return `true` for structs that have non standard layout members.
#endif
        (!std::is_polymorphic<type>::value ) )
        return false;
    constexpr size_t numFields = details::detect_fields_count_greedy<T>(
        details::size_t_<0>{},
        details::size_t_<MaxArityForTieAsTuple + 1>{} );
    return (numFields &&
            details::is_aggregate_initializable_n<type, numFields>::value);
}
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
    auto&[a, b] = val;
    return std::tie(a, b);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<3>) noexcept {
    auto&[a, b, c] = val;
    return std::tie(a, b, c);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<4>) noexcept {
    auto&[a, b, c, d] = val;
    return std::tie(a, b, c, d);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<5>) noexcept {
    auto&[a, b, c, d, e] = val;
    return std::tie(a, b, c, d, e);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<6>) noexcept {
    auto&[a, b, c, d, e, f] = val;
    return std::tie(a, b, c, d, e, f);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<7>) noexcept {
    auto&[a, b, c, d, e, f, g] = val;
    return std::tie(a, b, c, d, e, f, g);
}
template <class T>
constexpr auto tie_as_tuple(T& val, size_t_<8>) noexcept {
    auto&[a, b, c, d, e, f, g, h] = val;
    return std::tie(a, b, c, d, e, f, g, h);
}
} //!details
template <typename T, size_t _NumFields = struct_num_fields<T>() >
constexpr auto tie_as_tuple(T& val) noexcept {
    STATIC_ASSERT(MaxArityForTieAsTuple == 8); // add details::tie_as_tuple() overloads if this number was incremented
    return details::tie_as_tuple(val, details::size_t_<_NumFields>{});
}
//----------------------------------------------------------------------------
#else //_HAS_CXX17
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool has_tie_as_tuple() noexcept {
    return false;
}
//----------------------------------------------------------------------------
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
