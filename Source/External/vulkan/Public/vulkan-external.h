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

#pragma include_alias("vk_video/vulkan_video_codec_av1std.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_av1std.h")
#pragma include_alias("vk_video/vulkan_video_codec_av1std_encode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_av1std_encode.h")
#pragma include_alias("vk_video/vulkan_video_codec_av1std_decode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_av1std_decode.h")

#pragma include_alias("vk_video/vulkan_video_codec_h264std.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h264std.h")
#pragma include_alias("vk_video/vulkan_video_codec_h264std_encode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h264std_encode.h")
#pragma include_alias("vk_video/vulkan_video_codec_h264std_decode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h264std_decode.h")

#pragma include_alias("vk_video/vulkan_video_codec_h265std.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h265std.h")
#pragma include_alias("vk_video/vulkan_video_codec_h265std_encode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h265std_encode.h")
#pragma include_alias("vk_video/vulkan_video_codec_h265std_decode.h", "External/vulkan/Vulkan-Header.git/include/vk_video/vulkan_video_codec_h265std_decode.h")

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
