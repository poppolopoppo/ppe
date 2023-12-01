#pragma once

#include "Core.h"
#include "Allocator/TrackingMalloc.h"

#ifdef __clang__
#   pragma clang diagnostic push
#endif

PRAGMA_MSVC_WARNING_PUSH()

PRAGMA_MSVC_WARNING_DISABLE(4505) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS.
PRAGMA_MSVC_WARNING_DISABLE(4996) // 'XXX': unreferenced function with internal linkage has been removed

#ifdef __clang__
#   pragma clang system_header
// #   pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC

//#define STBI_WRITE_NO_STDIO // Disabling STDIO also disables HDR support... (https://github.com/nothings/stb/issues/793)

#define STBIW_MALLOC(sz)         TRACKING_MALLOC(STBImageWrite, sz)
#define STBIW_REALLOC(p,newsz)   TRACKING_REALLOC(STBImageWrite, p, newsz)
#define STBIW_FREE(p)            TRACKING_FREE(STBImageWrite, p)
#define STBIW_ASSERT(x)          Assert_NoAssume(("stb_image_write: "), (x))

#include "External/stb/stb.git/stb_image_write.h"

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

PRAGMA_MSVC_WARNING_POP()
