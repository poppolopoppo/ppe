#pragma once

#define PPE_COMPILER_MESSAGE(_Message) \
    __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ",1): " EXPAND(_Message)))
#define PPE_COMPILER_WARNING(_Code, _Message) PPE_COMPILER_MESSAGE("warning " _Code ": " _Message)
#define PPE_COMPILER_ERROR(_Code, _Message) PPE_COMPILER_MESSAGE("error " _Code ": " _Message)

#if defined(CPP_VISUALSTUDIO)
#   define PPE_DEPRECATED __declspec(deprecated)
#elif defined(CPP_CLANG)
#   define PPE_DEPRECATED __attribute__((deprecated))
#else
PPE_COMPILER_ERROR("Windows", "unsupported compiler for this platform")
#endif

#define PRAGMA_MSVC_WARNING_PUSH() __pragma(warning(push))
#define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE)  __pragma(warning(disable: _WARNING_CODE))
#define PRAGMA_MSVC_WARNING_POP() __pragma(warning(pop))

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
PRAGMA_MSVC_WARNING_DISABLE(26812) // unscoped enums

#if USE_PPE_PLATFORM_DEBUG

#   define PPE_DEBUG_BREAK() __debugbreak() // more comfy to break in current frame
#   define PPE_DEBUG_CRASH() ::FatalExit(-1) // transfers execution control to the debugger
#   define PPE_DECLSPEC_ALLOCATOR() __declspec(allocator)
#   define PPE_DECLSPEC_CODE_SECTION(_NAME) __declspec(code_seg(_NAME))

#endif
