#pragma once

#ifdef OS_WINDOWS
#pragma warning(disable: 4127) // L'expression conditionnelle est une constante
#pragma warning(disable: 4201) // Extension non standard utilisée : struct/union sans nom.
#pragma warning(disable: 4503) // Longueur du nom décoré dépassée, le nom a été tronqué
#pragma warning(disable: 6011) // Suppression de la référence du pointeur NULL 'XXX'.
#pragma warning(disable: 6255) // _alloca signale un échec en levant une exception de dépassement de capacité de la pile. Utilisez _malloca à la place.
#pragma warning(disable: 6385) // Lecture de données non valides depuis 'XXX' : la taille lisible est 'XXX' octets, mais 'XXX' octets sont peut-être lus.
#endif

