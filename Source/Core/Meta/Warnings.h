#pragma once

#ifdef PLATFORM_WINDOWS
// /W3
#pragma warning(disable: 4127) // L'expression conditionnelle est une constante
#pragma warning(disable: 4201) // Extension non standard utilisée: struct/union sans nom.
#pragma warning(disable: 4503) // Longueur du nom décoré dépassée, le nom a été tronqué
#pragma warning(disable: 4714) // Fonction 'XXX' marquée comme __forceinline non inline
// /analyze
#pragma warning(disable: 6054) // String 'XXX' might not be zero-terminated.
#pragma warning(disable: 6255) // _alloca signale un échec en levant une exception de dépassement de capacité de la pile. Utilisez _malloca à la place.
#pragma warning(disable: 6326) // Potential comparison of a constant with another constant.
#endif

#define CORE_MESSAGE(_Message) \
    __pragma(message(__FILE__ "(" STRINGIZE(__LINE__) ",1): " _Message))

#define CORE_WARNING(_Code, _Message) CORE_MESSAGE("WARNING " _Code ": " _Message)

#if     defined(CPP_VISUALSTUDIO)
#   define CORE_DEPRECATED __declspec(deprecated)
#elif   defined(CPP_GCC) || defined(CPP_CLANG)
#   define CORE_DEPRECATED __attribute__((deprecated))
#else
    CORE_WARNING("Core", "You need to implement CORE_DEPRECATED for this compiler")
#   define CORE_DEPRECATED
#endif
