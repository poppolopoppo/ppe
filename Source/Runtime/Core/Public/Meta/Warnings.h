#pragma once

#include "Meta/Aliases.h"

#ifdef PLATFORM_WINDOWS
#   define PPE_MESSAGE(_Message) \
    __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ",1): " EXPAND(_Message)))
#else
#   define PPE_MESSAGE(_Message) \
    _Pragma(STRINGIZE(message(_Message)))
#endif

#define PPE_WARNING(_Code, _Message) PPE_MESSAGE("warning " _Code ": " _Message)
#define PPE_ERROR(_Code, _Message) PPE_MESSAGE("error " _Code ": " _Message)

#if defined(CPP_VISUALSTUDIO)
#   define PPE_DEPRECATED __declspec(deprecated)
#elif   defined(CPP_GCC) || defined(CPP_CLANG)
#   define PPE_DEPRECATED __attribute__((deprecated))
#else
PPE_WARNING("Core", "You need to implement PPE_DEPRECATED for this compiler")
#   define PPE_DEPRECATED
#endif

#ifdef _MSC_VER
#   define PRAGMA_MSVC_WARNING_PUSH() \
        __pragma(warning(push))
#   define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE) \
        __pragma(warning(disable: _WARNING_CODE))
#   define PRAGMA_MSVC_WARNING_POP() \
        __pragma(warning(pop))
#else
#   define PRAGMA_MSVC_WARNING_PUSH()
#   define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE)
#   define PRAGMA_MSVC_WARNING_POP()
#endif

// /W3
#if !PPE_HAS_CXX17 // use IF_CONSTEXPR for C++-17
PRAGMA_MSVC_WARNING_DISABLE(4127) // conditional expression is constant
#endif
PRAGMA_MSVC_WARNING_DISABLE(4201) // nonstandard extension used : nameless struct/union
PRAGMA_MSVC_WARNING_DISABLE(4503) // 'identifier' : decorated name length exceeded, name was truncated
PRAGMA_MSVC_WARNING_DISABLE(4714) // function 'function' marked as __forceinline not inlined

// /analyze
PRAGMA_MSVC_WARNING_DISABLE(6054) // String 'XXX' might not be zero-terminated.
PRAGMA_MSVC_WARNING_DISABLE(6255) // _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
PRAGMA_MSVC_WARNING_DISABLE(6326) // Potential comparison of a constant with another constant.
