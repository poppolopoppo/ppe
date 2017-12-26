#pragma once

#include "Core/Meta/Aliases.h"

#ifdef PLATFORM_WINDOWS
// /W3
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable: 4503) // 'identifier' : decorated name length exceeded, name was truncated
#pragma warning(disable: 4714) // function 'function' marked as __forceinline not inlined
// /analyze
#pragma warning(disable: 6054) // String 'XXX' might not be zero-terminated.
#pragma warning(disable: 6255) // _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
#pragma warning(disable: 6326) // Potential comparison of a constant with another constant.
#endif

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
