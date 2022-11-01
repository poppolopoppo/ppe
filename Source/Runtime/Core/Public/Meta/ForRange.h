#pragma once

#include "Core_fwd.h"

#include "Meta/TypeTraits.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <class F, u32... Is>
FORCE_INLINE constexpr auto expand_indices_seq(F f, std::integer_sequence<u32, Is...>) {
    return f(std::integral_constant<u32, Is> {}...);
}
}//!details
template <size_t N, class F>
FORCE_INLINE constexpr auto static_for(F f) {
    return details::expand_indices_seq(f, std::make_integer_sequence<u32, N>{});
}
//----------------------------------------------------------------------------
namespace details {
template <template <size_t...> typename T, typename _Sequence>
struct expand_indices_t_ {};
template <template <size_t...> typename T, size_t... _Indices>
struct expand_indices_t_<T, std::index_sequence<_Indices...>> {
    using type = T<_Indices...>;
};
} //!details
template <template <size_t...> typename T, size_t N>
using make_expand_indices = typename details::expand_indices_t_<T, std::make_index_sequence<N>>::type;
template <template <size_t...> typename T, typename... _Args>
using expand_indices_for = make_expand_indices<T, sizeof...(_Args)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define forrange(_Variable, _Start, _End) \
    for (   PPE::Meta::TDecay<decltype(_End)> _Variable = (_Start), CONCAT(_Variable, _END) = _End; \
            Assert_NoAssume(CONCAT(_Variable, _END) == (_End)), (_Variable) != CONCAT(_Variable, _END); \
            ++(_Variable) )
//----------------------------------------------------------------------------
#define reverseforrange(_Variable, _Start, _End) \
    for (   PPE::Meta::TDecay<decltype(_End)> CONCAT(_Variable, _REV) = (_Start), CONCAT(_Variable, _END) = (_End), (_Variable) = CONCAT(_Variable, _END) - 1; \
            Assert_NoAssume(CONCAT(_Variable, _END) == (_End)), CONCAT(_Variable, _REV) != CONCAT(_Variable, _END); \
            ++ CONCAT(_Variable, _REV), --(_Variable) )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define foreachitem(_Variable, _ContainerLike) \
    forrange(_Variable, std::begin(_ContainerLike), std::end(_ContainerLike))
//----------------------------------------------------------------------------
#define reverseforeachitem(_Variable, _ContainerLike) \
    forrange(_Variable, std::rbegin(_ContainerLike), std::rend(_ContainerLike))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://stackoverflow.com/questions/48045470/portably-detect-va-opt-support
// https://devblogs.microsoft.com/cppblog/announcing-full-support-for-a-c-c-conformant-preprocessor-in-msvc/
#if __cplusplus <= 201703 && defined __GNUC__ && !defined __clang__ && !defined __EDG__
#   define PPE_VA_OPT_SUPPORTED false // These compilers pretend to be GCC
#elif defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#   define PPE_VA_OPT_SUPPORTED false // Legacy MSVC preprocessor
#elif !PPE_HAS_CXX20
#   define PPE_VA_OPT_SUPPORTED false // __VA_OPT__ is supported only since C++20
#else
#   define PPE_THIRD_ARG(a,b,c,...) c
#   define PPE_VA_OPT_SUPPORTED_I(...) PPE_THIRD_ARG(__VA_OPT__(,),true,false,)
#   define PPE_VA_OPT_SUPPORTED PPE_VA_OPT_SUPPORTED_I(?)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Accept any number of args >= N, but expand to just the Nth one.
// Here, N == 16.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define _LPARENTHESIS (
#define _RPARENTHESIS )
//----------------------------------------------------------------------------
// Define some macros to help us create overrides based on the
// arity of a for-each-style macro. (COMMA SEPARATED)
#define _PP_FE_ARGS_0(_call, ...)
#define _PP_FE_ARGS_1(_call, x) _call(x)
#define _PP_FE_ARGS_2(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_1(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_3(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_2(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_4(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_3(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_5(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_4(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_6(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_5(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_7(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_6(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_8(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_7(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_9(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_8(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_10(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_9(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_11(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_10(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_12(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_11(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_13(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_12(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_14(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_13(_call, ##__VA_ARGS__) )
#define _PP_FE_ARGS_15(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_14(_call, ##__VA_ARGS__) )
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 14 args.
 */
#if PPE_VA_OPT_SUPPORTED
#   define PP_FOREACH_ARGS(x, ...) \
    EXPAND( _GET_NTH_ARG(unused __VA_OPT__(,) __VA_ARGS__, \
        _PP_FE_ARGS_15, _PP_FE_ARGS_14, _PP_FE_ARGS_13, _PP_FE_ARGS_12, _PP_FE_ARGS_11, _PP_FE_ARGS_10, _PP_FE_ARGS_9, \
        _PP_FE_ARGS_8, _PP_FE_ARGS_7, _PP_FE_ARGS_6, _PP_FE_ARGS_5, _PP_FE_ARGS_4, _PP_FE_ARGS_3, _PP_FE_ARGS_2, _PP_FE_ARGS_1, _PP_FE_ARGS_0) \
            (x __VA_OPT__(,) __VA_ARGS__) )
#else
#   define PP_FOREACH_ARGS(x, ...) \
    EXPAND( _GET_NTH_ARG(unused , ## __VA_ARGS__, \
        _PP_FE_ARGS_15, _PP_FE_ARGS_14, _PP_FE_ARGS_13, _PP_FE_ARGS_12, _PP_FE_ARGS_11, _PP_FE_ARGS_10, _PP_FE_ARGS_9, \
        _PP_FE_ARGS_8, _PP_FE_ARGS_7, _PP_FE_ARGS_6, _PP_FE_ARGS_5, _PP_FE_ARGS_4, _PP_FE_ARGS_3, _PP_FE_ARGS_2, _PP_FE_ARGS_1, _PP_FE_ARGS_0) \
            (x , ## __VA_ARGS__) )
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Define some macros to help us create overrides based on the
// arity of a for-each-style macro.
#define _PP_FE_TUPLE_0(_call, ...)
#define _PP_FE_TUPLE_1(_call, x) EXPAND(_call EXPAND(x) )
#define _PP_FE_TUPLE_2(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_1(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_3(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_2(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_4(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_3(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_5(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_4(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_6(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_5(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_7(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_6(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_8(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_7(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_9(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_8(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_10(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_9(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_11(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_10(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_12(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_11(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_13(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_12(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_14(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_13(_call, __VA_ARGS__) )
#define _PP_FE_TUPLE_15(_call, x, ...) EXPAND(_call EXPAND(x) ) EXPAND( _PP_FE_TUPLE_14(_call, __VA_ARGS__) )
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 14 args.
 *
 * Example usage 1:
 *     #define FWD_DECLARE_CLASS(cls) class cls;
 *     PP_FOREACH(FWD_DECLARE_CLASS, Foo, Bar)
 *
 * Example usage 2:
 *     #define START_NS(ns) namespace ns {
 *     #define END_NS(ns) }
 *     #define MY_NAMESPACES System, Net, Http
 *     PP_FOREACH(START_NS, MY_NAMESPACES)
 *     typedef foo int;
 *     PP_FOREACH(END_NS, MY_NAMESPACES)
 */
#if PPE_VA_OPT_SUPPORTED
#   define PP_FOREACH_TUPLE(x, ...) \
    EXPAND( _GET_NTH_ARG(unused __VA_OPT__(,) __VA_ARGS__, \
        _PP_FE_TUPLE_15, _PP_FE_TUPLE_14, _PP_FE_TUPLE_13, _PP_FE_TUPLE_12, _PP_FE_TUPLE_11, _PP_FE_TUPLE_10, _PP_FE_TUPLE_9, \
        _PP_FE_TUPLE_8, _PP_FE_TUPLE_7, _PP_FE_TUPLE_6, _PP_FE_TUPLE_5, _PP_FE_TUPLE_4, _PP_FE_TUPLE_3, _PP_FE_TUPLE_2, _PP_FE_TUPLE_1, _PP_FE_TUPLE_0) \
            (x __VA_OPT__(,) __VA_ARGS__) )
#else
#   define PP_FOREACH_TUPLE(x, ...) \
    EXPAND( _GET_NTH_ARG(unused , ## __VA_ARGS__, \
        _PP_FE_TUPLE_15, _PP_FE_TUPLE_14, _PP_FE_TUPLE_13, _PP_FE_TUPLE_12, _PP_FE_TUPLE_11, _PP_FE_TUPLE_10, _PP_FE_TUPLE_9, \
        _PP_FE_TUPLE_8, _PP_FE_TUPLE_7, _PP_FE_TUPLE_6, _PP_FE_TUPLE_5, _PP_FE_TUPLE_4, _PP_FE_TUPLE_3, _PP_FE_TUPLE_2, _PP_FE_TUPLE_1, _PP_FE_TUPLE_0) \
            (x , ## __VA_ARGS__) )
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Define some macros to help us create overrides based on the
// arity of a for-each-style macro.
#define _PP_FE_0(_call, ...)
#define _PP_FE_1(_call, x) _call(x)
#define _PP_FE_2(_call, x, ...) _call(x) EXPAND( _PP_FE_1(_call, __VA_ARGS__) )
#define _PP_FE_3(_call, x, ...) _call(x) EXPAND( _PP_FE_2(_call, __VA_ARGS__) )
#define _PP_FE_4(_call, x, ...) _call(x) EXPAND( _PP_FE_3(_call, __VA_ARGS__) )
#define _PP_FE_5(_call, x, ...) _call(x) EXPAND( _PP_FE_4(_call, __VA_ARGS__) )
#define _PP_FE_6(_call, x, ...) _call(x) EXPAND( _PP_FE_5(_call, __VA_ARGS__) )
#define _PP_FE_7(_call, x, ...) _call(x) EXPAND( _PP_FE_6(_call, __VA_ARGS__) )
#define _PP_FE_8(_call, x, ...) _call(x) EXPAND( _PP_FE_7(_call, __VA_ARGS__) )
#define _PP_FE_9(_call, x, ...) _call(x) EXPAND( _PP_FE_8(_call, __VA_ARGS__) )
#define _PP_FE_10(_call, x, ...) _call(x) EXPAND( _PP_FE_9(_call, __VA_ARGS__) )
#define _PP_FE_11(_call, x, ...) _call(x) EXPAND( _PP_FE_10(_call, __VA_ARGS__) )
#define _PP_FE_12(_call, x, ...) _call(x) EXPAND( _PP_FE_11(_call, __VA_ARGS__) )
#define _PP_FE_13(_call, x, ...) _call(x) EXPAND( _PP_FE_12(_call, __VA_ARGS__) )
#define _PP_FE_14(_call, x, ...) _call(x) EXPAND( _PP_FE_13(_call, __VA_ARGS__) )
#define _PP_FE_15(_call, x, ...) _call(x) EXPAND( _PP_FE_14(_call, __VA_ARGS__) )
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 14 args.
 *
 * Example usage 1:
 *     #define FWD_DECLARE_CLASS(cls) class cls;
 *     PP_FOREACH(FWD_DECLARE_CLASS, Foo, Bar)
 *
 * Example usage 2:
 *     #define START_NS(ns) namespace ns {
 *     #define END_NS(ns) }
 *     #define MY_NAMESPACES System, Net, Http
 *     PP_FOREACH(START_NS, MY_NAMESPACES)
 *     typedef foo int;
 *     PP_FOREACH(END_NS, MY_NAMESPACES)
 */
#if PPE_VA_OPT_SUPPORTED
#   define PP_FOREACH(x, ...) \
    EXPAND( _GET_NTH_ARG(unused __VA_OPT__(,) __VA_ARGS__, \
        _PP_FE_15, _PP_FE_14, _PP_FE_13, _PP_FE_12, _PP_FE_11, _PP_FE_10, _PP_FE_9, \
        _PP_FE_8, _PP_FE_7, _PP_FE_6, _PP_FE_5, _PP_FE_4, _PP_FE_3, _PP_FE_2, _PP_FE_1, _PP_FE_0) \
            (x __VA_OPT__(,) __VA_ARGS__) )
#else
#   define PP_FOREACH(x, ...) \
    EXPAND( _GET_NTH_ARG(unused , ## __VA_ARGS__, \
        _PP_FE_15, _PP_FE_14, _PP_FE_13, _PP_FE_12, _PP_FE_11, _PP_FE_10, _PP_FE_9, \
        _PP_FE_8, _PP_FE_7, _PP_FE_6, _PP_FE_5, _PP_FE_4, _PP_FE_3, _PP_FE_2, _PP_FE_1, _PP_FE_0) \
            (x , ## __VA_ARGS__) )
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/**
* Return the number of arguments given to the macro. Supports up
* to 14 args.
*
* Example usage 1:
*       PP_NUM_ARGS(a, b, c) // expanded as 3
*
* Example usage 2:
*       PP_NUM_ARGS() // expanded as 0
*/
#if PPE_VA_OPT_SUPPORTED
// should work with every compiler supporting C++20 thanks to new __VA_OPT__()
#   define PP_NUM_ARGS(...) \
    EXPAND( _GET_NTH_ARG(unused __VA_OPT__(,) __VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0) )
#elif defined(_MSC_VER) && not defined(__clang__)
#   define PP_NUM_ARGS(...) \
    _GET_NTH_ARG _LPARENTHESIS EXPAND_VA(unused , ## __VA_ARGS__), 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 _RPARENTHESIS
#else
#   define PP_NUM_ARGS(...) \
    EXPAND( _GET_NTH_ARG(unused , ## __VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, error) )
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
