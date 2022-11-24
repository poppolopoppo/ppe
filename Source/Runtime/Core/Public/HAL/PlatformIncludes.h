#pragma once

// mark files including as system headers : warnings will be ignored starting from here

#ifdef __clang__
#   pragma clang system_header
#endif
#ifdef __gcc__
#   pragma GCC system_header
#endif
#if defined(_MSC_VER) && defined(USE_PPE_MSVC_PRAGMA_SYSTEMHEADER)
#   pragma system_header
#endif

#include "HAL/PlatformMacros.h"
#include PPE_HAL_MAKEINCLUDE(PlatformIncludes)
