#pragma once

#include "Core.h"
#include "Allocator/TrackingMalloc.h"

#ifdef __clang__
#   pragma clang diagnostic push
#endif

PRAGMA_MSVC_WARNING_PUSH()

PRAGMA_MSVC_WARNING_DISABLE(4244) // conversion from int to unsigned char, possible loss of data

#ifdef __clang__
#   pragma clang system_header
// #   pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#define STB_DXT_IMPLEMENTATION
#define STB_DXT_STATIC

#include "External/stb/stb.git/stb_dxt.h"

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

PRAGMA_MSVC_WARNING_POP()
