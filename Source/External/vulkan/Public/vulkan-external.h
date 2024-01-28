#pragma once

#include "vulkan-platform.h"

#ifdef __clang__
#    pragma clang system_header
#endif

#ifdef __gcc__
#    pragma GCC system_header
#endif

PRAGMA_MSVC_WARNING_PUSH()

#pragma include_alias(<vulkan/vulkan.h>, <External/vulkan/Vulkan-Header.git/include/vulkan/vulkan.h>)
#pragma include_alias("vulkan/vk_platform.h", "External/vulkan/Vulkan-Header.git/include/vulkan/vk_platform.h")

#include "HAL/PlatformIncludes.h"

#include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR) && VK_USE_PLATFORM_WIN32_KHR
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_win32.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR) && VK_USE_PLATFORM_ANDROID_KHR
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_android.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR) && VK_USE_PLATFORM_XCB_KHR
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_xcb.h"
#elif defined(VK_USE_PLATFORM_XLIB_KHR) && VK_USE_PLATFORM_XLIB_KHR
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_xlib.h"
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT) && VK_USE_PLATFORM_XLIB_XRANDR_EXT
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_xlib_xrandr.h"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR) && VK_USE_PLATFORM_WAYLAND_KHR
#   include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_wayland.h"
#endif

#ifdef Always // spilled from X11/X.h
#   undef Always
#endif
#ifdef None // spilled from X11/X.h
#   undef None
#endif

PRAGMA_MSVC_WARNING_POP()
