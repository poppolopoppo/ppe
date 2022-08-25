#pragma once

#if defined(PLATFORM_WINDOWS)
//# define _WIN32_WINNT 0x0603//_WIN32_WINNT_WINBLUE: See you, space cowboy
#   define _WIN32_WINNT 0x0A00//_WIN32_WINNT_WIN10
#   include <SDKDDKVer.h>
#endif
