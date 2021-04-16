#pragma once

#include "Core_fwd.h"

#define VK_NO_PROTOTYPES

#if     defined(PLATFORM_WINDOWS)
#   define VK_USE_PLATFORM_WIN32_KHR 1
#elif   defined(PLATFORM_LINUX)
#   define VK_USE_PLATFORM_XLIB_KHR 1
#else
#   error "unknown platform"
#endif
