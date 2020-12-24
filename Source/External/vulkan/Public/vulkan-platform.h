#pragma once

#include "Core_fwd.h"

#define VK_NO_PROTOTYPES
#define USE_PPE_VULKAN_MINIMALAPI (0)

#if     defined(PLATFORM_WINDOWS)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif   defined(PLATFORM_LINUX)
#   define VK_USE_PLATFORM_XLIB_KHR
#else
#   error "unknown platform"
#endif
