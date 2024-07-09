#pragma once

#include "Core.h"
#include "Allocator/TrackingMalloc.h"

#ifdef __clang__
#   pragma clang diagnostic push
#endif

PRAGMA_MSVC_WARNING_PUSH()

PRAGMA_MSVC_WARNING_DISABLE(4456) // declaration of 'c' hides previous local declaration
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'XXX': unreferenced function with internal linkage has been removed
PRAGMA_MSVC_WARNING_DISABLE(4668) // undefined preprocessor macro

#ifdef __clang__
#   pragma clang system_header
// #   pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC

#define STBIR_USE_FMA

#define STBIR_MALLOC(size,user_data) ((void)(user_data), TRACKING_MALLOC(STBImageResize, size))
#define STBIR_FREE(ptr,user_data)    ((void)(user_data), TRACKING_FREE(STBImageResize, ptr))
#define STBIR_ASSERT(x) Assert_NoAssume(("stb_image_resize2: "), (x))

#include "External/stb/stb.git/stb_image_resize2.h"

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

PRAGMA_MSVC_WARNING_POP()
