#pragma once

#include "vulkan-platform.h"

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

PRAGMA_MSVC_WARNING_PUSH()

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

#elif

PRAGMA_MSVC_WARNING_POP()
