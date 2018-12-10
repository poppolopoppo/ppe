#pragma once

#include "Core.h"

#include "Meta/TypeTraits.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <class F, size_t... Is>
constexpr auto expand_indices_seq(F f, std::index_sequence<Is...>) {
    return f(std::integral_constant<size_t, Is> {}...);
}
}//!details
//----------------------------------------------------------------------------
template <size_t N, class F>
constexpr auto static_for(F f) {
    return details::expand_indices_seq(f, std::make_index_sequence<N>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define forrange(_Variable, _Start, _End) \
    for (   PPE::Meta::TDecay<decltype(_End)> _Variable = _Start, CONCAT(_Variable, _END) = _End; \
            Assert_NoAssume(CONCAT(_Variable, _END) == _End), _Variable != CONCAT(_Variable, _END); \
            ++ _Variable )
//----------------------------------------------------------------------------
#define reverseforrange(_Variable, _Start, _End) \
    for (   PPE::Meta::TDecay<decltype(_End)> CONCAT(_Variable, _REV) = _Start, CONCAT(_Variable, _END) = _End, _Variable = CONCAT(_Variable, _END) - 1; \
            Assert_NoAssume(CONCAT(_Variable, _END) == _End), CONCAT(_Variable, _REV) != CONCAT(_Variable, _END); \
            ++ CONCAT(_Variable, _REV), -- _Variable )
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
// Accept any number of args >= N, but expand to just the Nth one.
// Here, N == 10.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define _LPARENTHESIS (
#define _RPARENTHESIS )
//----------------------------------------------------------------------------
// Define some macros to help us create overrides based on the
// arity of a for-each-style macro. (COMMA SEPARATED)
#define _PP_FE_ARGS_0(_call, ...)
#define _PP_FE_ARGS_1(_call, x) _call(x)
#define _PP_FE_ARGS_2(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_1(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_3(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_2(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_4(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_3(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_5(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_4(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_6(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_5(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_7(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_6(_call, __VA_ARGS__) )
#define _PP_FE_ARGS_8(_call, x, ...) _call(x) , EXPAND( _PP_FE_ARGS_7(_call, __VA_ARGS__) )
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 8 args.
 */
#define PP_FOREACH_ARGS(x, ...) \
    EXPAND( _GET_NTH_ARG("ignored", ##__VA_ARGS__, \
        _PP_FE_ARGS_8, _PP_FE_ARGS_7, _PP_FE_ARGS_6, _PP_FE_ARGS_5, _PP_FE_ARGS_4, _PP_FE_ARGS_3, _PP_FE_ARGS_2, _PP_FE_ARGS_1, _PP_FE_ARGS_0) \
            (x, ##__VA_ARGS__) )
//----------------------------------------------------------------------------
// Define some macros to help us create overrides based on the
// arity of a for-each-style macro.
#define _PP_FE_0(_call, ...)
#define _PP_FE_1(_call, x) _call(x)
#define _PP_FE_2(_call, x, ...) _call(x) , EXPAND( _PP_FE_1(_call, __VA_ARGS__) )
#define _PP_FE_3(_call, x, ...) _call(x) , EXPAND( _PP_FE_2(_call, __VA_ARGS__) )
#define _PP_FE_4(_call, x, ...) _call(x) , EXPAND( _PP_FE_3(_call, __VA_ARGS__) )
#define _PP_FE_5(_call, x, ...) _call(x) , EXPAND( _PP_FE_4(_call, __VA_ARGS__) )
#define _PP_FE_6(_call, x, ...) _call(x) , EXPAND( _PP_FE_5(_call, __VA_ARGS__) )
#define _PP_FE_7(_call, x, ...) _call(x) , EXPAND( _PP_FE_6(_call, __VA_ARGS__) )
#define _PP_FE_8(_call, x, ...) _call(x) , EXPAND( _PP_FE_7(_call, __VA_ARGS__) )
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 8 args.
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
#define PP_FOREACH(x, ...) \
    EXPAND( _GET_NTH_ARG("ignored", ##__VA_ARGS__, \
        _PP_FE_8, _PP_FE_7, _PP_FE_6, _PP_FE_5, _PP_FE_4, _PP_FE_3, _PP_FE_2, _PP_FE_1, _PP_FE_0) \
            (x, ##__VA_ARGS__) )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
/**
* Return the number of arguments given to the macro. Supports up
* to 8 args.
*
* Example usage 1:
*       PP_COUNT_ARGS(a, b, c) // expanded as 3
*
* Example usage 2:
*       PP_COUNT_ARGS() // expanded as 0
*/
#if defined(_MSC_VER) && not defined(__clang__)
#   define PP_NUM_ARGS(...) \
    _GET_NTH_ARG _LPARENTHESIS EXPAND_VA("ignored", ##__VA_ARGS__), 8, 7, 6, 5, 4, 3, 2, 1, 0 _RPARENTHESIS
#else
#   define PP_NUM_ARGS(...) \
    EXPAND( _GET_NTH_ARG("ignored", ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0) )
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
