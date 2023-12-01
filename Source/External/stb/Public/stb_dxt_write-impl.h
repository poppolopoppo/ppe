#pragma once

#include "Core.h"
#include "Allocator/TrackingMalloc.h"

#ifdef __clang__
#   pragma clang diagnostic push
#endif

PRAGMA_MSVC_WARNING_PUSH()

// PRAGMA_MSVC_WARNING_DISABLE(4065) // switch statement contains 'default' but no 'case' labels
// PRAGMA_MSVC_WARNING_DISABLE(4389) // '!=': signed/unsigned mismatch
// PRAGMA_MSVC_WARNING_DISABLE(4701) // potentially uninitialized local variable
// PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code
// PRAGMA_MSVC_WARNING_DISABLE(4703) // potentially uninitialized local pointer variable
// PRAGMA_MSVC_WARNING_DISABLE(4706) // assignment within conditional expression
// PRAGMA_MSVC_WARNING_DISABLE(4805) // '!=': unsafe mix of type 'const uint32_t' and type 'const bool' in operation
// PRAGMA_MSVC_WARNING_DISABLE(4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

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
