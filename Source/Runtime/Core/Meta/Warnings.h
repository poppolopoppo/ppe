#pragma once

#include "Core/Meta/Aliases.h"

#define CORE_MESSAGE(_Message) \
    __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ",1): " _Message))

#define CORE_WARNING(_Code, _Message) CORE_MESSAGE("warning " _Code ": " _Message)
#define CORE_ERROR(_Code, _Message) CORE_MESSAGE("error " _Code ": " _Message)

#if     defined(CPP_VISUALSTUDIO)
#   define CORE_DEPRECATED __declspec(deprecated)
#elif   defined(CPP_GCC) || defined(CPP_CLANG)
#   define CORE_DEPRECATED __attribute__((deprecated))
#else
CORE_WARNING("Core", "You need to implement CORE_DEPRECATED for this compiler")
#   define CORE_DEPRECATED
#endif

#ifdef _MSC_VER
#   define PRAGMA_MSVC_WARNING_PUSH() \
        __pragma(warning(push))
#   define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE) \
        /*__pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ",1): disabled MSVC warning C" STRINGIZE(_WARNING_CODE) ))*/ \
        __pragma(warning(disable: _WARNING_CODE))
#   define PRAGMA_MSVC_WARNING_POP() \
        __pragma(warning(pop))
#else
#   define PRAGMA_MSVC_WARNING_PUSH()
#   define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE)
#   define PRAGMA_MSVC_WARNING_POP()
#endif

// /W3
#if !_HAS_CXX17 // use IF_CONSTEXPR for C++-17
PRAGMA_MSVC_WARNING_DISABLE(4127) // conditional expression is constant
#endif
PRAGMA_MSVC_WARNING_DISABLE(4201) // nonstandard extension used : nameless struct/union
PRAGMA_MSVC_WARNING_DISABLE(4503) // 'identifier' : decorated name length exceeded, name was truncated
PRAGMA_MSVC_WARNING_DISABLE(4714) // function 'function' marked as __forceinline not inlined

// /analyze
PRAGMA_MSVC_WARNING_DISABLE(6054) // String 'XXX' might not be zero-terminated.
PRAGMA_MSVC_WARNING_DISABLE(6255) // _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
PRAGMA_MSVC_WARNING_DISABLE(6326) // Potential comparison of a constant with another constant.
