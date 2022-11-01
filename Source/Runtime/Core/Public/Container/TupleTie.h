#pragma once

#include "Core_fwd.h"

#include "Container/Tuple.h"

#include <utility>
#include <type_traits>

#if PPE_HAS_CXX20
// C++20 is introducing default comparison operators: https://en.cppreference.com/w/cpp/language/default_comparisons
#define __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, _OP, ...) \
    __VA_ARGS__ bool operator _OP (const _NAME& , const _NAME& ) NOEXCEPT = default;
#else
#define __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, _OP, ...) \
    __VA_ARGS__ bool operator _OP (const _NAME& lhs, const _NAME& rhs) NOEXCEPT { \
        return (PPE::tie_as_tuple(lhs) _OP PPE::tie_as_tuple(rhs)); \
    }
#endif

#define TIE_AS_TUPLE_EQUALS(_NAME, ...) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, ==, __VA_ARGS__) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, !=, __VA_ARGS__)
#define TIE_AS_TUPLE_COMPARE(_NAME, ...) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, < , __VA_ARGS__) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, >=, __VA_ARGS__) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, > , __VA_ARGS__) \
    __TIE_AS_TUPLE_OPERATOR_IMPL(_NAME, <=, __VA_ARGS__)
#define TIE_AS_TUPLE_HASH(_NAME, ...) \
    __VA_ARGS__ PPE::hash_t hash_value(const _NAME& value) NOEXCEPT { \
        return PPE::hash_value( PPE::tie_as_tuple(value) ); \
    }
#define TIE_AS_TUPLE_STRUCT(_NAME) \
    TIE_AS_TUPLE_EQUALS(_NAME, friend) \
    TIE_AS_TUPLE_HASH(_NAME, friend)

#include <climits>      // CHAR_BIT
#include <type_traits>
#include <utility>      // metaprogramming stuff

#ifndef BOOST_PFR_HAS_GUARANTEED_COPY_ELISION
#   if  defined(__cpp_guaranteed_copy_elision) && (!defined(_MSC_VER) || _MSC_VER > 1928)
#       define BOOST_PFR_HAS_GUARANTEED_COPY_ELISION 1
#   else
#       define BOOST_PFR_HAS_GUARANTEED_COPY_ELISION 0
#   endif
#endif

// Alternative implementation of tie_as_tuple()
// https://godbolt.org/z/Kr5P4xhf6

namespace PPE {
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
//----------------------------------------------------------------------------
template <std::size_t Index>
using size_t_ = std::integral_constant<std::size_t, Index >;

// This function serves as a link-time assert. If linker requires it, then
// `unsafe_declval()` is used at runtime.
void report_if_you_see_link_error_with_this_function() noexcept;

// For returning non default constructible types. Do NOT use at runtime!
//
// GCCs std::declval may not be used in potentionally evaluated contexts,
// so we reinvent it.
template <class T>
constexpr T unsafe_declval() noexcept {
    report_if_you_see_link_error_with_this_function();

    typename std::remove_reference<T>::type* ptr = 0;
    ptr += 42; // suppresses 'null pointer dereference' warnings
    return static_cast<T>(*ptr);
}

///////////////////// Structure that can be converted to reference to anything
struct ubiq_lref_constructor {
    std::size_t ignore;
    template <class Type> constexpr operator Type&() const && noexcept {  // tweak for template_unconstrained.cpp like cases
        return details::unsafe_declval<Type&>();
    }

    template <class Type> constexpr operator Type&() const & noexcept {  // tweak for optional_chrono.cpp like cases
        return details::unsafe_declval<Type&>();
    }
};

///////////////////// Structure that can be converted to rvalue reference to anything
struct ubiq_rref_constructor {
    std::size_t ignore;
    template <class Type> /*constexpr*/ operator Type() const && noexcept {  // Allows initialization of rvalue reference fields and move-only types
        return details::unsafe_declval<Type>();
    }
};


#ifndef __cpp_lib_is_aggregate
///////////////////// Hand-made is_aggregate_initializable_n<T> trait

// Structure that can be converted to reference to anything except reference to T
template <class T, bool IsCopyConstructible>
struct ubiq_constructor_except {
    std::size_t ignore;
    template <class Type> constexpr operator std::enable_if_t<!std::is_same<T, Type>::value, Type&> () const noexcept; // Undefined
};

template <class T>
struct ubiq_constructor_except<T, false> {
    std::size_t ignore;
    template <class Type> constexpr operator std::enable_if_t<!std::is_same<T, Type>::value, Type&&> () const noexcept; // Undefined
};


// `std::is_constructible<T, ubiq_constructor_except<T>>` consumes a lot of time, so we made a separate lazy trait for it.
template <std::size_t N, class T> struct is_single_field_and_aggregate_initializable: std::false_type {};
template <class T> struct is_single_field_and_aggregate_initializable<1, T>: std::integral_constant<
    bool, !std::is_constructible<T, ubiq_constructor_except<T, std::is_copy_constructible<T>::value>>::value
> {};

// Hand-made is_aggregate<T> trait:
// Before C++20 aggregates could be constructed from `decltype(ubiq_?ref_constructor{I})...` but type traits report that
// there's no constructor from `decltype(ubiq_?ref_constructor{I})...`
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
        || is_not_constructible_n(details::make_index_sequence<N>{})
    ;
};

#endif // #ifndef __cpp_lib_is_aggregate

///////////////////// Detect aggregates with inheritance
template <class Derived, class U>
constexpr bool static_assert_non_inherited() noexcept {
    static_assert(
            !std::is_base_of<U, Derived>::value,
            "====================> Boost.PFR: Boost.PFR: Inherited types are not supported."
    );
    return true;
}

template <class Derived>
struct ubiq_lref_base_asserting {
    template <class Type> constexpr operator Type&() const &&  // tweak for template_unconstrained.cpp like cases
        noexcept(details::static_assert_non_inherited<Derived, Type>())  // force the computation of assert function
    {
        return details::unsafe_declval<Type&>();
    }

    template <class Type> constexpr operator Type&() const &  // tweak for optional_chrono.cpp like cases
        noexcept(details::static_assert_non_inherited<Derived, Type>())  // force the computation of assert function
    {
        return details::unsafe_declval<Type&>();
    }
};

template <class Derived>
struct ubiq_rref_base_asserting {
    template <class Type> /*constexpr*/ operator Type() const &&  // Allows initialization of rvalue reference fields and move-only types
        noexcept(details::static_assert_non_inherited<Derived, Type>())  // force the computation of assert function
    {
        return details::unsafe_declval<Type>();
    }
};

template <class T, std::size_t I0, std::size_t... I, class /*Enable*/ = typename std::enable_if<std::is_copy_constructible<T>::value>::type>
constexpr auto assert_first_not_base(std::index_sequence<I0, I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_lref_base_asserting<T>{}, ubiq_lref_constructor{I}... })>::type
{
    return nullptr;
}

template <class T, std::size_t I0, std::size_t... I, class /*Enable*/ = typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
constexpr auto assert_first_not_base(std::index_sequence<I0, I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_rref_base_asserting<T>{}, ubiq_rref_constructor{I}... })>::type
{
    return nullptr;
}

template <class T>
constexpr void* assert_first_not_base(std::index_sequence<>) noexcept
{
    return nullptr;
}

///////////////////// Helper for SFINAE on fields count
template <class T, std::size_t... I, class /*Enable*/ = typename std::enable_if<std::is_copy_constructible<T>::value>::type>
constexpr auto enable_if_constructible_helper(std::index_sequence<I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_lref_constructor{I}... })>::type;

template <class T, std::size_t... I, class /*Enable*/ = typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
constexpr auto enable_if_constructible_helper(std::index_sequence<I...>) noexcept
    -> typename std::add_pointer<decltype(T{ ubiq_rref_constructor{I}... })>::type;

template <class T, std::size_t N, class /*Enable*/ = decltype( enable_if_constructible_helper<T>(std::make_integer_sequence<std::size_t, N>()) ) >
using enable_if_constructible_helper_t = std::size_t;

///////////////////// Helpers for range size detection
template <std::size_t Begin, std::size_t Last>
using is_one_element_range = std::integral_constant<bool, Begin == Last>;

using multi_element_range = std::false_type;
using one_element_range = std::true_type;

///////////////////// Non greedy fields count search. Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T)).
template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(details::one_element_range, long) noexcept {
    static_assert(
        Begin == Middle,
        "====================> Boost.PFR: Internal logic error."
    );
    return Begin;
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(details::multi_element_range, int) noexcept;

template <class T, std::size_t Begin, std::size_t Middle>
constexpr auto detect_fields_count(details::multi_element_range, long) noexcept
    -> details::enable_if_constructible_helper_t<T, Middle>
{
    constexpr std::size_t next_v = Middle + (Middle - Begin + 1) / 2;
    return details::detect_fields_count<T, Middle, next_v>(details::is_one_element_range<Middle, next_v>{}, 1L);
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(details::multi_element_range, int) noexcept {
    constexpr std::size_t next_v = Begin + (Middle - Begin) / 2;
    return details::detect_fields_count<T, Begin, next_v>(details::is_one_element_range<Begin, next_v>{}, 1L);
}

///////////////////// Greedy search. Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T))*T in worst case.
template <class T, std::size_t N>
constexpr auto detect_fields_count_greedy_remember(long) noexcept
    -> details::enable_if_constructible_helper_t<T, N>
{
    return N;
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_greedy_remember(int) noexcept {
    return 0;
}

template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(details::one_element_range) noexcept {
    static_assert(
        Begin == Last,
        "====================> Boost.PFR: Internal logic error."
    );
    return details::detect_fields_count_greedy_remember<T, Begin>(1L);
}

template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(details::multi_element_range) noexcept {
    constexpr std::size_t middle = Begin + (Last - Begin) / 2;
    constexpr std::size_t fields_count_big_range = details::detect_fields_count_greedy<T, middle + 1, Last>(
        details::is_one_element_range<middle + 1, Last>{}
    );

    constexpr std::size_t small_range_begin = (fields_count_big_range ? 0 : Begin);
    constexpr std::size_t small_range_last = (fields_count_big_range ? 0 : middle);
    constexpr std::size_t fields_count_small_range = details::detect_fields_count_greedy<T, small_range_begin, small_range_last>(
        details::is_one_element_range<small_range_begin, small_range_last>{}
    );
    return fields_count_big_range ? fields_count_big_range : fields_count_small_range;
}

///////////////////// Choosing between array size, greedy and non greedy search.
template <class T, std::size_t N>
constexpr auto detect_fields_count_dispatch(size_t_<N>, long, long) noexcept
    -> typename std::enable_if<std::is_array<T>::value, std::size_t>::type
{
    return sizeof(T) / sizeof(typename std::remove_all_extents<T>::type);
}

template <class T, std::size_t N>
constexpr auto detect_fields_count_dispatch(size_t_<N>, long, int) noexcept
    -> decltype(sizeof(T{}))
{
    constexpr std::size_t middle = N / 2 + 1;
    return details::detect_fields_count<T, 0, middle>(details::multi_element_range{}, 1L);
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_dispatch(size_t_<N>, int, int) noexcept {
    // T is not default aggregate initializable. It means that at least one of the members is not default constructible,
    // so we have to check all the aggregate initializations for T up to N parameters and return the biggest succeeded
    // (we can not use binary search for detecting fields count).
    return details::detect_fields_count_greedy<T, 0, N>(details::multi_element_range{});
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <class T>
constexpr std::size_t struct_num_fields() noexcept {
    using type = std::remove_cv_t<T>;

    static_assert(
        !std::is_reference<type>::value,
        "====================> Boost.PFR: Attempt to get fields count on a reference. This is not allowed because that could hide an issue and different library users expect different behavior in that case."
    );

#if !BOOST_PFR_HAS_GUARANTEED_COPY_ELISION
    static_assert(
        std::is_copy_constructible<std::remove_all_extents_t<type>>::value || (
            std::is_move_constructible<std::remove_all_extents_t<type>>::value
            && std::is_move_assignable<std::remove_all_extents_t<type>>::value
        ),
        "====================> Boost.PFR: Type and each field in the type must be copy constructible (or move constructible and move assignable)."
    );
#endif  // #if !BOOST_PFR_HAS_GUARANTEED_COPY_ELISION

    static_assert(
        !std::is_polymorphic<type>::value,
        "====================> Boost.PFR: Type must have no virtual function, because otherwise it is not aggregate initializable."
    );

#ifdef __cpp_lib_is_aggregate
    static_assert(
        std::is_aggregate<type>::value             // Does not return `true` for built-in types.
        || std::is_scalar<type>::value,
        "====================> Boost.PFR: Type must be aggregate initializable."
    );
#endif

// Can't use the following. See the non_std_layout.cpp test.
//#if !BOOST_PFR_USE_CPP17
//    static_assert(
//        std::is_standard_layout<type>::value,   // Does not return `true` for structs that have non standard layout members.
//        "Type must be aggregate initializable."
//    );
//#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1920)
    // Workaround for msvc compilers. Versions <= 1920 have a limit of max 1024 elements in template parameter pack
    constexpr std::size_t max_fields_count = (sizeof(type) * CHAR_BIT >= 1024 ? 1024 : sizeof(type) * CHAR_BIT);
#else
    constexpr std::size_t max_fields_count = (sizeof(type) * CHAR_BIT); // We multiply by CHAR_BIT because the type may have bitfields in T
#endif

    constexpr std::size_t result = details::detect_fields_count_dispatch<type>(details::size_t_<max_fields_count>{}, 1L, 1L);

    details::assert_first_not_base<type>(std::make_index_sequence<result>{});

#ifndef __cpp_lib_is_aggregate
    static_assert(
        details::is_aggregate_initializable_n<type, result>::value,
        "====================> Boost.PFR: Types with user specified constructors (non-aggregate initializable types) are not supported."
    );
#endif

    static_assert(
        result != 0 || std::is_empty<type>::value || std::is_fundamental<type>::value || std::is_reference<type>::value,
        "====================> Boost.PFR: If there's no other failed static asserts then something went wrong. Please report this issue to the github along with the structure you're reflecting."
    );

    return result;
}
//----------------------------------------------------------------------------
template <class T>
constexpr std::size_t struct_num_fields_v = struct_num_fields<T>();
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
#if PPE_HAS_CXX17
//----------------------------------------------------------------------------
static constexpr size_t MaxArityForTieAsTuple = 30;
//----------------------------------------------------------------------------
template <typename T>
constexpr bool has_tie_as_tuple() noexcept {
    using type = std::remove_cv_t<T>;

    if constexpr (std::is_reference<type>::value)
        return false;

#if !BOOST_PFR_HAS_GUARANTEED_COPY_ELISION
    if constexpr (not (std::is_copy_constructible<std::remove_all_extents_t<type>>::value || (
            std::is_move_constructible<std::remove_all_extents_t<type>>::value
        &&  std::is_move_assignable<std::remove_all_extents_t<type>>::value )))
        return false;
#endif // #if !BOOST_PFR_HAS_GUARANTEED_COPY_ELISION

    if constexpr (std::is_polymorphic<type>::value)
        return false;

#ifdef __cpp_lib_is_aggregate
    if constexpr (not (std::is_aggregate<type>::value || std::is_scalar<type>::value))
        return false;
#else
    if constexpr (not details::is_aggregate_initializable_n<type, result>::value)
        return false;
#endif

    constexpr size_t numFields = details::detect_fields_count_dispatch<type>(details::size_t_<MaxArityForTieAsTuple+1>{}, 1L, 1L);
    static_assert(
        numFields != 0 || std::is_empty<type>::value || std::is_fundamental<type>::value || std::is_reference<type>::value,
        "====================> Boost.PFR: If there's no other failed static asserts then something went wrong. Please report this issue to the github along with the structure you're reflecting."
    );

    return (numFields > 0 && numFields <= MaxArityForTieAsTuple);
}
//----------------------------------------------------------------------------
template <typename T>
constexpr bool has_tie_as_tuple_v = has_tie_as_tuple<T>();
//----------------------------------------------------------------------------
// Use new structured bindings with C++17
namespace details {
/*
** /!\ Forbid tying as tuple for types with a single or no element.
**     Those shouldn't be tied as a tuple and add ambiguity to overloads.
**
template <class T, typename _Tie>
constexpr auto tie_as_tuple_impl_(T& val, size_t_<0>, _Tie&& ) noexcept {
    return TTuple<>{};
}
template <class T, typename _Tie>
constexpr auto tie_as_tuple(T& val, size_t_<1>, _Tie&& tie, Meta::TEnableIf< not std::is_class_v< std::remove_cv_t<T> > >* = 0) noexcept {
    return tie(val);
}
*/
template <class T, typename _Tie>
constexpr auto tie_as_tuple(T& val, size_t_<1>, _Tie&& tie, Meta::TEnableIf< std::is_class_v< std::remove_cv_t<T> > >* = 0) noexcept {
    auto& [a] = val;
    return tie(a);
}

template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<2>, _Tie&& tie) NOEXCEPT {
    auto&[a,b] = val;
    return tie(a,b); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<3>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c] = val;
    return tie(a,b,c); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<4>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d] = val;
    return tie(a,b,c,d); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<5>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e] = val;
    return tie(a,b,c,d,e); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<6>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f] = val;
    return tie(a,b,c,d,e,f); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<7>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g] = val;
    return tie(a,b,c,d,e,f,g); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<8>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h] = val;
    return tie(a,b,c,d,e,f,g,h); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<9>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i] = val;
    return tie(a,b,c,d,e,f,g,h,i); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<10>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j] = val;
    return tie(a,b,c,d,e,f,g,h,i,j); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<11>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<12>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<13>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<14>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<15>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<16>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<17>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<18>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<19>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<20>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<21>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<22>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<23>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<24>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<25>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<26>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<27>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<28>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<29>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb); }
template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<30>, _Tie&& tie) NOEXCEPT {
    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db] = val;
    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<31>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<32>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<33>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<34>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<35>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<36>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<37>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<38>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<39>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<40>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<41>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<42>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<43>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<44>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<45>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<46>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<47>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<48>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<49>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<50>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<51>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<52>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<53>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<54>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<55>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<56>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<57>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<58>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<59>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc,gc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc,gc); }
//template <class T, typename _Tie> CONSTEXPR auto tie_as_tuple_impl_(T& val, size_t_<60>, _Tie&& tie) NOEXCEPT {
//    auto&[a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc,gc,hc] = val;
//    return tie(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,ab,bb,cb,db,eb,fb,gb,hb,ib,jb,kb,lb,mb,nb,ob,pb,qb,rb,sb,tb,ub,vb,wb,xb,yb,zb,ac,bc,cc,dc,ec,fc,gc,hc); }
} //!details
template <typename T, size_t _NumFields = struct_num_fields_v<T> >
constexpr auto tie_as_tuple(T& val) noexcept {
    STATIC_ASSERT(_NumFields <= MaxArityForTieAsTuple);
    STATIC_ASSERT(MaxArityForTieAsTuple == 30); // add details::tie_as_tuple() overloads if this number was incremented
    return details::tie_as_tuple_impl_(val, details::size_t_<_NumFields>{}, [](auto&... x) {
        return std::tie(x...);
    });
}
#if 0 // working, but waiting for C++23: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1061r1.html
template <typename T>
constexpr auto tie_as_tuple_rec(T& val) noexcept {
    if constexpr (has_tie_as_tuple_v<T>) {i
        return details::tie_as_tuple_impl_(val, details::size_t_<struct_num_fields_v<T>>{}, [](auto&... x) {
            return std::tie(tie_as_tuple_rec(x)...);
        });
    }
    else {
        return val;
    }
}
#endif
//----------------------------------------------------------------------------
#else //PPE_HAS_CXX17
//----------------------------------------------------------------------------
static constexpr size_t MaxArityForTieAsTuple = 0;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool has_tie_as_tuple() noexcept {
    return false;
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool has_tie_as_tuple_v = false;
//----------------------------------------------------------------------------
#endif //!PPE_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
