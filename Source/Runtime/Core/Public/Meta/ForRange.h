#pragma once

#include "Core.h"

#include "Meta/TypeTraits.h"

namespace PPE {
namespace Meta {
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
// Here, N == 8.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, N, ...) N
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
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 6 args.
 */
#define PP_FOREACH_ARGS(x, ...) \
    EXPAND( _GET_NTH_ARG("ignored", ##__VA_ARGS__, \
        _PP_FE_ARGS_6, _PP_FE_ARGS_5, _PP_FE_ARGS_4, _PP_FE_ARGS_3, _PP_FE_ARGS_2, _PP_FE_ARGS_1, _PP_FE_ARGS_0) \
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
//----------------------------------------------------------------------------
/**
 * Provide a for-each construct for variadic macros. Supports up
 * to 6 args.
 *
 * Example usage1:
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
        _PP_FE_6, _PP_FE_5, _PP_FE_4, _PP_FE_3, _PP_FE_2, _PP_FE_1, _PP_FE_0) \
            (x, ##__VA_ARGS__) )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
