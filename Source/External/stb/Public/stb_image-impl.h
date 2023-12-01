#pragma once

#include "Core.h"
#include "Allocator/TrackingMalloc.h"

#ifdef __clang__
#   pragma clang diagnostic push
#endif

PRAGMA_MSVC_WARNING_PUSH()

PRAGMA_MSVC_WARNING_DISABLE(4244) // 'XXX': conversion from 'int' to 'short', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'XXX': unreferenced function with internal linkage has been removed

#ifdef __clang__
#   pragma clang system_header
// #   pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#define STBI_NO_STDIO
#define STBI_MALLOC(sz)         TRACKING_MALLOC(STBImage, sz)
#define STBI_REALLOC(p,newsz)   TRACKING_REALLOC(STBImage, p, newsz)
#define STBI_FREE(p)            TRACKING_FREE(STBImage, p)
#define STBI_ASSERT(x)          Assert_NoAssume(("stb_image: "), (x))

#include "External/stb/stb.git/stb_image.h"

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

PRAGMA_MSVC_WARNING_POP()
