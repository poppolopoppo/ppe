#pragma once

#ifdef PLATFORM_WINDOWS
#pragma warning(disable: 4127) // L'expression conditionnelle est une constante
#pragma warning(disable: 4201) // Extension non standard utilis�e�: struct/union sans nom.
#pragma warning(disable: 4503) // Longueur du nom d�cor� d�pass�e, le nom a �t� tronqu�
#pragma warning(disable: 6011) // Suppression de la r�f�rence du pointeur NULL 'XXX'.
#pragma warning(disable: 6255) // _alloca signale un �chec en levant une exception de d�passement de capacit� de la pile. Utilisez _malloca � la place.
#pragma warning(disable: 6385) // Lecture de donn�es non valides depuis 'XXX'�: la taille lisible est 'XXX' octets, mais 'XXX' octets sont peut-�tre lus.
#pragma warning(disable: 4714) // Fonction 'XXX' marqu�e comme __forceinline non inline
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
